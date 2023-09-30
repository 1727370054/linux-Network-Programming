#include "udpClient.hpp"
#include <memory>

using namespace udpClient;

static void Usage(std::string proc)
{
    std::cerr << "\nUsage:\n\t" << proc << " server_ip server_port\n\n";
}

// ./udpClient server_ip server_port
int main(int argc, char *argv[])
{
    if (3 != argc)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }
    std::string serverIp = argv[1];
    uint16_t serverPort = atoi(argv[2]);
    std::unique_ptr<Client> ucli(new Client(serverIp, serverPort));
    ucli->initClient();
    ucli->run();
    return 0;
}