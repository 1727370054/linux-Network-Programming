#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

namespace udpClient
{
    using namespace std;

    enum
    {
        USAGE_ERR = 1,
        SOCKET_ERR
    };
    class Client
    {
    public:
        Client(const std::string &serverIp, const uint16_t &sreverPort)
            : _serverIp(serverIp), _serverPort(sreverPort), _sockfd(-1), _quit(false) {}

        void initClient()
        {
            // 创建 socket
            _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (_sockfd == -1)
            {
                std::cerr << "socket error: " << errno << strerror(errno) << std::endl;
                exit(SOCKET_ERR);
            }
            std::cout << "socket success"
                      << " : " << _sockfd << std::endl;
        }
        static void *readMessage(void *args)
        {
            int sockfd = *(static_cast<int *>(args));
            pthread_detach(pthread_self());
            while (true)
            {
                char buffer[1024];
                struct sockaddr_in temp;
                socklen_t len = sizeof(temp);
                ssize_t s = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&temp, &len);
                if (s >= 0)
                    buffer[s] = 0;
                cout << "\n"
                     << buffer << endl;
            }
        }

        void run()
        {
            pthread_create(&_reader, nullptr, readMessage, (void *)&_sockfd);
            struct sockaddr_in server;
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = inet_addr(_serverIp.c_str());
            server.sin_port = htons(_serverPort);

            std::string message;
            char cmdline[1024];
            while (!_quit)
            {
                // std::cout << " Please Enter# ";
                // std::cin >> message;
                // cout << "[hwk@VM-12-12-centos XXXX]$ ";
                fprintf(stderr, "Enter# ");
                fflush(stderr);
                fgets(cmdline, sizeof(cmdline), stdin);
                cmdline[strlen(cmdline) - 1] = 0; // "xxxx\n\0"
                message = cmdline;

                sendto(_sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&server, sizeof(server));
            }
        }
        ~Client() {}

    private:
        int _sockfd;
        std::string _serverIp;
        uint16_t _serverPort;
        bool _quit;

        pthread_t _reader;
    };
}