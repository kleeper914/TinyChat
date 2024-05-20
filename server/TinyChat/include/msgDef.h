#ifndef _MSGDEF_H_
#define _MSGDEF_H_

#include "basic.h"
#include <mysql/mysql.h>

static int max_ID_index = 100;      //max counter of the accounts
static pthread_mutex_t _mutex;      //the definition of Mutual Exclusion 

//define the data structure that the server processes at once
struct messageHead
{
    char mark[3];   //protocol identifier, "ROY"
    int len;        //the length of the body
};

struct messageBody
{
    int type;   //1 request   2 reply   3 notify
    int error;  //0 success   not 0 return relevant error codes
    int sequence;
    int command;
};

struct messagePacket
{
    char* head;
    char* body;
    int bodyLen;

    messagePacket()
    {
        head = NULL;
        body = NULL;
        bodyLen = 0;
    }

    ~messagePacket()
    {
        if(head != NULL)
        {
            delete[] head;
            head = NULL;
        }
        if(body != NULL)
        {
            delete[] body;
            body = NULL;
        }
    }
};

/*
*   the request structure definition include:
*   register information
*/

//online user information
struct userInfo
{
    //char m_username[30];
    //char m_password[30];
    int m_userAccount;
    int m_socket;
    bool is_login;
};

//friend information
struct friendInfo
{
    char friend_name[30];
    int account;
    bool status;    //true is online
};

//regist user info request
struct registInfoReq
{
    char m_username[30];
    char m_password[30];
};

//reply to registing user info request
struct registInfoReply
{
    int m_userAccount;   //random between [20000, 30000]
};

//login info request
struct loginInfoReq
{
    int m_account;
    char m_password[30];
};

//login info reply
struct loginInfoReply
{
    int m_account;
    char m_name[30];
};

//friend Online notify
struct friendOnlineNotify
{
    int m_account;
    bool status;
};

//register info request
struct registerInfoReq
{
    char m_name[30];
    char m_password[30];
};

//register info reply
struct registerInfoReply
{
    int m_account;
};

//friend_list request
struct friendListReq
{
    int m_account;
};

//friend_list reply
struct friendListReply
{
    int m_account;
    int length;     //number of user's friends
};

//refresh friend status request
struct refreshFriendStatusReq
{
    int m_account;
};

//refresh friend status reply
struct refreshFriendStatusReply
{
    int m_account;
};

//private chat request
struct privateChatReq
{
    int m_userAccount;
    int m_msgLen;
    int m_type;     //the type of message, 0 is text...
    int m_friendAccount;
};

//group chat request
struct groupChatReq
{
    int m_userAccount;  //account of sender
    int m_msgLen;       //the length of message
    int type;
    int m_groupAccount; //group account to be sent
};

//get group List request
struct getGroupListReq
{
    int m_account;
};

//get group list reply
struct getGroupListReply
{
    int group_account;
    char group_name[30];
    //char master_name[30];
    int size;
};

//group information send to client
struct groupInfo
{
    int account;
    char name[30];
    int size;
};

//info of group member
struct groupMemInfo
{
    int account;
    char name[30];
    int right;      //authority
};

typedef std::map<int, groupMemInfo*>groupMemInfoMap;

//store the detailed group Infomation in the server
struct groupStoreInfo
{
    int account;
    char name[30];
    int size;
    groupMemInfoMap m_groupMemInfoMap;
};

//user to be friend info
struct addFriendInfo
{
    int senderAccount;
    int receiverAccount;
    int is_agree;   //1-agree   -1-unknow   0-reject
};

//add friend request
struct addFriendInfoReq
{
    int senderAccount;
    int receiverAccount;
    char message[60];
};

struct addFriendInfoReply
{
    int senderAccount;
    int receiverAccount;
    int is_agree;       //1-agree   0-not agree
};

//refresh friend list request
struct refreshFriendListReq
{
    int account;
};

//refresh friend list reply
struct refreshFriendListReply
{
    int account;
    char name[30];
};

//search account request
struct searchAccountReq
{
    int account;
};

//search account reply
struct searchAccountReply
{
    int m_account;
    char m_name[30];
    bool is_friend;
    bool is_online;
};

//create group request
struct createGroupReq
{
    int master_account;
    char group_name[30];
    int size;
};

//create group reply
struct createGroupReply
{
    int group_account;
    char group_name[30];
    int size;
    int master_account;
};

//add Group request
struct addGroupReq
{
    int group_account;
    char group_name[30];
    int sender_account;
};

/*
*   define the return value of the Task::readEvent()
*/
#define READ_OK      0
#define READ_ERROR  -1
#define READ_AGAIN  -2
#define READ_EXIT   -3

/*
*   define receive message type enum
*/
typedef enum{
    RECV_HEAD = 0,
    RECV_BODY = 1
}recvType;

/*
*   define receice command type enum
*/

enum
{
    command_login = 0,
    command_logout,
    command_register,
    command_friendList,
    command_groupList,
    command_privateChat,
    command_groupChat,
    command_refreshFriendStatus,
    command_addFriend,
    command_searchAccount,
    command_refreshFriendList,
    command_refreshGroupList,
    command_createGroup,
    command_addGroup
};

/*
*   online user map define
*/
typedef std::map<int, userInfo*> userInfoMap;
typedef std::map<int, friendInfo>friendInfoMap;
typedef std::map<int, groupStoreInfo*>groupInfoMap;
typedef std::map<int, addFriendInfo*>addFriendInfoMap;

static userInfoMap m_userInfoMap;
static addFriendInfoMap m_addFriendInfoMap;

/*
*   define the class of task processing
*/
class Task
{
public:
    Task(int socket);   //constructor
    ~Task();            //destructor
public:
    int readEvent();
    int readHead();
    int readBody();
    int messageBasicHandle();
    int messageHandle(messagePacket*);
    int loginHandle(void* arg, void* message);
    int logoutHandle(void* arg, void* message);
    int registerHandle(void* arg, void* message);
    int initFriendListHandle(void* arg, void* message);
    int refreshFriendListHandle(void* arg, void* message);
    int initGroupListHandle(void* arg, void* message);
    int refreshGroupListHandle(void* arg, void* message);
    int privateChatHandle(void* message);
    int groupChatHandle(void* message);
    int refreshFriendStatusHandle(void* arg, void* message);
    int addFriendReqHandle(void* arg, void* message);
    int addFriendReplyHandle(void* arg, void* message);
    int searchAccountHandle(void* message);
    int createGroupHandle(void* message);
public:
    static void sendMsg(int socket, void* buf, int bufLen, int command, int error = 0, int type = 2);
private:
    void closeTask();
private:
    int m_socket;
    int m_account;
    //char m_username[30];
    int m_bufLen;       //receive message buffer size
    int m_readNow;      //the length of data currently read
    char* m_head;       //head message storage address
    char* m_body;       //pack message storage address
    recvType m_type;    //type of received message
    bool is_finish;     //task comletion identifier
    bool is_login;
    friendInfoMap m_friendInfoMap;
    groupInfoMap m_groupInfoMap;
    //addFriendInfoMap m_addFriendInfoMap;
};

#endif