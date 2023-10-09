#include "CalClient.hpp"
#include <memory>

using namespace std;
using namespace client;

static void Usage(string proc)
{
    cout << "\nUsage:\n\t" << proc << " serverip serverport\n\n";
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        Usage(argv[0]);
        exit(1);
    }
    string serverip = argv[1];
    uint16_t serverport = atoi(argv[2]);
    unique_ptr<CalClient> tcli(new CalClient(serverip, serverport));
    tcli->initClient();
    tcli->start();
    return 0;
}