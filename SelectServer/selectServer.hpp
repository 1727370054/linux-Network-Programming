#pragma once

#include <iostream>
#include <string>
#include <sys/select.h>
#include <functional>
#include "socket.hpp"

namespace select_ns
{
    static const int default_port = 8080;
    static const int fd_num = sizeof(fd_set) * 8;
    static const int default_fd = -1;
    static const int default_val = -1;

    using func_t = std::function<std::string(const std::string &)>;

    class SelectServer
    {
    public:
        SelectServer(func_t func, uint16_t port = default_port) : _func(func), _port(port), _listen_sock(default_val), _fd_array(nullptr) {}

        void initServer()
        {
            _listen_sock = Sock::Socket();
            Sock::Bind(_listen_sock, _port);
            Sock::Listen(_listen_sock);

            _fd_array = new int[fd_num];
            for (int i = 0; i < fd_num; i++)
                _fd_array[i] = default_fd;

            _fd_array[0] = _listen_sock;
        }

        void Print()
        {
            std::cout << "fd list: ";
            for (int i = 0; i < fd_num; i++)
            {
                if (_fd_array[i] != default_fd)
                    std::cout << _fd_array[i] << " ";
            }
            std::cout << std::endl;
        }

        void Accepter(int listen_sock)
        {
            logMessage(DEBUG, "Accepter in");
            std::string client_ip;
            uint16_t client_port;
            int sock = Sock::Accept(_listen_sock, &client_ip, &client_port);
            if (sock < 0)
                return;
            logMessage(NORMAL, "accept success %s:%d", client_ip.c_str(), client_port);
            // 将新的sock 托管给select！
            int i = 0;
            for (; i < fd_num; i++)
            {
                if (_fd_array[i] != default_fd)
                    continue;
                else
                    break;
            }
            if (i == fd_num)
            {
                logMessage(WARNING, "server if full, please wait");
                close(sock);
            }
            else
            {
                _fd_array[i] = sock;
            }
            Print();
            logMessage(DEBUG, "Accepter out");
        }

        void Recver(int sock, int pos)
        {
            logMessage(DEBUG, "Recver in");

            char buffer[1024];
            ssize_t s = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (s > 0)
            {
                buffer[s] = 0;
                logMessage(NORMAL, "client# %s", buffer);
            }
            else if (s == 0)
            {
                close(sock);
                _fd_array[pos] = default_fd;
                logMessage(NORMAL, "client quit");
                return;
            }
            else
            {
                close(sock);
                _fd_array[pos] = default_fd;
                logMessage(ERROR, "client quit: %s", strerror(errno));
                return;
            }

            // 2.处理request
            std::string response = _func(buffer);

            // 3. 返回response
            // bug
            write(sock, response.c_str(), response.size());
            logMessage(DEBUG, "Recver out");
        }

        void handlerReadEvent(fd_set &readfds)
        {
            for (int i = 0; i < fd_num; i++)
            {
                if (_fd_array[i] == default_fd)
                    continue;

                if (FD_ISSET(_fd_array[i], &readfds) && _fd_array[i] == _listen_sock)
                    Accepter(_listen_sock);
                else if (FD_ISSET(_fd_array[i], &readfds))
                    Recver(_fd_array[i], i);
                else
                {
                    // TODO
                }
            }
        }

        void start()
        {
            for (;;)
            {
                fd_set readfds;
                FD_ZERO(&readfds);
                int max_fd = _fd_array[0];

                for (int i = 0; i < fd_num; i++)
                {
                    if (_fd_array[i] == default_fd)
                        continue;
                    FD_SET(_fd_array[i], &readfds); // 合法 fd 全部添加到读文件描述符集中
                    if (max_fd < _fd_array[i])
                        max_fd = _fd_array[i]; // 更新所有fd中最大的fd
                }

                // struct timeval timeout = {1, 0};
                // int n = select(_listen_sock + 1, &readfds, nullptr, nullptr, nullptr);
                // 一般而言，要是用select，需要程序员自己维护一个保存所有合法fd的数组！
                int n = select(max_fd + 1, &readfds, nullptr, nullptr, nullptr);

                switch (n)
                {
                case 0:
                    logMessage(NORMAL, "timeout...");
                    break;
                case -1:
                    logMessage(WARNING, "select error, code: %d, err string: %s", errno, strerror(errno));
                    break;
                default:
                    // 说明有事件就绪了,目前只有一个监听事件就绪了
                    logMessage(NORMAL, "have evnet ready!");
                    handlerReadEvent(readfds);
                    break;
                }
            }
        }

        ~SelectServer()
        {
            if (_listen_sock != default_val)
                close(_listen_sock);
            if (_fd_array)
                delete[] _fd_array;
        }

    private:
        uint16_t _port;
        int _listen_sock;
        int *_fd_array;
        func_t _func;
    };
}