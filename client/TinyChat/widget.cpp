#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "widget.h"
#include "ui_widget.h"
#include "logindlg.h"
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QNetworkProxy>
#include <QUrl>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_uInfo.m_account = 0;
    init();
    initUI();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::initUI()
{
    LOGINFO() << "开始UI初始化";
    this->setWindowTitle("TinyChat");
    ui->chatNameLabel->setText("UNKONWN");
    ui->status_label->setText("UNKNOWN");
    QFont* labelFont = new QFont;
    labelFont->setPointSize(20);
    ui->chatNameLabel->setFont(*labelFont);
    labelFont->setPointSize(10);
    ui->status_label->setFont(*labelFont);

    initGroupChat();

    connect(this, SIGNAL(friendList_finish()), this, SLOT(init_privateChat()));
}

void Widget::init_privateChat()
{
    privateButtonWidget = new QWidget;
    ui->stackedWidget_button->addWidget(privateButtonWidget);
    QVBoxLayout* privateButtonWidgetLayout = new QVBoxLayout;
    privateButtonWidgetLayout->setAlignment(Qt::AlignTop);
    privateButtonWidget->setLayout(privateButtonWidgetLayout);
    friendInfoMap::iterator ite = m_friendInfoMap.begin();

    friendButtonList = new privateChatButton*[m_friendInfoMap.size()];
    privateChatPageList = new privateChatPage*[m_friendInfoMap.size()];
    privateChatLayoutList = new QVBoxLayout*[m_friendInfoMap.size()];
    privateChatTextBrowserList = new chatTextBrowser*[m_friendInfoMap.size()];
    privateChatTextEditList = new chatTextEdit*[m_friendInfoMap.size()];

    for(int i = 0; ite != m_friendInfoMap.end(); i++, ite++)
    {
        //创建按钮
        friendButtonList[i] = new privateChatButton(ite->second);
        friendButtonList[i]->setText(ite->second->name);
        privateButtonWidgetLayout->addWidget(friendButtonList[i]);

        //创建每个好友对应的对话框
        privateChatPageList[i] = new privateChatPage(ite->second);
        ui->stackedWidget_chat->addWidget(privateChatPageList[i]);
        privateChatLayoutList[i] = new QVBoxLayout;
        privateChatLayoutList[i]->setAlignment(Qt::AlignTop);
        privateChatPageList[i]->setLayout(privateChatLayoutList[i]);

        //加入textedit和textbrowser组件
        privateChatTextBrowserList[i] = new chatTextBrowser(ite->second);
        privateChatLayoutList[i]->addWidget(privateChatTextBrowserList[i]);
        privateChatTextEditList[i] = new chatTextEdit(ite->second);
        privateChatTextEditList[i]->installEventFilter(this);   //设置事件过滤器
        privateChatLayoutList[i]->addWidget(privateChatTextEditList[i]);
        privateChatLayoutList[i]->setStretch(0, 3);
        privateChatLayoutList[i]->setStretch(1, 1);

        connect(friendButtonList[i], &QPushButton::clicked, [this, i](){
            chat_type = 0;      //设置要发送的信息是私聊
            friendAccount_to_chat = friendButtonList[i]->getFriendAccount();
            LOGINFO() << "当前要发送数据的好友账号是: " << friendAccount_to_chat;
            ui->stackedWidget_chat->setCurrentWidget(privateChatPageList[i]);
            LOGINFO() << "设置聊天框标签为: " << friendButtonList[i]->getFriendName();
            ui->chatNameLabel->setText(friendButtonList[i]->getFriendName());
            if(friendButtonList[i]->getFriendStatus() == true)
            {
                ui->status_label->setText("在线");
            }
            else if(friendButtonList[i]->getFriendStatus() == false)
            {
                ui->status_label->setText("离线");
            }
            edit_to_chat = privateChatTextEditList[i];
            //将消息未读数置零
            friendButtonList[i]->num_not_read_zero();
        });
    }
    LOGINFO() << "私聊界面初始化完成";
}

void Widget::initGroupChat()
{
    LOGINFO() << "初始化群聊界面" ;
    groupButtonWidget = new QWidget;
    ui->stackedWidget_button->addWidget(groupButtonWidget);
    groupButtonWidgetLayout = new QVBoxLayout;
    groupButtonWidgetLayout->setAlignment(Qt::AlignTop);
    groupButtonWidget->setLayout(groupButtonWidgetLayout);

    connect(this, SIGNAL(groupList_finish()), this, SLOT(insert_groupPage()));
    connect(this, SIGNAL(groupInsert_finish()), this, SLOT(connnect_groupEvent()));
}

void Widget::insert_groupPage()
{
    LOGINFO() << "开始插入群聊信息，account: " << groupAccount_to_insert;
    groupInfoMap::iterator ite_groupInfoMap = m_groupInfoMap.find(groupAccount_to_insert);

    //创建按钮
    groupChatButton* newGroupButton = new groupChatButton(ite_groupInfoMap->second);
    newGroupButton->setText(ite_groupInfoMap->second->name);
    groupButtonList.push_back(newGroupButton);
    groupButtonWidgetLayout->addWidget(newGroupButton);

    //创建每个群聊对应的对话框
    groupChatPage* newGroupPage = new groupChatPage;
    groupChatPageList.push_back(newGroupPage);
    ui->stackedWidget_chat->addWidget(newGroupPage);
    QVBoxLayout* newLayout = new QVBoxLayout;
    newLayout->setAlignment(Qt::AlignTop);
    newGroupPage->setLayout(newLayout);
    groupChatLayoutList.push_back(newLayout);

    //加入textEdit和textBrowser组件
    chatTextBrowser* groupTextBrowser = new chatTextBrowser(ite_groupInfoMap->second);
    newLayout->addWidget(groupTextBrowser);
    groupChatTextBrowserList.push_back(groupTextBrowser);
    chatTextEdit* groupTextEdit = new chatTextEdit(ite_groupInfoMap->second);
    groupTextEdit->installEventFilter(this);    //设置事件过滤器
    newLayout->addWidget(groupTextEdit);
    groupChatTextEditList.push_back(groupTextEdit);
    newLayout->setStretch(0, 3);
    newLayout->setStretch(1, 1);

    emit groupInsert_finish();
}

void Widget::connnect_groupEvent()
{
    int i = groupButtonList.size() - 1;
    connect(groupButtonList[i], &QPushButton::clicked, [this, i](){
        chat_type = 1;      //设置要发生的信息是群聊
        groupAccount_to_chat = groupButtonList[i]->getGroupAccount();
        LOGINFO() << "当前要发送的群聊账号是: " << groupAccount_to_chat;
        ui->stackedWidget_chat->setCurrentWidget(groupChatPageList[i]);
        LOGINFO() << "设置聊天框标签为: " << groupButtonList[i]->getGroupName();
        ui->chatNameLabel->setText(groupButtonList[i]->getGroupName());
        ui->status_label->setText("");
        edit_to_chat = groupChatTextEditList[i];
    });
}

void Widget::init()
{
    //使用户能够在使用代理的情况下连接至服务器
    QNetworkProxyFactory::setUseSystemConfiguration(false);

    socketState = false;
    is_login = false;
    m_socket = new QTcpSocket;

    QString ipAddr = SERVER_IP;
    quint16 port = SERVER_PORT;

    if(!socketState)
    {
        m_socket->connectToHost(ipAddr, port);
        if(m_socket->waitForConnected())
        {
            LOGINFO() << "连接服务端成功";
            socketState = true;
        }
        else
        {
            LOGINFO() << m_socket->errorString();
            return;
        }
    }
    else
    {
        m_socket->close();
        socketState = false;
    }

    m_task = new Task(m_socket);
    connect(m_task, SIGNAL(signals_emit(messagePacket*)), this, SLOT(signals_handle(messagePacket*)));

    edit_to_chat = NULL;

    LoginDlg logdlg;
    do
    {
        logdlg.show();
        int status = logdlg.exec();
        //接收到logindlg.cpp中on_pushButton_login_clicked()发送的accept()信号
        if(status == QDialog::Accepted)
        {
            LOGINFO() << "loging...";
            userInfo* uInfo = logdlg.getUInfo();
            memmove(&m_uInfo, uInfo, sizeof(userInfo));
            LOGINFO() << "account:" << m_uInfo.m_account;
            LOGINFO() << "password:" << m_uInfo.m_password;
            login();
        }
        else if(status == QDialog::Rejected)
        {
            this->is_login = false;
            return;
        }
    }while(is_login == false);

    //初始化音频
    player = new QMediaPlayer;
    output = new QAudioOutput;
    player->setAudioOutput(output);

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));

    getFriendList();
    getGroupList();

    connect(this, SIGNAL(emit_friendAccount(int)), this, SLOT(searchPrivateBrowser(int)));
    connect(this, SIGNAL(emit_groupAccount(int)), this, SLOT(searchGroupBrowser(int)));
}

void Widget::readyReadSlot()
{
    if(m_task->readEvent() != READ_EXIT)
    {
        LOGINFO() << "读取出错";
    }
}

void Widget::closeEvent(QCloseEvent* event)
{
    //如果没有用户登陆直接退出
    if(m_uInfo.m_account == 0)
    {
        return;
    }

    while(1)
    {
        //发出登出请求
        sendMsg(NULL, 0, command_logout);

        //接收服务端回复
        messageHead* head = (messageHead*)malloc(sizeof(messageHead));
        memset(head, 0, sizeof(messageHead));
        m_socket->read((char*)&head, sizeof(messageHead));
        char* pBody = (char*)malloc(head->len);
        messageBody* body = (messageBody*)pBody;
        m_socket->read((char*)pBody, head->len);
        if(body->error == 0)
        {
            //退出成功
            LOGINFO() << "退出成功，用户名为：" << m_uInfo.m_userName;
            if(pBody != NULL)
            {
                free(pBody);
                pBody = NULL;
            }
            if(head != NULL)
            {
                free(head);
                head = NULL;
            }
            break;
        }
        if(pBody != NULL)
        {
            free(pBody);
            pBody = NULL;
        }
        if(head != NULL)
        {
            free(head);
            head = NULL;
        }
    }

    //发出刷新好友状态请求
    refreshFriendStatusReq* refreshStatusReq = (refreshFriendStatusReq*)malloc(sizeof(refreshFriendStatusReq));
    refreshStatusReq->m_account = m_uInfo.m_account;
    sendMsg((char*)refreshStatusReq, sizeof(refreshFriendStatusReq), command_refreshFriendStatus);
    if(refreshStatusReq != NULL)
    {
        free(refreshStatusReq);
        refreshStatusReq = NULL;
    }
    qDebug("close");
}

int Widget::getFriendList()
{
    //发送获得好友表请求
    friendListReq friReq;
    memset(&friReq, 0, sizeof(friendListReq));
    friReq.m_account = m_uInfo.m_account;
    sendMsg((char*)&friReq, sizeof(friendListReq), command_friendList);

    return true;
}

int Widget::getGroupList()
{
    //发送获得群聊好友表请求
    getGroupListReq* groupReq = (getGroupListReq*)malloc(sizeof(getGroupListReq));
    groupReq->m_account = m_uInfo.m_account;
    sendMsg((char*)groupReq, sizeof(getGroupListReq), command_groupList);
    if(groupReq != NULL)
    {
        free(groupReq);
        groupReq = NULL;
    }

    return true;
}

void Widget::login()
{
    //发送登陆请求
    loginInfoReq loginInfo;
    memset(&loginInfo, 0, sizeof(loginInfoReq));
    loginInfo.m_account = m_uInfo.m_account;
    strncpy(loginInfo.m_password, m_uInfo.m_password, strlen(m_uInfo.m_password));
    sendMsg((char*)&loginInfo, sizeof(loginInfoReq), command_login);
    if(this->m_socket->waitForReadyRead() == false)
    {
        LOGINFO() << m_socket->errorString();
        return;
    }

    //接收服务端响应
    messageHead head;
    memset(&head, 0, sizeof(messageHead));
    m_socket->read((char*)&head, sizeof(messageHead));
    char* pBody = (char*)malloc(head.len);
    messageBody* body = (messageBody*)pBody;
    m_socket->read((char*)pBody, head.len);
    loginInfoReply* loginReply = (loginInfoReply*)malloc(sizeof(loginInfoReply));
    memmove((char*)loginReply, pBody + sizeof(messageBody), sizeof(loginInfoReply));
    memmove(m_uInfo.m_userName, loginReply->m_name, strlen(loginReply->m_name));
    if(body->error == 0)
    {
        //登陆成功
        is_login = true;
        LOGINFO() << "登陆成功，用户名为：" << m_uInfo.m_userName;
    }

    if(pBody != NULL)
    {
        free(pBody);
        pBody = NULL;
    }
    if(loginReply != NULL)
    {
        free(loginReply);
        loginReply = NULL;
    }
}

void Widget::sendMsg(void* buf, int bufLen, int command, int error, int type)
{
    messageHead head;
    memcpy(head.mark, "ROY", sizeof(head.mark));
    head.len = sizeof(messageBody) + bufLen;

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
    if(m_socket->isWritable()){
        m_socket->write(sendMsg, sizeof(messageHead) + head.len);
    }
    else
    {
        LOGINFO() << "m_socket不可传输数据";
    }
    if(pBody != NULL)
    {
        free(pBody);
        pBody = NULL;
    }
    if(sendMsg != NULL)
    {
        free(sendMsg);
        sendMsg = NULL;
    }
}

int Widget::signals_handle(messagePacket* msgPacket)
{
    messageHead* head = (messageHead*)msgPacket->head;
    messageBody* body = (messageBody*)msgPacket->body;
    LOGINFO() << "mark : " << head->mark << "\tlength : " << head->len;
    LOGINFO() << "received command: " << body->command;
    LOGINFO() << "sizeof(messageHead): " << sizeof(messageHead) << " sizeof(messageBody): " << sizeof(messageBody);

    switch (body->command)
    {
    case command_login:
        loginHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_logout:
        //logoutHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_register:
        //registerHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_friendList:
        friendListHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_groupList:
        groupListHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_privateChat:
        privateChatHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_groupChat:
        groupChatHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_refreshFriendStatus:
        refreshFriendStatusHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_searchAccount:
        searchAccountHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_addFriend:
        if(body->type == 1)     //接收到请求
        {
            addFriendReqHandle(msgPacket->body + sizeof(messageBody));
        }
        else if(body->type == 2)    //接收到回复
        {
            //接收到服务端对请求的回复
            if(head->len - sizeof(messageBody) == 0)
            {
                if(body->error == 0)
                {
                    LOGINFO() << "发送请求成功...";
                }
                else
                {
                    LOGINFO() << "发送请求失败...error: " << body->error;
                }
            }
            else
            {
                addFriendReplyHandle(msgPacket->body + sizeof(messageBody));
            }
        }
        break;
    }
    return 0;
}

int Widget::loginHandle(void* message)
{
    friendOnlineNotify* onlineNotify = (friendOnlineNotify*)message;
    LOGINFO() << "有好友上线了, account: " << onlineNotify->m_account;

    friendInfoMap::iterator ite = m_friendInfoMap.find(onlineNotify->m_account);
    ite->second->status = true;
    return 0;
}

int Widget::friendListHandle(void* message)
{
    friendListReply* friReply = (friendListReply*)message;
    LOGINFO() << "sizeof(friendListReply): " << sizeof(friendListReply);
    LOGINFO() << "friendListReply from server: m_account:" << friReply->m_account << "\tlength of friendList: " << friReply->length;

    for(int i = 0; i < friReply->length; i++)
    {
        //读取一个好友数据
        friendInfo* pfriend = (friendInfo*)((char*)message + sizeof(friendListReply) + sizeof(friendInfo) * i);
        friendInfo* newFriend = (friendInfo*)malloc(sizeof(friendInfo));
        memmove(newFriend, pfriend, (sizeof(friendInfo)));
        //在好友表中插入信息
        m_friendInfoMap.insert(std::pair(newFriend->m_account, newFriend));
    }

    friendInfoMap::iterator ite = m_friendInfoMap.begin();
    qDebug() << "账号: " << m_uInfo.m_account << " 好友信息如下：";
    while(ite != m_friendInfoMap.end())
    {
        qDebug() << "account: " << ite->second->m_account << " 用户名: " << ite->second->name << " 在线情况: " << ite->second->status;
        ite++;
    }

    emit friendList_finish();

    return true;
}

int Widget::groupListHandle(void* message)
{
    getGroupListReply* groupReply = (getGroupListReply*)message;
    LOGINFO() << "groupListReply from server";
    LOGINFO() << "groupAccount: " << groupReply->group_account << " groupName: " << groupReply->group_name << " groupSize: " << groupReply->size;
    groupInfo* gInfo = new groupInfo;
    gInfo->account = groupReply->group_account;
    memmove(gInfo->name, groupReply->group_name, sizeof(groupReply->group_name));
    gInfo->size = groupReply->size;
    m_groupInfoMap.insert(std::pair(groupReply->group_account, gInfo));

    for(int i = 0; i < groupReply->size; i++)
    {
        LOGINFO() << "收到第" << i + 1 << "个群聊用户信息";
        groupMemInfo* groupMem = (groupMemInfo*)((char*)message + sizeof(getGroupListReply) + sizeof(groupMemInfo) * i);
        groupMemInfo* newGroupMem = new groupMemInfo;
        memmove(newGroupMem, groupMem, sizeof(groupMemInfo));
        gInfo->groupMemInfoList.push_back(groupMem);
        LOGINFO() << "用户信息： 账号: " << newGroupMem->account << " 用户名: " << newGroupMem->name << " 权限: " << newGroupMem->right;
    }

    //由于不是一次全部接收所有群聊信息，所有将初始化群聊界面和插入群聊信息放在一个slot函数中
    groupAccount_to_insert = groupReply->group_account;
    emit groupList_finish();

    return true;
}

int Widget::privateChatHandle(void* message)
{
    LOGINFO() << "收到聊天消息";

    privateChatReq* priChatReq = (privateChatReq*)message;
    //如果发送请求中的好友账号为用户账号
    if(priChatReq->m_friendAccount == m_uInfo.m_account)
    {
        int sendAccount = priChatReq->m_userAccount;    //发送者账号即为发送请求中的用户账户
        emit emit_friendAccount(sendAccount);
        char* buf = (char*)malloc(priChatReq->m_msgLen + 1);
        memset(buf, 0, priChatReq->m_msgLen + 1);
        memmove(buf, (char*)message + sizeof(privateChatReq), priChatReq->m_msgLen);
        QString displayMsg = QString(buf);
        friendInfo* friInfo = searchFriendInfo(sendAccount);
        //TODO 将聊天信息存储在本地或数据库

        //将发送的消息显示在对应的聊天框内
        displayMessage(browser_to_send, &displayMsg, friInfo);
        LOGINFO() << "[" << friInfo->name << "]" << buf;
        if(buf != NULL)
        {
            free(buf);
            buf = NULL;
        }
        //更新消息未读数；若未设置免打扰，响起提示音
        privateChatButton* button_to_change = NULL;
        for(int i = 0; i < m_friendInfoMap.size(); i++)
        {
            if(sendAccount == friendButtonList[i]->getFriendAccount())
            {
                LOGINFO() << "找到了对应的私聊按钮";
                button_to_change = friendButtonList[i];
            }
        }
        if(button_to_change != NULL)
        {
            button_to_change->num_not_read_plus();
            messageAlert();
        }
    }
    return true;
}

int Widget::groupChatHandle(void* message)
{
    LOGINFO() << "收到群聊消息";
    groupChatReq* gChatReq = (groupChatReq*)message;
    //如果发送的群聊请求中的群聊账号在群聊信息表中
    groupInfoMap::iterator ite_gMap = m_groupInfoMap.find(gChatReq->m_groupAccount);
    if(ite_gMap != m_groupInfoMap.end())
    {
        emit emit_groupAccount(gChatReq->m_groupAccount);
        char* buf = (char*)malloc(gChatReq->m_msgLen + 1);
        memset(buf, 0, gChatReq->m_msgLen + 1);
        memmove(buf, (char*)message + sizeof(groupChatReq), gChatReq->m_msgLen);
        QString dispalyMsg = QString(buf);
        groupInfo* gInfo = searchGroupInfo(gChatReq->m_groupAccount);
        //TODO 将聊天信息存储在本地或数据库

        //将发送的消息显示在对应的聊天框内
        groupMemInfo* gmInfo = searchGroupMemInfo(gChatReq->m_userAccount, gChatReq->m_groupAccount);
        displayGroupMessage(browser_to_send, &dispalyMsg, gInfo, gmInfo);
        LOGINFO() << "[" << ite_gMap->second->name <<"]" << "[" << gmInfo->name << "]: " << buf;
        if(buf != NULL)
        {
            free(buf);
            buf = NULL;
        }
    }

    return true;
}

int Widget::refreshFriendStatusHandle(void* message)
{
    refreshFriendStatusReply* refreshStatusReply = (refreshFriendStatusReply*)message;
    LOGINFO() << "有好友登出，account: " << refreshStatusReply->m_account;

    friendInfoMap::iterator ite = m_friendInfoMap.find(refreshStatusReply->m_account);
    ite->second->status = false;

    return true;
}

int Widget::searchAccountHandle(void* message)
{
    LOGINFO() << "收到searchAccountReply";
    searchAccountReply* searchReply = (searchAccountReply*) message;
    LOGINFO() << "account: " << searchReply->m_account << " name: " << searchReply->name << " is_friend: " << searchReply->is_friend << " is_online: " << searchReply->is_online;
    emit_searchAccountFinish(searchReply);
    return true;
}

int Widget::addFriendReqHandle(void* message)
{
    addFriendInfoReq* addFriReq = (addFriendInfoReq*) message;
    LOGINFO() << "receive addFriend request...";
    LOGINFO() << "sender: " << addFriReq->senderAccount << " receiver: " << addFriReq->receiverAccount << " message: " << addFriReq->message;



    return true;
}

int Widget::addFriendReplyHandle(void* message)
{
    return true;
}

void Widget::displayUserMessage(chatTextBrowser* textBrowser, QString* message)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString uAccount = QString::number(m_uInfo.m_account);
    QString uName = QString(QLatin1String(m_uInfo.m_userName));
    QString uinfo = uName + "(" + uAccount + ") " + timeStr;
    textBrowser->append(uinfo);
    textBrowser->append(*message);
}

void Widget::displayMessage(chatTextBrowser* browser, QString* message, friendInfo* friInfo)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString friAccount = QString::number(friInfo->m_account);
    QString friName = QString(QLatin1String(friInfo->name));
    QString friendInfo = friName + "(" + friAccount + ") " + timeStr;
    browser->append(friendInfo);
    browser->append(*message);
}

void Widget::displayGroupMessage(chatTextBrowser* browser, QString* message, groupInfo* gInfo, groupMemInfo* gmInfo)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString gmAccount = QString::number(gmInfo->account);
    QString gmName = QString(QLatin1String(gmInfo->name));
    QString info = gmName + "(" + gmAccount + ") " + timeStr;
    browser->append(info);
    browser->append(*message);
}

void Widget::searchPrivateBrowser(int account)
{
    for(int i = 0; i < m_friendInfoMap.size(); i++)
    {
        if(privateChatTextBrowserList[i]->getAccount() == account)
        {
            browser_to_send =  privateChatTextBrowserList[i];
        }
    }
}

void Widget::searchGroupBrowser(int account)
{
    for(int i = 0; i < m_groupInfoMap.size(); i++)
    {
        if(groupChatTextBrowserList[i]->getGroupAccount() == account)
        {
            browser_to_send =  groupChatTextBrowserList[i];
        }
    }
}

friendInfo* Widget::searchFriendInfo(int account)
{
    friendInfoMap::iterator ite = m_friendInfoMap.find(account);
    if(ite != m_friendInfoMap.end())
    {
        return ite->second;
    }
    return NULL;
}

groupInfo* Widget::searchGroupInfo(int account)
{
    groupInfoMap::iterator ite = m_groupInfoMap.find(account);
    if(ite != m_groupInfoMap.end())
    {
        return ite->second;
    }

    return NULL;
}

groupMemInfo* Widget::searchGroupMemInfo(int account, int groupAccount)
{
    groupInfo* gInfo = searchGroupInfo(groupAccount);
    int num = gInfo->groupMemInfoList.size();
    for(int i = 0; i < num; i++)
    {
        if(gInfo->groupMemInfoList[i]->account == account)
        {
            return gInfo->groupMemInfoList[i];
        }
    }

    return NULL;
}

void Widget::on_privateChat_clicked()
{
    ui->stackedWidget_button->setCurrentWidget(privateButtonWidget);
}


void Widget::on_groupChat_clicked()
{
    ui->stackedWidget_button->setCurrentWidget(groupButtonWidget);
}

void Widget::on_sendData_clicked()
{
    LOGINFO() << "开始发送数据...";

    if(edit_to_chat == NULL)
    {
        LOGINFO() << "未选中聊天对象";
        return;
    }
    if(edit_to_chat->toPlainText().isEmpty())
    {
        LOGINFO() << "聊天数据为NULL";
        return;
    }
    QString str = edit_to_chat->toPlainText();

    if(chat_type == 0)  //私聊
    {
        privateChatReq* priChatReq;
        char* p = (char*)malloc(sizeof(privateChatReq) + str.toStdString().size());
        priChatReq = (privateChatReq*)p;

        priChatReq->m_userAccount = m_uInfo.m_account;
        priChatReq->m_msgLen = str.toStdString().size();
        priChatReq->type = 0;   //发送文本信息
        priChatReq->m_friendAccount = friendAccount_to_chat;
        memmove(p + sizeof(privateChatReq), str.toStdString().c_str(), str.toStdString().size());
        sendMsg(p, sizeof(privateChatReq) + str.toStdString().size(), command_privateChat);

        //将用户的聊天信息显示在textbrowser内
        emit emit_friendAccount(edit_to_chat->getAccount());
        if(browser_to_send != NULL)
        {
            displayUserMessage(browser_to_send, &str);
            edit_to_chat->clear();
        }

        if(p != NULL)
        {
            free(p);
            p = NULL;
        }
    }
    else if(chat_type == 1) //群聊
    {
        groupChatReq* gChatReq;
        char* p = (char*)malloc(sizeof(groupChatReq) + str.toStdString().size());
        gChatReq = (groupChatReq*)p;

        gChatReq->m_userAccount = m_uInfo.m_account;
        gChatReq->m_msgLen = str.toStdString().size();
        gChatReq->type = 0;
        gChatReq->m_groupAccount = groupAccount_to_chat;
        memmove(p + sizeof(groupChatReq), str.toStdString().c_str(), str.toStdString().size());
        sendMsg(p, sizeof(groupChatReq) + str.toStdString().size(), command_groupChat);

        emit emit_groupAccount(edit_to_chat->getGroupAccount());
        if(browser_to_send != NULL)
        {
            displayUserMessage(browser_to_send, &str);
            edit_to_chat->clear();
        }

        if(p != NULL)
        {
            free(p);
            p = NULL;
        }
    }
}

bool Widget::eventFilter(QObject* target, QEvent* event)
{
    for(int i = 0; i < m_friendInfoMap.size(); i++)
    {
        if(target == privateChatTextEditList[i])
        {
            QKeyEvent* k = static_cast<QKeyEvent*>(event);
            if(k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter)
            {
                on_sendData_clicked();
                return true;
            }
        }
    }

    for(int i = 0; i < groupChatTextEditList.size(); i++)
    {
        if(target == groupChatTextEditList[i])
        {
            QKeyEvent* k = static_cast<QKeyEvent*>(event);
            if(k->key() == Qt::Key_Return || k->key() == Qt::Key_Enter)
            {
                on_sendData_clicked();
                return true;
            }
        }
    }

    return false;
}

void Widget::on_userInfo_button_clicked()
{
    userInfoDlg uInfoDlg(&m_uInfo);
    uInfoDlg.show();
    uInfoDlg.exec();
}


void Widget::on_pushButton_add_clicked()
{
    addFriendDlg* addFriDlg = new addFriendDlg();
    connect(addFriDlg, SIGNAL(emit_searchButtonClicked(int)), this, SLOT(send_searchAccount(int)));
    connect(this, SIGNAL(emit_searchAccountFinish(searchAccountReply*)), addFriDlg, SLOT(displaySearchAccount(searchAccountReply*)));
    connect(addFriDlg, SIGNAL(emit_addButtonClicked(int,char*)), this, SLOT(send_addFriend(int,char*)));
    //addFriDlg->setParent(this);
    addFriDlg->show();
}

void Widget::send_searchAccount(int account)
{
    searchAccountReq* searchReq = new searchAccountReq;
    searchReq->account = account;
    sendMsg((char*)searchReq, sizeof(searchAccountReq), command_searchAccount);
    if(searchReq != NULL)
    {
        delete searchReq;
        searchReq = NULL;
    }
}

void Widget::send_addFriend(int account, char* message)
{
    addFriendInfoReq* addFriReq = new addFriendInfoReq;
    addFriReq->senderAccount = m_uInfo.m_account;
    addFriReq->receiverAccount = account;
    memmove(addFriReq->message, message, strlen(message));
    sendMsg(addFriReq, sizeof(addFriendInfoReq), command_addFriend);
    if(message != NULL)
    {
        free(message);
        message = NULL;
    }
    if(addFriReq != NULL)
    {
        delete addFriReq;
        addFriReq = NULL;
    }
}

void Widget::messageAlert()
{
    QString run_path = QCoreApplication::applicationDirPath();
    QString path = run_path + "/source/QQ.wav";
    player->setSource(QUrl::fromLocalFile(path));
    player->play();
}

void Widget::on_pushButton_TODO_clicked()
{

}

