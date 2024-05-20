#ifndef MSGDEF_H
#define MSGDEF_H

#include "basic.h"

//定义前后端交互数据包结构，解决粘包半包问题
struct messageHead
{
    char mark[3];   //数据包唯一标识符, "ROY"
    int len;        //数据包体与负载的长度
};

struct messageBody
{
    int type;       //数据包类型，1-请求    2-响应    3-通知
    int error;      //错误码，0-成功  !0-发生错误
    int sequence;   //消息序列号
    int command;    //数据命令类型
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
        if (head != NULL)
        {
            delete[] head;
            head = NULL;
        }
        if (body != NULL)
        {
            delete[] body;
            body = NULL;
        }
    }
};

//定义接收数据类型枚举
typedef enum {
    RECV_HEAD = 0,
    RECV_BODY = 1
}recvType;

//定义readevent的返回值类型
#define READ_OK      0
#define READ_ERROR  -1
#define READ_AGAIN  -2
#define READ_EXIT   -3

//定义command命令类型
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

//与界面交互的数据结构
struct userInfo{
    char m_userName[30];
    char m_password[30];
    int  m_account;
};

//好友信息
struct friendInfo
{
    char name[30];
    int m_account;
    bool status;        //好友在线信息, true-在线
};

//登陆请求信息
struct loginInfoReq
{
    int m_account;
    char m_password[30];
};

//登陆请求的回复
struct loginInfoReply
{
    int m_account;
    char m_name[30];
};

//好友上线的通知
struct friendOnlineNotify
{
    int m_account;
    bool status;
};

//注册请求
struct registerInfoReq
{
    char m_name[30];
    char m_password[30];
};

//注册请求的回复
struct registerInfoReply
{
    int m_account;
};

//获得好友表请求
struct friendListReq
{
    int m_account;
};

//刷新好友列表请求
struct refreshFriendStatusReq
{
    int m_account;
};

//刷新好友列表回复
struct refreshFriendStatusReply
{
    int m_account;
};

//好友请求的回复
struct friendListReply
{
    int m_account;
    int length;
};

//更新好友信息请求
struct refreshFriendReq
{
    int account;
};

//更新还有信息请求的回复
struct refreshFriendReply
{
    int account;
    char name[30];
};

//获得群聊信息请求
struct getGroupListReq
{
    int m_account;
};

//获得群聊信息回复
struct getGroupListReply
{
    int group_account;
    char group_name[30];
    //char master_name[30];     //群主名
    int size;
};

//群聊成员信息
struct groupMemInfo
{
    int account;
    char name[30];
    int right;      //权限
};

//群聊信息
struct groupInfo
{
    int account;
    char name[30];
    int size;
    std::vector<groupMemInfo*>   groupMemInfoList;   //存放所有群聊用户信息
};

//私聊请求
struct privateChatReq
{
    int m_userAccount;  //发送者账号
    int m_msgLen;       //小心长度
    int type;           //消息类型, 0-文本, (目前只允许发送文本信息)
    int m_friendAccount;    //好友账号
};

//群聊请求
struct groupChatReq
{
    int m_userAccount;  //发生者账号
    int m_msgLen;       //消息长度
    int type;
    int m_groupAccount; //发送的群聊账号
};

//添加好友请求
struct addFriendInfoReq
{
    int senderAccount;
    int receiverAccount;
    char message[60];
};

//添加好友回复
struct addFriendInfoReply
{
    int senderAccount;
    int receiverAccount;
    int is_agree;       //1-同意  0-不同意
};

//查找账户信息请求
struct searchAccountReq
{
    int account;
};

//查找账户信息回复
struct searchAccountReply
{
    int m_account;
    char name[30];
    bool is_friend;
    bool is_online;
};

//创建群聊请求
struct createGroupReq
{
    int master_account;
    char group_name[30];
    int size;
};

//创建群聊请求的回复
struct createGroupReply
{
    int group_account;
    char group_name[30];
    int size;
    int master_account;
};

//添加群组请求
struct addGroupReq
{
    int group_account;
    char group_name[30];
    int sender_account;
};

//定义用户好友表
typedef std::map<int, friendInfo*> friendInfoMap;
//定义群聊信息表
typedef std::map<int, groupInfo*> groupInfoMap;

#endif // MSGDEF_H
