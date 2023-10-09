#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "protocol.hpp"

#define NUM 1024

namespace client
{
    class CalClient
    {
    public:
        CalClient(const std::string serverip, const uint16_t serverport)
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
                std::string line;
                std::string inbuffer;
                while (true)
                {
                    std::cout << "calculator>>> ";
                    std::getline(std::cin, line);

                    Request req = Parseline(line);
                    std::string content;
                    req.serialize(&content);
                    std::string send_str = enLength(content);
                    send(_sock, send_str.c_str(), send_str.size(), 0);

                    std::string package, text;
                    if (!recvPackage(_sock, inbuffer, &package))
                        continue;
                    if (!deLength(package, &text))
                        continue;
                    Response resp;
                    resp.deserialize(text);
                    std::cout << "exitCode: " << resp._exitCode << std::endl;
                    std::cout << "result: " << resp._result << std::endl;
                }
            }
        }

        ~CalClient() {}

    private:
        Request Parseline(const std::string &line)
        {
            // 简易版状态机
            int status = 0; // 0 操作符之前， 1 操作符， 2 操作符之后
            int i = 0, len = line.size();
            std::string left, right;
            char op;
            while (i < len)
            {
                switch (status)
                {
                case 0:
                {
                    if (!isdigit(line[i]))
                    {
                        op = line[i];
                        status = 1;
                    }
                    else
                    {
                        left.push_back(line[i++]);
                    }
                }
                break;
                case 1:
                    i++;
                    status = 2;
                    break;
                case 2:
                    right.push_back(line[i++]);
                    break;
                }
            }
            return Request(std::stoi(left), std::stoi(right), op);
        }

    private:
        int _sock;
        std::string _serverip;
        uint16_t _serverport;
    };
}