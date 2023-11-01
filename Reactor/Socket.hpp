#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Log.hpp"
#include "Error.hpp"

class Sock
{
    const static int backlog = 32;
    const static int default_fd = -1;

public:
    Sock() : _listen_sock(default_fd) {}
    ~Sock() {}

public:
    void Socket()
    {
        _listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (_listen_sock < 0)
        {
            logMessage(FATAL, "create socket error");
            exit(SOCKET_ERR);
        }
        logMessage(NORMAL, "create socket success: %d", _listen_sock);

        int opt = 1;
        setsockopt(_listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    }

    void Bind(int port)
    {
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        local.sin_addr.s_addr = INADDR_ANY;
        if (bind(_listen_sock, (struct sockaddr *)&local, sizeof(local)) < 0)
        {
            logMessage(FATAL, "bind socket error");
            exit(BIND_ERR);
        }
        logMessage(NORMAL, "bind socket success");
    }

    void Listen()
    {
        if (listen(_listen_sock, backlog) < 0)
        {
            logMessage(FATAL, "listen socket error");
            exit(LISTEN_ERR);
        }
        logMessage(NORMAL, "listen socket success");
    }

    int Accept(std::string *client_ip, uint16_t *client_port, int *err)
    {
        struct sockaddr_in peer;
        socklen_t length = sizeof(peer);
        int sock = accept(_listen_sock, (struct sockaddr *)&peer, &length);
        *err = errno;
        if (sock < 0)
        {
            // TODO
        }
        else
        {
            logMessage(NORMAL, "accept a new link success, get new sock: %d", sock);
            *client_ip = inet_ntoa(peer.sin_addr);
            *client_port = ntohs(peer.sin_port);
        }

        return sock;
    }

    int Fd()
    {
        return _listen_sock;
    }

    void Close()
    {
        if (_listen_sock != default_fd)
            close(_listen_sock);
    }

private:
    int _listen_sock;
};