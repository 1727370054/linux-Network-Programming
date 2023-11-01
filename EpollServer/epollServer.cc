#include "epollServer.hpp"
#include "error.hpp"
#include <memory>

using namespace std;
using namespace epoll_ns;

static void usage(std::string proc)
{
    std::cerr << "Usage:\n\t" << proc << " port"
              << "\n\n";
}

std::string echo(const std::string &message)
{
    return "I am epollserver, " + message;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port = atoi(argv[1]);

    unique_ptr<EpollServer> epsvr(new EpollServer(echo));
    epsvr->initServer();
    epsvr->start();

    return 0;
}