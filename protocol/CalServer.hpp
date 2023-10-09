#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#include "log.hpp"
#include "protocol.hpp"

#define NUM 1024

namespace server
{
    enum
    {
        USAGE_ERR = 1,
        SOCKET_ERR,
        BIND_ERR,
        LISTEN_ERR
    };

    static const uint16_t gport = 8080;
    static const int gbacklog = 5;

    using func_t = std::function<void(const Request &req, Response &resp)>;
    void handlerEnter(int sock, func_t func)
    {
        std::string inbuffer;
        while (true)
        {
            // 1. 读取："content_len"\r\n"x op y"\r\n
            std::string read_text, req_str;
            if (!recvPackage(sock, inbuffer, &read_text))
                return;
            std::cout << "带报头的请求：\n"
                      << read_text << std::endl;
            // 1.1 我们保证，我们read_text里面一定是一个完整的请求："content_len"\r\n"x op y"\r\n
            if (!deLength(read_text, &req_str))
                return;
            std::cout << "去掉报头的正文：\n"
                      << req_str << std::endl;
            // 2. 对请求Request，反序列化
            Request req;
            if (!req.deserialize(req_str))
                return;

            // 3. 计算机处理，req.x, req.op, req.y --- 业务逻辑
            // 3.1 得到一个结构化的响应
            Response resp;
            func(req, resp);

            // 4.对响应Response，进行序列化
            // 4.1 得到了一个"字符串"
            std::string resp_str;
            resp.serialize(&resp_str);

            std::cout << "计算完成, 序列化响应: " << resp_str << std::endl;
            // 5. 然后我们在发送响应
            // 5.1 构建成为一个完整的报文
            std::string send_str = enLength(resp_str);
            std::cout << "构建完成完整的响应\n"
                      << send_str << std::endl;
            send(sock, send_str.c_str(), send_str.size(), 0);
        }
    }

    class CalServer
    {
    public:
        CalServer(const uint16_t &port = gport) : _listensock(-1), _port(port) {}

        void initServer()
        {
            // 1. 创建socket
            _listensock = socket(AF_INET, SOCK_STREAM, 0);
            if (_listensock < 0)
            {
                logMessage(FATAL, "create socket error");
                exit(SOCKET_ERR);
            }
            logMessage(NORMAL, "create socket success: %d", _listensock);

            // 2.bind自己的网络信息
            struct sockaddr_in local;
            memset(&local, 0, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            local.sin_addr.s_addr = INADDR_ANY;
            if (bind(_listensock, (struct sockaddr *)&local, sizeof(local)) < 0)
            {
                logMessage(FATAL, "bind socket error");
                exit(BIND_ERR);
            }
            logMessage(NORMAL, "bind socket success");

            // 3.设置socket为监听状态
            if (listen(_listensock, gbacklog) < 0)
            {
                logMessage(FATAL, "listen socket error");
                exit(LISTEN_ERR);
            }
            logMessage(NORMAL, "listen socket success");
        }

        void start(func_t func)
        {
            // signal(SIGCHLD, SIG_IGN);
            for (;;)
            {
                // 4.server获取新链接
                // sock 用来和client通信的 fd
                struct sockaddr_in peer;
                socklen_t len = sizeof(peer);
                int sock = accept(_listensock, (struct sockaddr *)&peer, &len);
                if (sock < 0)
                {
                    logMessage(ERROR, "accept error, next");
                    continue;
                }
                logMessage(NORMAL, "accept a new link success, get new sock: %d", sock);

                // version 2 多进程
                pid_t id = fork();
                if (id == 0)
                {
                    close(_listensock);
                    // if (fork() > 0)
                    //     exit(0);
                    handlerEnter(sock, func);
                    close(sock);
                    exit(0);
                }
                close(sock);
                pid_t ret = waitpid(id, nullptr, 0);
                if (ret > 0)
                    logMessage(NORMAL, "wait child success");
            }
        }

        ~CalServer() {}

    private:
        int _listensock; // 不是用来进行数据通信的，它是用来监听链接到来，获取新链接的！
        uint16_t _port;
    };
}