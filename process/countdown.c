#include <stdio.h>
#include <unistd.h>

int main()
{
    int i = 10;
    while (i > 0)
    {
        printf("倒计时：%2d\r", i);
        fflush(stdout);
        i--;
        sleep(1);
    }

    return 0;
}
