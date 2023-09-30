#include "udpServer.hpp"
#include "onlineUser.hpp"
#include <unordered_map>
#include <memory>
#include <fstream>
#include <cstdio>
#include <strings.h>
#include <signal.h>

using namespace udpServer;

static void Usage(std::string proc)
{
    std::cout << "\nUsage:\n\t" << proc << " local_port\n\n";
}

/////////////////////////////////////////////////////字典业务////////////////////////////////////////////
static const string dictTXT = "./dict.txt";
static unordered_map<string, string> dict;

static bool cutString(const string &target, string *s1, string *s2, const string &sep)
{
    // apple:苹果
    auto pos = target.find(sep);
    if (pos == string::npos)
        return false;
    *s1 = target.substr(0, pos);
    *s2 = target.substr(pos + sep.size());
    return true;
}

static void initDict()
{
    ifstream in(dictTXT, std::ios::binary);
    if (!in.is_open())
    {
        cout << "open file " << dictTXT << " error" << endl;
        exit(OPEN_ERR);
    }
    string line;
    string key, value;
    while (getline(in, line))
    {
        if (cutString(line, &key, &value, ":"))
        {
            dict.insert(make_pair(key, value));
        }
    }
    in.close();
    cout << "load dict success" << endl;
}

void reload(int signo)
{
    (void)signo;
    initDict();
}

static void debugPrint()
{
    for (auto &iter : dict)
    {
        cout << iter.first << " # " << iter.second << endl;
    }
}

void handlermessage(int sockfd, string clientIp, uint16_t clientPort, string message)
{
    string response_message;
    auto iter = dict.find(message);
    if (iter == dict.end())
        response_message = "unknown";
    else
        response_message = iter->second;
    // 开始返回
    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(clientPort);
    client.sin_addr.s_addr = inet_addr(clientIp.c_str());
    sendto(sockfd, response_message.c_str(), response_message.size(), 0, (struct sockaddr *)&client, sizeof(client));
}
/////////////////////////////////////////////////////字典业务 done////////////////////////////////////////////

////////////////////////////////////////////////////远程命令行解析////////////////////////////////////////////
void execCommand(int sockfd, string clientIp, uint16_t clientPort, string cmd)
{
    if (cmd.find("rm") != string::npos || cmd.find("mv") != string::npos || cmd.find("rmdir") != string::npos)
    {
        cerr << clientIp << ":" << clientPort << " 正在做一个非法的操作: " << cmd << endl;
        return;
    }

    string response;
    FILE *fp = popen(cmd.c_str(), "r");
    if (fp == nullptr)
        response = cmd + "failed";
    else
    {
        char line[1024];
        while (fgets(line, sizeof(line), fp))
        {
            response += line;
        }
    }
    pclose(fp);
    // 开始返回
    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(clientPort);
    client.sin_addr.s_addr = inet_addr(clientIp.c_str());
    sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr *)&client, sizeof(client));
}
/////////////////////////////////////////////////远程命令行解析 done////////////////////////////////////////////

/////////////////////////////////////////////////路由转接/////////////////////////////////////////////////////
onlineUser onlineuser;
void routeMessage(int sockfd, string clientIp, uint16_t clientPort, string message)
{
    if (message == "online")
        onlineuser.addUser(clientIp, clientPort);
    else if (message == "offline")
        onlineuser.delUser(clientIp, clientPort);

    if (onlineuser.isOnline(clientIp, clientPort))
    {
        // 消息路由
        onlineuser.broadcastMessage(sockfd, clientIp, clientPort, message);
    }
    else
    {
        struct sockaddr_in client;
        bzero(&client, sizeof(client));
        client.sin_family = AF_INET;
        client.sin_port = htons(clientPort);
        client.sin_addr.s_addr = inet_addr(clientIp.c_str());
        string response;
        if (message == "offline")
            response = "下线成功, 欢迎下次登陆!";
        else
            response = "你还没有上线，请先上线，运行指令: online";
        sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr *)&client, sizeof(client));
    }
}

int main(int argc, char *argv[])
{
    if (2 != argc)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port = atoi(argv[1]);
    // std::unique_ptr<Server> usvr(new Server(handlermessage, port));
    // initDict();
    // signal(2, reload);

    // std::unique_ptr<Server> usvr(new Server(execCommand, port));
    std::unique_ptr<Server> usvr(new Server(routeMessage, port));
    usvr->initServer();
    usvr->start();
    return 0;
}