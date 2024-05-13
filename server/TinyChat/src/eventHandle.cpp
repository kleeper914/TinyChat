#include "../include/eventHandle.h"
#include "../include/msgDef.h"

void* event_handle(void* arg)
{
    printf("new connected client.................\n");
    int clnt_sock = *((int*)arg);
    free(arg); arg = NULL;      //avoid wild pointer
    char recv_msg[1024*2];
    int recv_len;
    int head_len = sizeof(messageHead);

    Task task(clnt_sock);
    while(1)
    {
        if(task.readEvent() == READ_EXIT)
        {
            close(clnt_sock);
            LOGINFO("client %d exit\n", clnt_sock);
            return NULL;
        }
    }
    return NULL;
}