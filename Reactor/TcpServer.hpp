#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <cassert>
#include <unordered_map>
#include "Error.hpp"
#include "Log.hpp"
#include "Epoller.hpp"
#include "Socket.hpp"
#include "Util.hpp"

namespace tcpserver
{
    class Connection;
    class TcpServer;

    static const uint16_t default_port = 8080;
    static const int num = 64;

    using func_t = std::function<void(Connection *)>;

    class Connection
    {
    public:
        Connection(int sock, TcpServer *tsp) : _sock(sock), _tsp(tsp) {}

        void Register(func_t recver, func_t sender, func_t excepter)
        {
            _recver = recver;
            _sender = sender;
            _excepter = excepter;
        }

        void Close()
        {
            close(_sock);
        }

        ~Connection() {}

    public:
        int _sock;
        std::string _inbuffer;  // 输入缓存区
        std::string _outbuffer; // 输出缓存区

        func_t _recver;   // 读方法
        func_t _sender;   // 写方法
        func_t _excepter; // 处理异常事件

        TcpServer *_tsp; // 回指针
    };

    class TcpServer
    {
    private:
        void Recver(Connection *con_ptr)
        {
            char buffer[1024];
            while (true)
            {
                ssize_t s = recv(con_ptr->_sock, buffer, sizeof(buffer) - 1, 0);
                if (s > 0)
                {
                    buffer[s] = 0;
                    con_ptr->_inbuffer += buffer; // 将读到的数据入队列
                    std::cout << con_ptr->_inbuffer << std::endl;
                    _service(con_ptr);
                }
                else if (s == 0)
                {
                    if (con_ptr->_excepter)
                    {
                        con_ptr->_excepter(con_ptr);
                        return;
                    }
                }
                else
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        break;
                    else if (errno == EINTR)
                        continue;
                    else
                    {
                        if (con_ptr->_excepter)
                        {
                            con_ptr->_excepter(con_ptr);
                            return;
                        }
                    }
                }
            }
        }

        void Sender(Connection *con_ptr)
        {
            while (true)
            {
                ssize_t s = send(con_ptr->_sock, con_ptr->_outbuffer.c_str(), con_ptr->_outbuffer.size(), 0);
                if (s >= 0)
                {
                    if (con_ptr->_outbuffer.empty())
                        break;
                    else
                        con_ptr->_outbuffer.erase(0, s);
                }
                else
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        break;
                    else if (errno == EINTR)
                        continue;
                    else
                    {
                        if (con_ptr->_excepter)
                        {
                            con_ptr->_excepter(con_ptr);
                            return;
                        }
                    }
                }
            }
            // 如果没有发送完毕，需要对对应的sock开启对写事件的关系， 如果发完了，我们要关闭对写事件的关心！
            if (!con_ptr->_outbuffer.empty())
                con_ptr->_tsp->EnableReadWrite(con_ptr, true, true);
            else
                con_ptr->_tsp->EnableReadWrite(con_ptr, true, false);
        }

        void Excepter(Connection *con_ptr)
        {
            logMessage(DEBUG, "Excepter begin");
            _epoller.Control(con_ptr->_sock, 0, EPOLL_CTL_DEL);
            con_ptr->Close();
            _connections.erase(con_ptr->_sock);
            logMessage(DEBUG, "关闭%d 文件描述符的所有的资源", con_ptr->_sock);
            delete con_ptr;
        }

        void Accepter(Connection *con_ptr)
        {
            for (;;)
            {
                std::string client_ip;
                uint16_t client_port;
                int err = 0;
                int sock = _sock.Accept(&client_ip, &client_port, &err);
                if (sock > 0)
                {
                    AddConnection(sock, EPOLLIN | EPOLLET,
                                  std::bind(&TcpServer::Recver, this, std::placeholders::_1),
                                  std::bind(&TcpServer::Sender, this, std::placeholders::_1),
                                  std::bind(&TcpServer::Excepter, this, std::placeholders::_1));
                    logMessage(DEBUG, "get a new link, info: [%s:%d]", client_ip.c_str(), client_port);
                }
                else
                {
                    if (err == EAGAIN || err == EWOULDBLOCK)
                        break;
                    else if (err == EINTR)
                        continue;
                    else
                        break;
                }
            }
        }

        void AddConnection(int sock, uint32_t events, func_t recver, func_t sender, func_t excepter)
        {
            // 1.为sock创建connection对象,初始化，添加到_connections中
            if (events & EPOLLET)
                Util::SetNonBlock(_sock.Fd()); // 设置非阻塞
            Connection *con_ptr = new Connection(sock, this);

            // 2.给sock设置回调方法
            con_ptr->Register(recver, sender, excepter);

            // 3.将sock关心事件'写透式'注册到epoll中
            bool ret = _epoller.AddEvent(sock, events);
            assert(ret);
            (void)ret;

            // 4.将kv添加到_connections
            _connections.insert(std::pair<int, Connection *>(sock, con_ptr));
            logMessage(DEBUG, "add new sock : %d in epoll and unordered_map", sock);
        }

        bool IsConnectionExists(int sock)
        {
            auto iter = _connections.find(sock);
            return iter != _connections.end();
        }

        void Loop(int timeout)
        {
            int n = _epoller.Wait(_revs, _num, timeout);
            for (int i = 0; i < n; i++)
            {
                int sock = _revs[i].data.fd;
                uint32_t events = _revs[i].events;

                // 将所有的异常问题，全部转化 成为读写问题
                if (events & EPOLLERR)
                    events |= (EPOLLIN | EPOLLOUT);
                if (events & EPOLLHUP)
                    events |= (EPOLLIN | EPOLLOUT);

                if ((events & EPOLLIN) && IsConnectionExists(sock) && _connections[sock]->_recver)
                    _connections[sock]->_recver(_connections[sock]);
                if ((events & EPOLLOUT) && IsConnectionExists(sock) && _connections[sock]->_sender)
                    _connections[sock]->_sender(_connections[sock]);
            }
        }

    public:
        TcpServer(func_t func, uint16_t port = default_port) : _service(func), _port(port), _revs(nullptr), _num(num) {}

        void InitServer()
        {
            // 1.创建套接字
            _sock.Socket();
            _sock.Bind(_port);
            _sock.Listen();
            // 2.创建epoll模型
            _epoller.Create();
            // 3.为sock创建connection对象
            AddConnection(_sock.Fd(), EPOLLIN | EPOLLET,
                          std::bind(&TcpServer::Accepter, this, std::placeholders::_1), nullptr, nullptr);

            _revs = new struct epoll_event[_num];
        }

        void EnableReadWrite(Connection *con_ptr, bool readable, bool writeable)
        {
            uint32_t event = (readable ? EPOLLIN : 0) | (writeable ? EPOLLOUT : 0) | EPOLLET;
            _epoller.Control(con_ptr->_sock, event, EPOLL_CTL_MOD);
        }

        // 事件派发器
        void Dispatcher()
        {
            int timeout = -1;
            while (true)
            {
                Loop(timeout);
            }
        }

        ~TcpServer()
        {
            _sock.Close();
            _epoller.Close();
            if (_revs)
                delete[] _revs;
        }

    private:
        uint16_t _port;
        Sock _sock;
        Epoller _epoller;
        std::unordered_map<int, Connection *> _connections;
        struct epoll_event *_revs;
        int _num;
        func_t _service;
    };
}
