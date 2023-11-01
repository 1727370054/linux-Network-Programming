#include "TcpServer.hpp"
#include "Protocol.hpp"
#include <memory>

using namespace std;
using namespace tcpserver;

static void usage(std::string proc)
{
    std::cerr << "Usage:\n\t" << proc << " port"
              << "\n\n";
}

bool cal(const Request &req, Response &resp)
{
    // req已经有结构化完成的数据啦，你可以直接使用
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

    return true;
}

void calculate(Connection *con_ptr)
{
    std::string onePackage;
    while (ParseOnePackage(con_ptr->_inbuffer, &onePackage))
    {
        std::string reqStr;
        if (!deLength(onePackage, &reqStr))
            return;
        std::cout << "去掉报头的正文：\n"
                  << reqStr << std::endl;
        // 2. 对请求Request，反序列化
        Request req;
        if (!req.deserialize(reqStr))
            return;
        // 3. 计算机处理，req.x, req.op, req.y --- 业务逻辑
        // 3.1 得到一个结构化的响应
        Response resp;
        cal(req, resp);
        // 4.对响应Response，进行序列化
        // 4.1 得到了一个"字符串"
        std::string respStr;
        resp.serialize(&respStr);
        std::cout << "计算完成, 序列化响应: " << respStr << std::endl;
        // 5.添加报头，构建响应
        con_ptr->_outbuffer += enLength(respStr);
    }
    // 直接发
    if (con_ptr->_sender)
        con_ptr->_sender(con_ptr);

    // // 如果没有发送完毕，需要对对应的sock开启对写事件的关系， 如果发完了，我们要关闭对写事件的关心！
    // if (!con_ptr->_outbuffer.empty())
    //     con_ptr->_tsp->EnableReadWrite(con_ptr, true, true);
    // else
    //     con_ptr->_tsp->EnableReadWrite(con_ptr, true, false);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port = atoi(argv[1]);
    unique_ptr<TcpServer> svr(new TcpServer(calculate, port));
    svr->InitServer();
    svr->Dispatcher();
    return 0;
}
