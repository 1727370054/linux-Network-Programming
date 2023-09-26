#pragma once

#include <iostream>
#include <cstdio>
#include <functional>

const std::string oper = "+-*/";

int math(int x, int y, char op)
{
    int result = 0;
    switch (op)
    {
    case '+':
        result = x + y;
        break;
    case '-':
        result = x - y;
        break;
    case '*':
        result = x * y;
        break;
    case '/':
    {
        if (y == 0)
        {
            std::cerr << "div zero error" << std::endl;
            return result = -1;
        }
        result = x / y;
    }
    break;
    case '%':
    {
        if (y == 0)
        {
            std::cerr << "mod zero error" << std::endl;
            return result = -1;
        }
        result = x % y;
    }
    break;
    default:
        break;
    }
    return result;
}

class Task
{
    using fun_t = std::function<int(int, int, char)>;

public:
    Task() {}
    Task(int x, int y, char op, fun_t func) : _x(x), _y(y), _op(op), _callback(func) {}
    std::string operator()()
    {
        int result = _callback(_x, _y, _op);
        char buffer[1024];
        snprintf(buffer, sizeof buffer, "%d %c %d = %d", _x, _op, _y, result);
        return buffer;
    }

    std::string toTaskString()
    {
        char buffer[1024];
        snprintf(buffer, sizeof buffer, "%d %c %d = ?", _x, _op, _y);
        return buffer;
    }

private:
    int _x;
    int _y;
    char _op;
    fun_t _callback;
};
