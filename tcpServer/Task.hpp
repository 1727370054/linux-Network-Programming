#pragma once

#include <iostream>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <functional>

#include "log.hpp"

#define NUM 1024

void serviceIO(int sock)
{
    char buffer[NUM];
    while (true)
    {
        ssize_t n = read(sock, buffer, sizeof(buffer) - 1);
        if (n > 0)
        {
            buffer[n] = 0;
            std::cout << "recv message# " << buffer << std::endl;

            std::string outBuffer;
            outBuffer = buffer;
            outBuffer += "[server-echo]";

            write(sock, outBuffer.c_str(), outBuffer.size());
        }
        else if (n == 0) // client端退出
        {
            logMessage(NORMAL, "client quit, me too");
            break;
        }
    }
    close(sock);
}

class Task
{
    using fun_t = std::function<void(int)>;

public:
    Task() {}
    Task(int sock, fun_t func) : _sock(sock), _callback(func) {}
    void operator()()
    {
        _callback(_sock);
    }

private:
    int _sock;
    fun_t _callback;
};
