#include "pollServer.hpp"
#include <iostream>
#include <memory>

using namespace std;
using namespace poll_ns;

static void usage(std::string proc)
{
    std::cerr << "Usage:\n\t" << proc << " port"
              << "\n\n";
}

std::string transaction(const std::string &request)
{
    return request;
}

int main(int argc, char *argv[])
{
    // if (argc != 2)
    // {
    //     usage(argv[0]);
    //     exit(USAGE_ERR);
    // }
    // unique_ptr<SelectServer> svr(new SelectServer(atoi(argv[1])));

    unique_ptr<PollServer> svr(new PollServer(transaction));

    svr->initServer();
    svr->start();
    return 0;
}