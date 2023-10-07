#pragma once

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEV "/dev/null"

void daemonSelf(const char *currPath = nullptr)
{
    // 1. 让调用进程忽略掉异常的信号
    signal(SIGPIPE, SIG_IGN);
    // 2. 让自己不是组长，setsid
    if (fork() > 0)
        exit(0); // 让父进程退出
    // 子进程 --- 守护进程（精灵进程）， 本质就是一个孤儿进程

    // 3. 守护进程是脱离终端的，关闭或者重定向以前进程默认打开的文件
    int fd = open(DEV, O_RDWR);
    if (fd >= 0)
    {
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);

        close(fd);
    }
    else
    {
        close(0);
        close(1);
        close(2);
    }

    // 进程执行路径发送改变
    if (currPath)
        chdir(currPath);
}