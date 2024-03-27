#ifndef _BASIC_H_
#define _BASIC_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <map>
#include <json/json.h>
using namespace std;

#define IS_NULL(x) ((x)?(x):"")

#define LOGINFO(format, ...)                                                    \
    {                                                                           \
        printf("[ %s : %d ] [%s]>>" format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);  \
    }

/*
*   @brief  create a thread
*   @param  TID         id of the thread
*   @param  STACK_KB    sizeof the stack of the thread, how many KB
*   @param  JOIN        is separated from the main thread, true is separated, false is not
*   @param  FUNC        thread execution funcrion
*   @param  PARAM       parameter incomed
*   @param  NAME        name of the thread
*   @return int
*/
#define CREATE_THREAD(TID, STACK_KB, JOIN, FUNC, PARAM, NAME)\
    {   \
        char Thd_name[16] = {0};/*the max length is 16*/\
        int Thd_len = strlen(IS_NULL(NAME));            \
        /*if the thread name is not empty, assign the name to the Thd_name*/    \
        if(Thd_len) \
        {           \
            snprintf(Thd_name, sizeof(Thd_name), "%s", IS_NULL(NAME));  \
        }           \
        pthread_t ThdID = 0;    \
        pthread_attr_t Thd_attr; \
        pthread_attr_init(&Thd_attr);    \
        if(STACK_KB >= 1024*8)          \
            fputs("Thread stack is too large!", stderr);    \
        /*set thread stack space size, how many KB*/    \
        if(STACK_KB > 0)    \
            pthread_attr_setstacksize(&Thd_attr, 1024*STACK_KB);    \
        /*set whether the thread is separated from the main thread*/    \
        pthread_attr_setdetachstate(&Thd_attr, JOIN ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED);   \
        pthread_create(&ThdID, &Thd_attr, FUNC, (void*) PARAM); \
        /*if the user passes in the TID, assign the generated ThdID to the TID*/        \
        if(TID) \
        {       \
            *(pthread_t*)(TID) = ThdID; \
        }       \
        if(ThdID == 0)  \
            fputs("pthread_create() error", stderr);    \
        /*if the user define the thread name*/    \
        if(Thd_len) \
            pthread_setname_np(ThdID, Thd_name);    \
        pthread_attr_destroy(&Thd_attr);        \
    }

/*
*   @brief report parameter as error
*   @param message
*   @return void
*/
void error_handling(const char* message);

/*
*   @brief global sequence create
*   @return int
*       sequence created
*/
int getSequence();

//TODO account create function
int getAccount();


#endif