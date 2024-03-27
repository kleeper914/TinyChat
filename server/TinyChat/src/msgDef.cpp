#include "../include/msgDef.h"
#include "../include/mysqlHandle.h"

/*
*   @brief  constructor
*   @param  socket
*/
Task::Task(int socket)
{
    m_socket = socket;
    m_account = 0;
    //memset(m_username, 0, sizeof(m_username));
    m_bufLen = 0;
    m_readNow = 0;
    m_head = NULL;
    m_body = NULL;
    m_type = RECV_HEAD;     //read the head first, by default
    is_finish = false;
    is_login = false;
    //mysql_init(&mysqlCon);
}

/*
*   @brief  destructor
*/
Task::~Task()
{
    if(m_head != NULL)
        delete[] m_head;
    if(m_body != NULL)
        delete[] m_body;
    if(m_socket != -1)
    {
        close(m_socket);
        m_socket = -1;
    }
}

/*
*   @brief  readEvent()
*       call different read functions
*   @return int
*       READ_OK READ_ERROR  READ_AGAIN  READ_EXIT
*/
int Task::readEvent()
{
    int returnVal = 0;
    switch (m_type)
    {
    case RECV_HEAD:
        returnVal = readHead();
        break;
    case RECV_BODY:
        returnVal = readBody();
        break;
    default:
        break;
    }
    LOGINFO("return value : %d\n", returnVal);
    //if the return value is READ_AGAIN, recursively call the readEvent()
    if(returnVal == READ_AGAIN)
        return readEvent();
    return returnVal;
}

/*
*   @brief  readHead()
*       read the head in the data packet
*   @return int
*       READ_OK READ_ERROR  READ_AGAIN  READ_EXIT
*/
int Task::readHead()
{
    //event of reading head initialization
    if(m_head == NULL)
    {
        m_head = new char[sizeof(messageHead)];
        assert(m_head != NULL);
        m_bufLen = sizeof(messageHead);
        m_readNow = 0;
    }

    //maybe can't read the head at once
    int readLen = read(m_socket, m_head + m_readNow, m_bufLen - m_readNow);
    if(readLen < 0)
    {
        return READ_ERROR;
    }
    if(readLen == 0)
    {
        return READ_EXIT;
    }
    m_readNow += readLen;
    //the following code is only executed after reading the complete head
    if(m_readNow == m_bufLen)
    {
        messageHead* tmpHead = (messageHead*)m_head;
        //if the identifier of the head is not "ROY"
        if(strncmp(tmpHead->mark, "ROY", 3) != 0)
        {
            //log the error info, then read again
            LOGINFO("mark: %s, length: %d\n", tmpHead->mark, tmpHead->len);
            m_bufLen = 0;
            m_readNow = 0;
            m_type = RECV_HEAD;
            return READ_AGAIN;
        }
        else
        {
            //the identifier of the head is "ROY", then read the body
            m_type = RECV_BODY;
            int bufLen = ((messageHead*)m_head)->len;   //set the buffer length
            m_body = new char[bufLen];
            assert(m_body != NULL);
            m_bufLen = bufLen;
            m_readNow = 0;
            return READ_AGAIN;
        }
    }
    return 0;
}

/*
*   @brief  readBody()
*       read the body in the data packet
*   @return int
*       READ_OK READ_ERROR  READ_AGAIN  READ_EXIT
*/
int Task::readBody()
{
    //whether reading the specific length in the messageHead
    //It means that readching the specific length in the messageHead
    //after multiple reads of buffer data 
    if(m_readNow == m_bufLen)
    {
        m_type = RECV_HEAD;
        messageBasicHandle();
        is_finish = true;
        return READ_AGAIN;
    }
    else
    {
        int readLen = read(m_socket, m_body + m_readNow, m_bufLen - m_readNow);
        if(readLen < 0)
        {
            return READ_ERROR;
        }
        m_readNow += readLen;

        //whether reading the specific length in the messageHead
        if(m_readNow == m_bufLen)
        {
            m_type = RECV_HEAD;
            messageBasicHandle();
            m_bufLen = 0;
            is_finish = true;
            return READ_AGAIN;
        }
    }
    return READ_OK;
}

/*
*   @brief  messageBasicHandle()
*       prepare for the formal data processing -> messageHandle()
*   @return int
*       READ_OK
*/
int Task::messageBasicHandle()
{
    messagePacket* msgPacket = new messagePacket();
    msgPacket->head = m_head;
    msgPacket->body = m_body;
    msgPacket->bodyLen = m_bufLen;
    messageHandle(msgPacket);

    m_head = NULL;
    m_body = NULL;
    return READ_OK;
}

int Task::messageHandle(messagePacket* msgPacket)
{
    messageHead* head = (messageHead*)msgPacket->head;
    messageBody* body = (messageBody*)msgPacket->body;
    LOGINFO("mark : %s, length : %d\n", head->mark, head->len);
    LOGINFO("received command: %d\n", body->command);

    void* replyData = NULL;
    int replyLength = -1;
    int returnVal = READ_OK;

    switch(body->command)
    {
        case command_login:
            loginHandle(this, msgPacket->body + sizeof(messageBody));
            break;
        case command_logout:
            logoutHandle(this, msgPacket->body + sizeof(messageBody));
            break;
        case command_register:
            registerHandle(this, msgPacket->body + sizeof(messageBody));
            break;
        case command_friendList:
            friendListHandle(this, msgPacket->body + sizeof(messageBody));
            break;
        case command_privateChat:
            privateChatHandle(msgPacket->body + sizeof(messageBody));
            break;
        case command_refreshFriendStatus:
            refreshFriendStatusHandle(this, msgPacket->body + sizeof(messageBody));
            break;
    }
    delete msgPacket;
    return 0;
}

/*
*   @brief  sendMsg()
*       send message from server to client
*   @param  socket : client socket
*   @param  buf:    the pointer of message to be sent
*   @param  bufLen: the length of message to be sent
*   @param  command:    which command is this message
*   @param  error:  error code, 0 is success, not 0 is error, default value:0
*   @param  type:   type of this message, default value 2 mean notify           
*/
void Task::sendMsg(int socket, void* buf, int bufLen, int command, int error, int type)
{
    messageHead head;
    memcpy(head.mark, "ROY", sizeof(head.mark));
    head.len = sizeof(messageBody) + bufLen;
    LOGINFO("send messageHead.len = %d\n", head.len);

    char* p = (char*)malloc(head.len);
    messageBody* pBody = (messageBody*) p;
    pBody->type = type;
    pBody->command = command;
    pBody->error = error;
    pBody->sequence = getSequence();

    if(buf)
    {
        memcpy(p+sizeof(messageBody), buf, bufLen);
    }
    char* sendMsg = (char*)malloc(sizeof(messageHead) + head.len);
    memset(sendMsg, 0, sizeof(messageHead) + head.len);
    memcpy(sendMsg, &head, sizeof(messageHead));
    memcpy(sendMsg + sizeof(messageHead), pBody, head.len);
    free(pBody);
    write(socket, sendMsg, sizeof(messageHead) + head.len);
}

void Task::closeTask()
{
    LOGINFO("start closeTask()\n");
    userInfoMap::iterator ite = m_userInfoMap.find(m_account);
    if(ite == m_userInfoMap.end())
    {
        LOGINFO("search userInfoMap wrong");
    }
    else
    {
        userInfo* userToDel = ite->second;
        m_userInfoMap.erase(ite);
        if(userToDel != NULL)
        {
            free(userToDel);
            userToDel = NULL;
        }
        LOGINFO("the task is closed, number of online user: %lu\n", m_userInfoMap.size());
    }
}

/*
*   @brief  loginHandle()
*       login information handling
*   @param  arg:    pointer this
*   @param  message:    message from client
*/
int Task::loginHandle(void* arg, void* message)
{
    Task* pthis = (Task*)arg;
    loginInfoReq* loginInfo = (loginInfoReq*) message;
    LOGINFO("loginInfo:\naccount:%d, password:%s\n", loginInfo->m_account, loginInfo->m_password);

    //mysql handle
    myDB mydb;
    mydb.myDB_init();
    char query[100];
    sprintf(query, "select * from user where account = \'%d\';", loginInfo->m_account);
    mydb.myDB_query(query);
    string** result = mydb.getResult();
    if(result == NULL)
    {
        //the userInfo is not in the db
        sendMsg(pthis->m_socket, NULL, 0, command_login, -1, 1);
        LOGINFO("this account is not exist in mysql\n");
        mydb.cleanResult();
    }
    else
    {
        mydb.cleanResult();
        userInfoMap::iterator ite = m_userInfoMap.find(loginInfo->m_account);
        if(ite != m_userInfoMap.end())
        {
            sendMsg(pthis->m_socket, NULL, 0, command_login, -1);
            LOGINFO("this account has already logined\n");
            return -1;
        }
        else
        {
            //comfirm account information
            sprintf(query, "select username from user where account = \'%d\' and password = \'%s\'", loginInfo->m_account, loginInfo->m_password);
            mydb.myDB_query(query);
            result = mydb.getResult();
            if(mydb.getNumRow() == 0)
            {
                sendMsg(pthis->m_socket, NULL, 0, command_login, -1);
                LOGINFO("password wrong\n");
                //mydb.cleanResult();
                return -1;
            }
            else
            {
                is_login = true;
                //task info insert
                pthis->m_account = loginInfo->m_account;
                //userInfoMap insert
                userInfo* user = (userInfo*)malloc(sizeof(userInfo));
                user->m_socket = pthis->m_socket;
                user->m_userAccount = loginInfo->m_account;
                user->is_login = true;
                m_userInfoMap.insert(std::make_pair(user->m_userAccount, user));

                //send respond to client
                loginInfoReply* loginReply;
                char* p = (char*)malloc(sizeof(loginInfoReply));
                loginReply = (loginInfoReply*)p;
                loginReply->m_account = pthis->m_account;
                memmove(loginReply->m_name, result[0][0].c_str(), result[0][0].size());
                LOGINFO("the name of user: %s\n", loginReply->m_name);
                sendMsg(pthis->m_socket, p, sizeof(loginInfoReply), command_login, 0, READ_OK);
                LOGINFO("login success, number of online users: %lu\n", m_userInfoMap.size());
                if(p != NULL)
                {
                    free(p);
                    p = NULL;
                }
                mydb.cleanResult();
            }
        }
    }
    return 0;
}

int Task::logoutHandle(void* arg, void* message)
{
    LOGINFO("enter logout function\n");
    Task* pthis = (Task*)arg;
    if(is_login == true)
    {
        pthis->closeTask();
        sendMsg(m_socket, NULL, 0, command_logout);
        pthis->is_login = false;
    }
    return 0;
}

int Task::registerHandle(void* arg, void* message)
{
    Task* pthis = (Task*)arg;
    registerInfoReq* regReq = (registerInfoReq*)malloc(sizeof(registerInfoReq));
    memmove(regReq, message, sizeof(registerInfoReq));
    LOGINFO("register request ... username: %s, password: %s\n", regReq->m_name, regReq->m_password);

    myDB mydb;
    mydb.myDB_init();
    char query[500];
    string** result;
    int account = getAccount();
    LOGINFO("create account: %d\n", account);
    bool is_success = false;

    //create unique account
    while(is_success == false)
    {
        sprintf(query, "select * from user where account = %d", account);
        mydb.myDB_query(query);
        result = mydb.getResult();
        if(result == NULL)
        {
            is_success = true;
        }
        else
        {
            is_success = false;
            account = getAccount();
            LOGINFO("create account: %d\n", account);
        }
        mydb.cleanResult();
    }

    //insert data to the mysql
    sprintf(query, "insert into user(username, account, password) VALUES (\'%s\', %d, \'%s\')", regReq->m_name, account, regReq->m_password);
    if(mydb.myDB_exe(query))
    {
        registerInfoReply* regReply;
        char* p = (char*)malloc(sizeof(registerInfoReply));
        regReply = (registerInfoReply*) p;
        regReply->m_account = account;
        LOGINFO("register success, account: %d\n", account);
        sendMsg(m_socket, p, sizeof(registerInfoReply), command_register, 0, 1);
        if(p != NULL)
        {
            free(p);
            p = NULL;
        }
    }
    return 0;
}

int Task::friendListHandle(void* arg, void* message)
{
    Task* pthis = (Task*)arg;
    friendListReq* friListReq = (friendListReq*)message;
    LOGINFO("friendList request from account: %d\n", friListReq->m_account);

    myDB mydb;
    mydb.myDB_init();
    char query[500];
    //first get the id by account from friendListReq
    sprintf(query, "select id from user where account = \'%d\'", friListReq->m_account);
    mydb.myDB_query(query);
    string** result = mydb.getResult();
    int user_id = atoi(result[0][0].c_str());
    mydb.cleanResult();

    //get the friends' id from friend_relation
    sprintf(query, "select user_id2 from friend_relation where user_id1 = %d", user_id);
    mydb.myDB_query(query);
    result = mydb.getResult();
    int num_id1 = mydb.getNumRow();
    string* friendId_1 = new string[mydb.getNumRow()];
    for(int i = 0; i < mydb.getNumRow(); i++)
    {
        friendId_1[i] = result[i][0];
    }
    mydb.cleanResult();

    sprintf(query, "select user_id1 from friend_relation where user_id2 = %d", user_id);
    mydb.myDB_query(query);
    result = mydb.getResult();
    int num_id2 = mydb.getNumRow();
    string* friendId_2 = new string[mydb.getNumRow()];
    for(int i = 0; i < mydb.getNumRow(); i++)
    {
        friendId_2[i] = result[i][0];
    }
    mydb.cleanResult();

    //search the friend name and account by the friend id
    for(int i = 0; i < num_id1; i++)
    {
        sprintf(query, "select account, username from user where id = %d", atoi(friendId_1[i].c_str()));
        mydb.myDB_query(query);
        result = mydb.getResult();
        friendInfo* friInfo = (friendInfo*)malloc(sizeof(friendInfo));
        friInfo->account = atoi(result[0][0].c_str());
        strcpy(friInfo->friend_name, result[0][1].c_str());
        friInfo->status = false;
        m_friendInfoMap.insert(std::pair(friInfo->account, *friInfo));
        if(friInfo != NULL)
        {
            free(friInfo);
            friInfo = NULL;
        }
        mydb.cleanResult();
    }
    for(int i = 0; i < num_id2; i++)
    {
        sprintf(query, "select account, username from user where id = %d", atoi(friendId_2[i].c_str()));
        mydb.myDB_query(query);
        result = mydb.getResult();
        friendInfo* friInfo = (friendInfo*)malloc(sizeof(friendInfo));
        friInfo->account = atoi(result[0][0].c_str());
        strcpy(friInfo->friend_name, result[0][1].c_str());
        friInfo->status = false;
        m_friendInfoMap.insert(std::pair(friInfo->account, *friInfo));
        if(friInfo != NULL)
        {
            free(friInfo);
            friInfo = NULL;
        }
        mydb.cleanResult();
    }

    /*
    for(const auto& entry : m_friendInfoMap)
    {
        Json::Value friInfoJson;
        friInfoJson["username"] = entry.second.friend_name;
        friInfoJson["account"] = entry.second.account;
        jsonFriendInfo.append(friInfoJson);
    }
    Json::StreamWriterBuilder builder;
    std::string jsonData = Json::writeString(builder, jsonFriendInfo);
    cout << "data of jsonFriendInfo : \n" << jsonData << endl;
    

    //send friend info to the client
    char* sendData = (char*)malloc(sizeof(friendListReply) + jsonData.length());
    friendListReply* friReply = (friendListReply*) sendData;
    friReply->m_account = m_account;
    friReply->length = jsonData.length();
    memmove(sendData + sizeof(friendListReply), jsonData.c_str(), jsonData.length());
    sendMsg(m_socket, sendData, sizeof(friendListReply) + jsonData.length(), command_friendList, 0, 1);
    LOGINFO("size of jsonData : %ld\n", jsonData.length());
    //cout << "friendList string data: \n" << sendData << endl;
    if(sendData != NULL)
    {
        free(sendData);
        sendData = NULL;
    }
    */

    //judge the friends' status
    friendInfoMap::iterator ite_friend = m_friendInfoMap.begin();
    while(ite_friend != m_friendInfoMap.end())
    {
        userInfoMap::iterator ite_user = m_userInfoMap.find(ite_friend->first);
        if(ite_user != m_userInfoMap.end())
        {
            ite_friend->second.status = true;
            LOGINFO("account: %d status: %d\n", ite_friend->second.account, ite_friend->second.status);
        }
        else
        {
            LOGINFO("account: %d status: %d\n", ite_friend->second.account, ite_friend->second.status);
        }
        ite_friend++;
    }

    //TODO
    //send user online message to all friends and update online status
    /*friendInfoMap::iterator ite = m_friendInfoMap.begin();
    while(ite != m_friendInfoMap.end())
    {
        if(ite->second.status == true)
        {
            userInfoMap::iterator ite_user = m_userInfoMap.find(ite->second.account);
            friendOnlineNotify* onlineInfo = (friendOnlineNotify*)malloc(sizeof(friendOnlineNotify));
            onlineInfo->m_account = ite->second.account;
            onlineInfo->status = true;
            sendMsg(ite_user->second->m_socket, (char*)onlineInfo, sizeof(friendOnlineNotify), command_login, 0);
            if(onlineInfo != NULL)
            {
                free(onlineInfo);
                onlineInfo = NULL;
            }
        }
        ite++;
    }*/

    LOGINFO("the number of m_account's friends : %lu\n", m_friendInfoMap.size());
    int length = sizeof(friendListReply) + sizeof(friendInfo) * m_friendInfoMap.size();
    char* p = (char*)malloc(length);
    friendListReply* friReply = (friendListReply*) p;
    friReply->m_account = m_account;
    friReply->length = m_friendInfoMap.size();

    friendInfoMap::iterator ite = m_friendInfoMap.begin();
    for(int i = 0; ite != m_friendInfoMap.end(); i++, ite++)
    {
        friendInfo* pfriend = &(ite->second);
        friendInfo* pSend = (friendInfo*)(p + sizeof(friendListReply) + sizeof(friendInfo) * i);
        strncpy(pSend->friend_name, pfriend->friend_name, sizeof(pfriend->friend_name));
        pSend->account = pfriend->account;
        //TODO send friend online status
        pSend->status = pfriend->status;

        //send user's online status to friends
        if(ite->second.status == true)
        {
            userInfoMap::iterator ite_user = m_userInfoMap.find(ite->first);
            friendOnlineNotify* onlineNotify = (friendOnlineNotify*)malloc(sizeof(friendOnlineNotify));
            onlineNotify->m_account = m_account;
            onlineNotify->status = true;
            sendMsg(ite_user->second->m_socket, (char*)onlineNotify, sizeof(friendOnlineNotify), command_login, 0);
            if(onlineNotify != NULL)
            {
                free(onlineNotify);
                onlineNotify = NULL;
            }
        }
    }
    sendMsg(m_socket, p, length, command_friendList, 0, 1);

    return true;
}

int Task::privateChatHandle(void* message)
{
    privateChatReq* pchatReq = (privateChatReq*) message;
    LOGINFO("account : %d, msgLen : %d, type : %d, friednAccount : %d\n", pchatReq->m_userAccount, pchatReq->m_msgLen, pchatReq->m_type, pchatReq->m_friendAccount);
    char* buffer = new char[pchatReq->m_msgLen];
    memset(buffer, 0, pchatReq->m_msgLen);
    //the message user sent is after the privateChatReq data
    memmove(buffer, (char*) message + sizeof(privateChatReq), pchatReq->m_msgLen);
    LOGINFO("received message : %s\n", buffer);

    userInfoMap::iterator ite = m_userInfoMap.find(pchatReq->m_friendAccount);
    if(ite == m_userInfoMap.end())
    {
        LOGINFO("the friend account is not online\n");
    }
    else
    {
        int friend_socket = ite->second->m_socket;
        sendMsg(friend_socket, message, sizeof(privateChatReq) + pchatReq->m_msgLen, command_privateChat);
    }
    if(buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }
    return 0;
}

int Task::refreshFriendStatusHandle(void* arg, void* message)
{
    Task* pthis = (Task*)arg;
    refreshFriendStatusReq* refreshStatusReq = (refreshFriendStatusReq*)message;
    LOGINFO("receive refreshFriendStatusRequest from account: %d\n", refreshStatusReq->m_account);

    //send refresh friend status request to online friends
    friendInfoMap::iterator ite = m_friendInfoMap.begin();
    while(ite != m_friendInfoMap.end())
    {
        userInfoMap::iterator ite_user = m_userInfoMap.find(ite->first);
        if(ite_user != m_userInfoMap.end())
        {
            LOGINFO("exist friend online... account = %d\n", ite->second.account);
            refreshFriendStatusReply* refreshStatusReply = (refreshFriendStatusReply*)malloc(sizeof(refreshFriendStatusReply));
            refreshStatusReply->m_account = m_account;
            sendMsg(ite_user->second->m_socket, refreshStatusReply, sizeof(refreshFriendStatusReply), command_refreshFriendStatus, 0, 1);
            if(refreshStatusReply != NULL)
            {
                free(refreshStatusReply);
                refreshStatusReply = NULL;
            }
        }
        ite++;
    }

    return 0;
}