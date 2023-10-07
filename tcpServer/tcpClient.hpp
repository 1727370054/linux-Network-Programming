#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NUM 1024

namespace client
{
    class tcpClient
    {
    public:
        tcpClient(const std::string serverip, const uint16_t serverport)
            : _sock(-1), _serverip(serverip), _serverport(serverport) {}

        void initClient()
        {
            _sock = socket(AF_INET, SOCK_STREAM, 0);
            if (_sock < 0)
            {
                std::cerr << "socket create error" << std::endl;
                exit(2);
            }
        }

        void start()
        {
            struct sockaddr_in server;
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(_serverport);
            server.sin_addr.s_addr = inet_addr(_serverip.c_str());

            if (connect(_sock, (struct sockaddr *)&server, sizeof(server)) != 0)
            {
                std::cerr << "socket connect error" << std::endl;
            }
            else
            {
                std::string message;
                while (true)
                {
                    std::cout << "Enter# ";
                    std::getline(std::cin, message);
                    write(_sock, message.c_str(), message.size());

                    char buffer[NUM];
                    int n = read(_sock, buffer, sizeof(buffer) - 1);
                    if (n > 0)
                    {
                        buffer[n] = 0;
                        std::cout << "server react# " << buffer << std::endl;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        ~tcpClient() {}

    private:
        int _sock;
        std::string _serverip;
        uint16_t _serverport;
    };
}