#pragma once

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <functional>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace udpServer
{
    using namespace std;

    static const std::string defaultIp = "0.0.0.0";
    static const int gnum = 1024;
    enum
    {
        USAGE_ERR = 1,
        SOCKET_ERR,
        BIND_ERR,
        OPEN_ERR
    };

    class Server
    {
        typedef function<void(int, string, uint16_t, string)> func_t;

    public:
        Server(const func_t callback, const uint16_t &port, const std::string &ip = defaultIp)
            : _callback(callback), _port(port), _ip(ip), _sockfd(-1) {}
        void initServer()
        {
            // 1. 创建 socket
            _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (_sockfd == -1)
            {
                std::cerr << "socket error: " << errno << strerror(errno) << std::endl;
                exit(SOCKET_ERR);
            }
            std::cout << "socket success"
                      << " : " << _sockfd << std::endl;
            // 2. 判定bind (绑定port，ip)
            struct sockaddr_in local;
            bzero(&local, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            // local.sin_addr.s_addr = htonl(INADDR_ANY); // 任意地址bind，服务器的真实写法
            local.sin_addr.s_addr = inet_addr(_ip.c_str()); // 1. string->uint32_t 2. htonl(); -> inet_addr
            int n = bind(_sockfd, (struct sockaddr *)&local, sizeof(local));
            if (n == -1)
            {
                std::cerr << "socket error: " << errno << strerror(errno) << std::endl;
                exit(BIND_ERR);
            }
        }
        void start()
        {
            // 服务器的本质其实就是一个死循环
            char buffer[gnum];
            for (;;)
            {
                // 读取数据
                struct sockaddr_in peer;
                socklen_t length = sizeof(peer);
                ssize_t s = recvfrom(_sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&peer, &length);
                if (s > 0)
                {
                    buffer[s] = 0;
                    std::string clientIp = inet_ntoa(peer.sin_addr); // 1. 网络序列 2. int->点分十进制IP
                    uint16_t clientPort = ntohs(peer.sin_port);
                    std::string message = buffer;
                    std::cout << clientIp << "[" << clientPort << "]# " << message << std::endl;
                    // 执行回调方法
                    _callback(_sockfd, clientIp, clientPort, message);
                }
            }
        }
        ~Server() {}

    private:
        std::string _ip; // 实际上，一款网络服务器，不建议指明一个IP
        uint16_t _port;
        int _sockfd;
        func_t _callback;
    };
}