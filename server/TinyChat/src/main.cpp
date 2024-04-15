#include "../include/basic.h"
#include "../include/msgDef.h"
#include "../include/eventHandle.h"
using namespace std;

int main(int argc, char* argv[])
{
    int default_port = 9190;        //the port used by the program by default
    int optch = 0;
    while((optch = getopt(argc, argv, "p:")) != -1)
    {
        switch (optch)
        {
        case 'p':
            default_port = atoi(optarg);
            LOGINFO("port: %s\n", optarg);
            break;
        case '?':
            LOGINFO("Unknown option: %c\n", (char)optopt);
            break;
        default:
            break;
        }
    }

    pthread_mutex_init(&_mutex, NULL);      //the initializatin of _mutex
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_sz;

    //declaration of server socket and client socket
    int serv_sock, clnt_sock;

    //create the server socket
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");
    
    //the initialization of serv_addr
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(default_port);

    //allow socket to be immediately reused
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &serv_sock, sizeof(serv_sock));

    //assign IP and port
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    //listen to client request
    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    //accept client request
    cout << "server start lister:" << "0.0.0.0" << ":" << default_port << endl;
    while(1)
    {
        clnt_addr_sz = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_sz);
        if(clnt_sock == -1)
            error_handling("accept() error");
        LOGINFO("Connect from %s:%u...\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

        int* arg = (int*)malloc(sizeof(int));
        *arg = clnt_sock;
        CREATE_THREAD(NULL, 1024*4, true, event_handle, (void*) arg, NULL);
    }
    close(serv_sock);
    return 0;
}