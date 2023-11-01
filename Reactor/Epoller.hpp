#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include "Log.hpp"
#include "Error.hpp"

class Epoller
{
    static const int default_fd = -1;
    static const int size = 128;

public:
    Epoller() : _epfd(default_fd) {}
    ~Epoller() {}

public:
    void Create()
    {
        _epfd = epoll_create(size);
        if (_epfd < 0)
        {
            logMessage(FATAL, "epoll_create error, code: %d, errstring: %s", errno, strerror(errno));
            exit(EPOLL_CREATE_ERR);
        }
    }

    bool AddEvent(int sock, uint32_t events)
    {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = sock;
        int n = epoll_ctl(_epfd, EPOLL_CTL_ADD, sock, &ev);

        return n == 0;
    }

    int Wait(struct epoll_event revs[], int num, int timeout)
    {
        int n = epoll_wait(_epfd, revs, num, timeout);
        return n;
    }

    bool Control(int sock, uint32_t event, int action)
    {
        int n = 0;
        if (action == EPOLL_CTL_MOD)
        {
            struct epoll_event ev;
            ev.data.fd = sock;
            ev.events = event;
            n = epoll_ctl(_epfd, action, sock, &ev);
        }
        else if (action == EPOLL_CTL_DEL)
        {
            n = epoll_ctl(_epfd, action, sock, nullptr);
        }
        else
        {
            n = -1;
        }

        return n == 0;
    }

    void Close()
    {
        if (_epfd != default_fd)
            close(_epfd);
    }

private:
    int _epfd;
};