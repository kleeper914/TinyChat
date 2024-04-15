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
            //return READ_AGAIN;
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
        case command_groupList:
            groupListHandle(this, msgPacket->body + sizeof(messageBody));
            break;
        case command_privateChat:
            privateChatHandle(msgPacket->body + sizeof(messageBody));
            break;
        case command_groupChat:
            groupChatHandle(msgPacket->body + sizeof(messageBody));
            break;
        case command_refreshFriendStatus:
            refreshFriendStatusHandle(this, msgPacket->body + sizeof(messageBody));
            break;
        case command_addFriend:
            if(body->type == 1) //type == 1 -> request
            {
                addFriendReqHandle(this, msgPacket->body + sizeof(messageBody));
            }
            else if(body->type == 2)    //type == 2 -> reply
            {
                addFriendReplyHandle(this, msgPacket->body + sizeof(messageBody));
            }
            break;
        case command_searchAccount:
            searchAccountHandle(msgPacket->body + sizeof(messageBody));
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
    sprintf(query, "select id from user where account = %d", friListReq->m_account);
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
    friendInfo* friInfo_1[num_id1];
    friendInfo* friInfo_2[num_id2];
    for(int i = 0; i < num_id1; i++)
    {
        sprintf(query, "select account, username from user where id = %d", atoi(friendId_1[i].c_str()));
        mydb.myDB_query(query);
        result = mydb.getResult();
        friInfo_1[i] = (friendInfo*)malloc(sizeof(friendInfo));
        friInfo_1[i]->account = atoi(result[0][0].c_str());
        strcpy(friInfo_1[i]->friend_name, result[0][1].c_str());
        friInfo_1[i]->status = false;
        m_friendInfoMap.insert(std::pair(friInfo_1[i]->account, *friInfo_1[i]));
        if(friInfo_1[i] != NULL)
        {
            free(friInfo_1[i]);
            friInfo_1[i] = NULL;
        }
        mydb.cleanResult();
    }
    for(int i = 0; i < num_id2; i++)
    {
        sprintf(query, "select account, username from user where id = %d", atoi(friendId_2[i].c_str()));
        mydb.myDB_query(query);
        result = mydb.getResult();
        friInfo_2[i] = (friendInfo*)malloc(sizeof(friendInfo));
        friInfo_2[i]->account = atoi(result[0][0].c_str());
        strcpy(friInfo_2[i]->friend_name, result[0][1].c_str());
        friInfo_2[i]->status = false;
        m_friendInfoMap.insert(std::pair(friInfo_2[i]->account, *friInfo_2[i]));
        if(friInfo_2[i] != NULL)
        {
            free(friInfo_2[i]);
            friInfo_2[i] = NULL;
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

int Task::groupListHandle(void* arg, void* message)
{
    Task* pthis = (Task*)arg;
    getGroupListReq* groupReq = (getGroupListReq*)message;
    LOGINFO("groupList request from account: %d\n", groupReq->m_account);

    myDB mydb;
    mydb.myDB_init();
    char query[500];
    //get the user id by user account
    sprintf(query, "select id from user where account = %d", groupReq->m_account);
    mydb.myDB_query(query);
    string** result = mydb.getResult();
    int user_id = atoi(result[0][0].c_str());
    mydb.cleanResult();

    //get group accounts and names by the user id
    sprintf(query, "select group_account, group_name, group_id from group_relation gr left join `group` g on gr.group_id = g.id where user_id = %d;", user_id);
    mydb.myDB_query(query);
    result = mydb.getResult();
    int num_group = mydb.getNumRow();
    string* group_name = new string[num_group];
    int* group_account = new int[num_group];
    int* group_id = new int[num_group];
    for(int i = 0; i < num_group; i++)
    {
        group_account[i] = atoi(result[i][0].c_str());
        group_name[i] = result[i][1];
        group_id[i] = atoi(result[i][2].c_str());
        LOGINFO("group %d info: group_account: %d group_name: %s\n", i+1, group_account[i], group_name[i].c_str());
    }
    mydb.cleanResult();

    //get group members by the group_id and send the getGroupListReply
    for(int i = 0; i < num_group; i++)
    {
        LOGINFO("group %d member info: \n", i + 1);
        sprintf(query, "select username, account from group_relation gr left join user u on u.id = gr.user_id where group_id = %d;", group_id[i]);
        mydb.myDB_query(query);
        result = mydb.getResult();
        int member_num = mydb.getNumRow();

        //insert groupInfoMap
        groupStoreInfo* newGroup = new groupStoreInfo;
        newGroup->account = group_account[i];
        newGroup->size = member_num;
        memmove(newGroup->name, group_name[i].c_str(), group_name[i].length());

        getGroupListReply* groupReply;
        char* p = (char*)malloc(sizeof(getGroupListReply) + sizeof(groupMemInfo) * member_num);
        groupReply = (getGroupListReply*)p;
        memmove(groupReply->group_name, group_name[i].c_str(), group_name[i].length());
        groupReply->group_account = group_account[i];
        groupReply->size = member_num;

        for(int j = 0; j < member_num; j++)
        {
            groupMemInfo* memInfo = new groupMemInfo;
            memmove(memInfo->name, result[j][0].c_str(), result[j][0].length());
            memInfo->account = atoi(result[j][1].c_str());
            memInfo->right = 0;
            newGroup->m_groupMemInfoMap.insert(std::pair(memInfo->account, memInfo));
            memmove(p + sizeof(getGroupListReply) + sizeof(groupMemInfo) * j, memInfo, sizeof(groupMemInfo));
            LOGINFO("group member %d info: name: %s, account: %d, right: %d\n", j+1, memInfo->name, memInfo->account, memInfo->right);
        }
        m_groupInfoMap.insert(std::pair(newGroup->account, newGroup));
        sendMsg(m_socket, p, sizeof(getGroupListReply) + sizeof(groupMemInfo)*member_num, command_groupList, 0, 1);

        if(p != NULL)
        {
            free(p);
            p = NULL;
        }
        mydb.cleanResult();
    }

    //print all the message from groupInfoMap and groupMemInfoMap
    LOGINFO("all the message from groupInfoMap and groupMemInfoMap follows:\n");
    groupInfoMap::iterator ite_gMap = m_groupInfoMap.begin();
    while(ite_gMap != m_groupInfoMap.end())
    {
        cout << "groupAccount: " << ite_gMap->first << " groupName: " << ite_gMap->second->name << " groupSize: " << ite_gMap->second->size << endl;
        groupMemInfoMap::iterator ite_gmMap = ite_gMap->second->m_groupMemInfoMap.begin();
        while(ite_gmMap != ite_gMap->second->m_groupMemInfoMap.end())
        {
            cout << "account: " << ite_gmMap->first << " name: " << ite_gmMap->second->name << " right: " << ite_gmMap->second->right << endl;
            ite_gmMap++;
        }
        ite_gMap++;
    }

    if(group_name != NULL)
    {
        delete[] group_name;
        group_name = NULL;
    }
    if(group_account != NULL)
    {
        delete[] group_account;
        group_account = NULL;
    }
    if(group_id != NULL)
    {
        delete[] group_id;
        group_id = NULL;
    }

    return 0;
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

int Task::groupChatHandle(void* message)
{
    groupChatReq* gChatReq = (groupChatReq*)message;
    LOGINFO("account: %d, msgLen: %d, type: %d, groupAccount: %d\n", gChatReq->m_userAccount, gChatReq->m_msgLen, gChatReq->type, gChatReq->m_groupAccount);

    char* buf = (char*)malloc(gChatReq->m_msgLen);
    memmove(buf, (char*)message + sizeof(groupChatReq), gChatReq->m_msgLen);
    LOGINFO("received groupchat message: %s\n", buf);

    //send message to all the online group member
    groupInfoMap::iterator ite_gMap = m_groupInfoMap.find(gChatReq->m_groupAccount);
    groupMemInfoMap::iterator ite_gmMap = ite_gMap->second->m_groupMemInfoMap.begin();

    while(ite_gmMap != ite_gMap->second->m_groupMemInfoMap.end())
    {
        userInfoMap::iterator ite_uMap = m_userInfoMap.find(ite_gmMap->first);
        //do not send to the sender
        if(ite_uMap != m_userInfoMap.end() && ite_uMap->first != m_account)
        {
            sendMsg(ite_uMap->second->m_socket, message, sizeof(groupChatReq)+gChatReq->m_msgLen, command_groupChat);
        }
        ite_gmMap++;
    }

    if(buf != NULL)
    {
        free(buf);
        buf = NULL;
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

int Task::addFriendReqHandle(void* arg, void* message)
{
    addFriendInfoReq* addfriReq = (addFriendInfoReq*) message;
    LOGINFO("receive addFriend request...\n");
    LOGINFO("addFriendInfo: sendeAccount: %d, receicerAccount: %d, message: %s\n", addfriReq->senderAccount, addfriReq->receiverAccount, addfriReq->message);

    //search if the friendAccount is online
    userInfoMap::iterator iter = m_userInfoMap.find(addfriReq->receiverAccount);
    if(iter == m_userInfoMap.end())
    {
        LOGINFO("this account is not exist or not online\n");
        sendMsg(m_socket, NULL, 0, command_addFriend, -1, 1);
        return -1;
    }
    else
    {
        //the receiver is online
        userInfo* recvInfo = iter->second;
    }

    //search map if the two accounts are already friends
    friendInfoMap::iterator ite_friend = m_friendInfoMap.find(iter->first);
    addFriendInfoMap::iterator ite_addFriend = m_addFriendInfoMap.begin();
    //the two accounts are already friends
    if(ite_friend != m_friendInfoMap.end())
    {

        LOGINFO("the two accounts are already friends\n");
        sendMsg(m_socket, NULL, 0, command_addFriend, -1, 1);
        return -1;
    }
    //the two accounts are not friends
    else
    {
        //search the m_addFriendInfoMap if the sender already sent a request
        while(ite_addFriend != m_addFriendInfoMap.end())
        {
            if(ite_addFriend->second->senderAccount == addfriReq->senderAccount && ite_addFriend->second->receiverAccount == addfriReq->receiverAccount)
            {
                //the sender has already send a request
                LOGINFO("the sender has already send a request\n");
                sendMsg(m_socket, NULL, 0, command_addFriend, -1, 2);
                return -1;
            }
            ite_addFriend++;
        }

        addFriendInfo* addInfo = new addFriendInfo;
        addInfo->senderAccount = addfriReq->senderAccount;
        addInfo->receiverAccount = addfriReq->receiverAccount;
        addInfo->is_agree = -1;
        int index = m_addFriendInfoMap.size();
        m_addFriendInfoMap.insert(std::pair(index, addInfo));
        LOGINFO("insert into a addFriendInfo: sender: %d, receiver: %d\n", addInfo->senderAccount, addInfo->receiverAccount);

        //send request to the receiver
        sendMsg(iter->second->m_socket, message, sizeof(addFriendInfoReq), command_addFriend, 0, 1);
        //send success to the client
        sendMsg(m_socket, NULL, 0, command_addFriend, 0, 2);
    }

    return 0;
}

int Task::addFriendReplyHandle(void* arg, void* message)
{
    Task* pthis = (Task*)arg;
    addFriendInfoReply* replyInfo = (addFriendInfoReply*) message;
    LOGINFO("receive addFriendInfoReply: sender: %d, recriver: %d, status: %d\n", replyInfo->senderAccount, replyInfo->receiverAccount, replyInfo->is_agree);

    userInfo* uInfo = NULL;
    userInfoMap::iterator ite_user = m_userInfoMap.find(replyInfo->senderAccount);
    if(ite_user != m_userInfoMap.end())
    {
        uInfo = ite_user->second;
    }
    else
    {
        //the sender is not online
        LOGINFO("the sender is not online\n");
        sendMsg(m_socket, NULL, 0, command_addFriend, -1, 2);
        return -1;
    }

    friendInfoMap::iterator ite_friend = m_friendInfoMap.find(replyInfo->receiverAccount);
    addFriendInfoMap::iterator ite_addFriend = m_addFriendInfoMap.begin();
    if(ite_friend != m_friendInfoMap.end())
    {
        LOGINFO("the two accounts are already friends\n");
        sendMsg(m_socket, NULL, 0, command_addFriend, -1, 2);
    }
    else
    {
        while(ite_addFriend != m_addFriendInfoMap.end())
        {
            if(ite_addFriend->second->senderAccount == replyInfo->senderAccount && ite_addFriend->second->receiverAccount == replyInfo->receiverAccount)
            {
                if(replyInfo->is_agree == 0)
                {
                    m_addFriendInfoMap.erase(ite_addFriend->first);
                    sendMsg(ite_user->second->m_socket, (char*)replyInfo, sizeof(addFriendInfoReply), command_addFriend, 0, 2);
                    return 0;
                }
                else if(replyInfo->is_agree == 1)
                {
                    m_addFriendInfoMap.erase(ite_addFriend->first);

                    myDB mydb;
                    mydb.myDB_init();
                    char query[500];
                    string** result = NULL;
                    //search the two user_id by the accounts
                    int sender_id = 0;
                    sprintf(query, "select id from user where account = %d", replyInfo->senderAccount);
                    mydb.myDB_query(query);
                    result = mydb.getResult();
                    sender_id = atoi(result[0][0].c_str());
                    mydb.cleanResult();

                    int receiver_id = 0;
                    sprintf(query, "select id from user where account = %d", replyInfo->receiverAccount);
                    mydb.myDB_query(query);
                    result = mydb.getResult();
                    receiver_id = atoi(result[0][0].c_str());
                    mydb.cleanResult();

                    //insert friend relationship into mysql table
                    sprintf(query, "insert into friend_relation(user_id1, user_id2) VALUES (%d, %d)", sender_id, receiver_id);
                    mydb.myDB_exe(query);

                    sendMsg(m_socket, NULL, 0, command_addFriend, 0, 2);
                    sendMsg(ite_user->second->m_socket, (char*)message, sizeof(addFriendInfoReply), command_addFriend, 0, 2);
                    return 0;
                }
            }
            ite_addFriend++;
        }

        sendMsg(m_socket, NULL, 0, command_addFriend, -1, 2);
        return -1;
    }

    return 0;
}

int Task::searchAccountHandle(void* message)
{
    searchAccountReq* searchReq = (searchAccountReq*)message;
    LOGINFO("receive search Account request, search account: %d\n", searchReq->account);

    searchAccountReply* searchReply = new searchAccountReply;
    searchReply->m_account = searchReq->account;

    myDB mydb;
    mydb.myDB_init();
    char query[200];
    sprintf(query, "select username from user where account = %d", searchReq->account);
    mydb.myDB_query(query);
    string** result = mydb.getResult();
    LOGINFO("result[0][0]: %s\n", result[0][0].c_str());
    if(result != NULL){
        memmove(searchReply->m_name, result[0][0].c_str(), result[0][0].size());
        LOGINFO("search account: %d, name: %s\n", searchReq->account, searchReply->m_name);
    }
    else{
        LOGINFO("the search account is not exist\n");
        return -1;
    }
    userInfoMap::iterator ite_user = m_userInfoMap.find(searchReq->account);
    friendInfoMap::iterator ite_friend = m_friendInfoMap.find(searchReq->account);
    if(ite_user != m_userInfoMap.end())
    {
        //searched account is online
        LOGINFO("searched account is online\n");
        searchReply->is_online = true;
    }
    else
    {
        //searched account is not online
        LOGINFO("searched account is not online\n");
        searchReply->is_online = false;
    }

    if(ite_friend != m_friendInfoMap.end())
    {
        //searched account is friend
        LOGINFO("searched account is friend\n");
        searchReply->is_friend = true;
    }
    else
    {
        //searched account is not friend
        LOGINFO("searched account is not friend\n");
        searchReply->is_friend = false;
    }

    LOGINFO("search account: %d, name: %s, is_online: %d, is_friend: %d\n", searchReq->account, searchReply->m_name, searchReply->is_online, searchReply->is_friend);
    sendMsg(m_socket, (char*)searchReply, sizeof(searchAccountReply), command_searchAccount);
    if(searchReply != NULL)
    {
        delete searchReply;
        searchReply = NULL;
    }

    return 0;
}