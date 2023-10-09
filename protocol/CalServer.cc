#include "CalServer.hpp"
#include <iostream>
#include <memory>

using namespace server;
using namespace std;

static void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " local_port\n\n";
}

bool calculator(const Request &req, Response &resp)
{
    resp._exitCode = OK;
    resp._result = OK;

    switch (req._op)
    {
    case '+':
        resp._result = req._x + req._y;
        break;
    case '-':
        resp._result = req._x - req._y;
        break;
    case '*':
        resp._result = req._x * req._y;
        break;
    case '/':
    {
        if (req._y == 0)
            resp._exitCode = DIV_ZERO;
        else
            resp._result = req._x / req._y;
    }
    break;
    case '%':
    {
        if (req._y == 0)
            resp._exitCode = MOD_ZERO;
        else
            resp._result = req._x % req._y;
    }
    break;
    default:
        resp._exitCode = OP_ERROR;
        break;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port = atoi(argv[1]);
    unique_ptr<CalServer> tsvr(new CalServer(port));
    tsvr->initServer();
    tsvr->start(calculator);
    return 0;
}