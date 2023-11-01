#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <sys/select.h>
#include <poll.h>
#include "socket.hpp"

namespace poll_ns
{
    static const int default_port = 8080;
    static const int num = 2048;
    static const int default_fd = -1;
    static const int default_val = -1;

    using func_t = std::function<std::string(const std::string &)>;

    class PollServer
    {
    public:
        PollServer(func_t func, uint16_t port = default_port) : _func(func), _port(port), _listen_sock(default_val), _read_pollfds(nullptr) {}

        void initServer()
        {
            _listen_sock = Sock::Socket();
            Sock::Bind(_listen_sock, _port);
            Sock::Listen(_listen_sock);

            _read_pollfds = new struct pollfd[num];
            for (int i = 0; i < num; i++)
                ResetItem(i);

            _read_pollfds[0].fd = _listen_sock;
            _read_pollfds[0].events = POLLIN;
        }

        void ResetItem(int i)
        {
            _read_pollfds[i].fd = default_fd;
            _read_pollfds[i].events = 0;
            _read_pollfds[i].revents = 0;
        }

        void Print()
        {
            std::cout << "fd list: ";
            for (int i = 0; i < num; i++)
            {
                if (_read_pollfds[i].fd != default_fd)
                    std::cout << _read_pollfds[i].fd << " ";
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
            for (; i < num; i++)
            {
                if (_read_pollfds[i].fd != default_fd)
                    continue;
                else
                    break;
            }
            if (i == num)
            {
                logMessage(WARNING, "server if full, please wait");
                close(sock);
            }
            else
            {
                _read_pollfds[i].fd = sock;
                _read_pollfds[i].events = POLLIN;
                _read_pollfds[i].revents = 0;
            }
            Print();
            logMessage(DEBUG, "Accepter out");
        }

        void Recver(int pos)
        {
            logMessage(DEBUG, "Recver in");

            char buffer[1024];
            ssize_t s = recv(_read_pollfds[pos].fd, buffer, sizeof(buffer) - 1, 0);
            if (s > 0)
            {
                buffer[s] = 0;
                logMessage(NORMAL, "client# %s", buffer);
            }
            else if (s == 0)
            {
                close(_read_pollfds[pos].fd);
                ResetItem(pos);
                logMessage(NORMAL, "client quit");
                return;
            }
            else
            {
                close(_read_pollfds[pos].fd);
                ResetItem(pos);
                logMessage(ERROR, "client quit: %s", strerror(errno));
                return;
            }

            // 2.处理request
            std::string response = _func(buffer);

            // 3. 返回response
            // bug
            write(_read_pollfds[pos].fd, response.c_str(), response.size());
            logMessage(DEBUG, "Recver out");
        }

        void handlerReadEvent()
        {
            logMessage(DEBUG, "handlerReadEvent in");
            for (int i = 0; i < num; i++)
            {
                if (_read_pollfds[i].fd == default_fd)
                    continue;
                if (!(_read_pollfds[i].events & POLLIN))
                    continue;

                if ((_read_pollfds[i].revents & POLLIN) && _read_pollfds[i].fd == _listen_sock)
                    Accepter(_listen_sock);
                else if (_read_pollfds[i].revents & POLLIN)
                    Recver(i);
                else
                {
                    // TODO
                }
            }
            logMessage(DEBUG, "handlerReadEvent out");
        }

        void start()
        {
            int timeout = -1;
            for (;;)
            {
                int n = poll(_read_pollfds, num, timeout);

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
                    handlerReadEvent();
                    break;
                }
            }
        }

        ~PollServer()
        {
            if (_listen_sock != default_val)
                close(_listen_sock);
            if (_read_pollfds)
                delete[] _read_pollfds;
        }

    private:
        uint16_t _port;
        int _listen_sock;
        struct pollfd *_read_pollfds;
        func_t _func;
    };
}