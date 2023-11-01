#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <functional>
#include <unistd.h>
#include <sys/epoll.h>
#include "log.hpp"
#include "socket.hpp"

namespace epoll_ns
{
    static const uint16_t default_port = 8080;
    static const int default_val = -1;
    static const int size = 128;
    static const int default_num = 64;

    using func_t = std::function<std::string(const std::string &)>;

    class EpollServer
    {
    public:
        EpollServer(func_t func, uint16_t port = default_port, int num = default_num)
            : _func(func), _port(port), _listen_sock(default_val), _num(num), _revs(nullptr), _epfd(default_val) {}

        void initServer()
        {
            // 1.创建socket
            _listen_sock = Sock::Socket();
            Sock::Bind(_listen_sock, _port);
            Sock::Listen(_listen_sock);
            // 2.创建epoll模型
            _epfd = epoll_create(size);
            if (_epfd < 0)
            {
                logMessage(FATAL, "epoll_create error: %s", strerror(errno));
                exit(EPOLL_CREATE_ERR);
            }
            // 3.添加listensock到epoll模型中
            struct epoll_event ev;
            ev.events = EPOLLIN;
            ev.data.fd = _listen_sock; // 事件就绪就知道该事件对应的fd
            epoll_ctl(_epfd, EPOLL_CTL_ADD, _listen_sock, &ev);

            // 4.申请就绪事件空间
            _revs = new struct epoll_event[_num];

            logMessage(NORMAL, "init server success");
        }

        void HandlerEvent(int ready_num)
        {
            logMessage(DEBUG, "HandlerEvent in");
            for (int i = 0; i < ready_num; i++)
            {
                uint32_t events = _revs[i].events;
                int sock = _revs[i].data.fd;

                if (sock == _listen_sock && (events & EPOLLIN))
                {
                    //_listensock读事件就绪, 获取新连接
                    std::string client_ip;
                    uint16_t client_port;
                    int fd = Sock::Accept(_listen_sock, &client_ip, &client_port);
                    if (fd < 0)
                    {
                        logMessage(WARNING, "accept error");
                        continue;
                    }
                    // 将新获取的fd放入到epoll模型中
                    struct epoll_event ev;
                    ev.events = EPOLLIN;
                    ev.data.fd = fd;
                    epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev);
                }
                else if (events & EPOLLIN)
                {
                    // 普通的读事件就绪
                    // bug
                    char buffer[1024];
                    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
                    if (n > 0)
                    {
                        buffer[n] = 0;
                        logMessage(DEBUG, "client# %s", buffer);
                        std::string response = _func(buffer);
                        send(sock, response.c_str(), response.size(), 0);
                    }
                    else if (n == 0)
                    {
                        // 先把sock从epoll模型中移除，再close
                        epoll_ctl(_epfd, EPOLL_CTL_DEL, sock, nullptr);
                        close(sock);
                        logMessage(NORMAL, "client quit");
                    }
                    else
                    {
                        epoll_ctl(_epfd, EPOLL_CTL_DEL, sock, nullptr);
                        close(sock);
                        logMessage(ERROR, "recv error, code: %d, errstring: %s", errno, strerror(errno));
                    }
                }
                else
                {
                }
            }
            logMessage(DEBUG, "HandlerEvent out");
        }

        void start()
        {
            int timeout = -1;
            for (;;)
            {
                int n = epoll_wait(_epfd, _revs, _num, timeout);
                switch (n)
                {
                case 0:
                    logMessage(NORMAL, "timeout ...");
                    break;
                case -1:
                    logMessage(WARNING, "epoll_wait failed, code: %d, errstring: %s", errno, strerror(errno));
                    break;
                default:
                    logMessage(NORMAL, "have event ready");
                    HandlerEvent(n);
                    break;
                }
            }
        }

        ~EpollServer()
        {
            if (_listen_sock != default_val)
                close(_listen_sock);
            if (_epfd != default_val)
                close(_epfd);
            if (_revs)
                delete[] _revs;
        }

    private:
        uint16_t _port;
        int _listen_sock;
        int _epfd;
        struct epoll_event *_revs;
        int _num;
        func_t _func;
    };
}
