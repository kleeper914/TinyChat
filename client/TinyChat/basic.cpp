#include "basic.h"

int getSequence()
{
    static int num = 0;
    if(num++ >= 0xFFFFFFFF - 1)
    {
        num = 0;
    }
    return num;
}
