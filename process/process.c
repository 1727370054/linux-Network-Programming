#include "process.h"

char style[S_NUM] = {'-', '#', '*', '>', '+'};

void ProcessOn()
{
    int cnt = 0;
    char bar[NUM];
    memset(bar, '\0', sizeof(bar)); //初始化bar数组

    const char* lable = "|/-\\";

    while (cnt <= 100)
    {
       // printf("[%-100s][%d%%][%c]\r", bar, cnt, lable[cnt%4]);
        //修改背景颜色        
        printf("\033[0m[%-100s][%d%%][%c]\033[0m\r", bar, cnt, lable[cnt%4]);
        fflush(stdout);
        bar[cnt++] = style[N];
        usleep(50000); //5 s : 5 / 100 = 0.05s = 50000ms 
    }

    printf("\n");

    return;
}
