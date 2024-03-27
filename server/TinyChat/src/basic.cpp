#include "../include/basic.h"

void error_handling(const char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int getSequence()
{
    static int num = 0;
    if(num++ >= 0xFFFFFFFF - 1)
    {
        num = 0;
    }
    return num;
}

int getAccount()
{
    srand((unsigned int)time(NULL));
    int retAccount = 0;
    int min = 20000;
    int max = 30000;
    retAccount = (rand() % (max - min)) + min;
    return retAccount;
}