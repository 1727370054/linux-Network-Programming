#include "HttpServer.hpp"
#include "Protocol.hpp"
#include <iostream>
#include <memory>

using namespace std;
using namespace server;

void Usage(std::string proc)
{
    cerr << "Usage:\n\t" << proc << " port\r\n\r\n";
}

std::string suffixToDescribe(const std::string &suffix)
{
    std::string ct = "Content-Type: ";
    if (suffix == ".html")
        ct += "text/html";
    else if (suffix == ".jpg")
        ct += "image/jpeg";
    else if (suffix == ".png")
        ct += "image/png";
    else if (suffix == ".css")
        ct += "text/css";
    else if (suffix == ".js")
        ct += "application/x-javascript";
    else if (suffix == "svg")
        ct += "text/xml";
    ct += "\r\n";
    return ct;
}

bool Get(const HttpRequest &req, HttpResponse &resp)
{
    cout << "----------------------http start---------------------------" << endl;
    cout << req.inbuffer << endl;
    std::cout << "method: " << req.method << std::endl;
    std::cout << "url: " << req.url << std::endl;
    std::cout << "httpversion: " << req.http_version << std::endl;
    std::cout << "path: " << req.path << std::endl;
    std::cout << "suffix: " << req.suffix << std::endl;
    std::cout << "size: " << req.size << " 字节" << std::endl;
    cout << "----------------------http end-----------------------------" << endl;

    std::string respline = "http/1.1 200 OK\r\n";
    // 重定向测试
    // std::string respline = "http/1.1 307 Temporary Redirect\r\n";
    std::string respheader = suffixToDescribe(req.suffix);
    std::string respblank = "\r\n";

    respheader += "Set-Cookie: name=123456789abcd; Max-Age=120\r\n";

    // respheader += "Location: https://gitee.com/huang-wankun";
    // std::string body = "<html lang=\"en\"><head><meta charset=\"UTF-8\"><title>for test</title><h1>hello world</h1></head><body><p>北京交通广播《一路畅通》“交通大家谈”节目，特邀北京市交通委员会地面公交运营管理处处长赵震、北京市公安局公安交通管理局秩序处副处长 林志勇、北京交通发展研究院交通规划所所长 刘雪杰为您解答公交车专用道6月1日起社会车辆进出公交车道须注意哪些？</p></body></html>";
    std::string body;
    body.resize(req.size + 1);
    if (!Util::readFile(req.path, (char *)body.c_str(), req.size))
    {
        Util::readFile(html_404, (char *)body.c_str(), req.size);
    }

    if (req.size > 0)
    {
        respheader += "Content-Length: ";
        respheader += std::to_string(body.size());
        respheader += "\r\n";
    }

    resp.outbuffer += respline;
    resp.outbuffer += respheader;
    resp.outbuffer += respblank;
    // cout << "----------------------http response start---------------------------" << endl;
    // std::cout << resp.outbuffer << std::endl;
    // cout << "----------------------http response end-----------------------------" << endl;
    resp.outbuffer += body;

    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(0);
    }
    uint16_t port = atoi(argv[1]);

    unique_ptr<HttpServer> httpsvr(new HttpServer(Get, port));
    httpsvr->initServer();
    httpsvr->start();

    return 0;
}