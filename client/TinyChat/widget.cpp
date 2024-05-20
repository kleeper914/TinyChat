#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "widget.h"
#include "ui_widget.h"
#include "logindlg.h"
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QNetworkProxy>
#include <QUrl>
#include <QMenu>
#include <QAction>

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

    QMenu* menu_add = new QMenu(this);
    action_addFriend = new QAction("添加好友", this);
    action_createGroup = new QAction("创建群聊", this);
    menu_add->addAction(action_addFriend);
    menu_add->addAction(action_createGroup);
    menu_add->setWindowFlags(menu_add->windowFlags() | Qt::FramelessWindowHint);
    menu_add->setAttribute(Qt::WA_TranslucentBackground);
    menu_add->setStyleSheet("QMenu {border-radius:5px; font-family:'Microsoft Yahei'; font-size:14px; color:#000;}"
                            "QMenu::item {height:30px; width:100px padding-left:20px; border:1px solid none;}"
                            "QMenu::item:selected {background-color:rgb(0, 120, 215); \
                            padding-left:20px; border:1px solid rgb(65, 173, 255);}");
    ui->pushButton_add->setMenu(menu_add);
    connect(menu_add, &QMenu::triggered, this, &Widget::add_action_triggered);

    tododlg = new todoDlg;
    connect(tododlg, SIGNAL(emit_addFriend_agree(addFriendInfoReq*)), this, SLOT(addFriend_agree(addFriendInfoReq*)));
    connect(tododlg, SIGNAL(emit_addFriend_reject(addFriendInfoReq*)), this, SLOT(addFriend_reject(addFriendInfoReq*)));

    connect(this, SIGNAL(friendList_finish()), this, SLOT(init_privateChat()));
}

void Widget::add_action_triggered(QAction* action)
{
    if(action == action_addFriend)
    {
        addFriendDlg* addFriDlg = new addFriendDlg();
        connect(addFriDlg, SIGNAL(emit_searchButtonClicked(int)), this, SLOT(send_searchAccount(int)));
        connect(this, SIGNAL(emit_searchAccountFinish(searchAccountReply*)), addFriDlg, SLOT(displaySearchAccount(searchAccountReply*)));
        connect(addFriDlg, SIGNAL(emit_addButtonClicked(int,char*)), this, SLOT(send_addFriend(int,char*)));
        //addFriDlg->setParent(this);
        addFriDlg->show();
    }
    else if(action == action_createGroup)
    {
        createGroupDlg* createGrpDlg = new createGroupDlg;
        connect(createGrpDlg, SIGNAL(emit_group_mem_info(groupInfo*)), this, SLOT(send_createGroup(groupInfo*)));
        createGrpDlg->init_friend_info(&m_friendInfoMap);
        createGrpDlg->show();
    }
}

void Widget::send_createGroup(groupInfo* create_group)
{
    char* p = (char*)malloc(sizeof(createGroupReq) + sizeof(groupMemInfo) * create_group->size);
    createGroupReq* createGrpReq = (createGroupReq*)p;
    createGrpReq->master_account = m_uInfo.m_account;
    memcpy(createGrpReq->group_name, create_group->name, sizeof(create_group->name));
    createGrpReq->size = create_group->size;
    LOGINFO() << "master_account: " << createGrpReq->master_account << " name: " << createGrpReq->group_name << " size: " << createGrpReq->size;
    //传递的create_group中存有除用户外其余的群聊成员
    //将用户本身信息存入第一个群聊成员中
    groupMemInfo* groupMem = new groupMemInfo;
    groupMem->account = m_uInfo.m_account;
    memcpy(groupMem->name, m_uInfo.m_userName, sizeof(m_uInfo.m_userName));
    groupMem->right = 0;
    memmove(p + sizeof(createGroupReq), (char*)groupMem, sizeof(groupMemInfo));
    for(int i = 0; i < create_group->size - 1; i++)
    {
        groupMemInfo* group_mem = new groupMemInfo;
        group_mem->account = create_group->groupMemInfoList[i]->account;
        memcpy(group_mem->name, create_group->groupMemInfoList[i]->name, sizeof(create_group->groupMemInfoList[i]->name));
        group_mem->right = create_group->groupMemInfoList[i]->right;
        memmove(p+sizeof(createGroupReq)+sizeof(groupMemInfo)*(i+1), group_mem, sizeof(groupMemInfo));
        //LOGINFO() << "member" << i+1 << " account: " << group_mem->account << " name: " << group_mem->name << " right: " << group_mem->right;
        if(group_mem != NULL)
        {
            delete group_mem; group_mem = NULL;
        }
    }

    sendMsg(p, sizeof(createGroupReq) + sizeof(groupMemInfo) * create_group->size, command_createGroup);
    LOGINFO() << "发送创建群聊请求成功...";

    if(create_group != NULL)
    {
        delete create_group;
        create_group = NULL;
    }
    if(p != NULL)
    {
        free(p); p = NULL;
    }
    if(p != NULL)
    {
        free(p);
        p = NULL;
    }
}

void Widget::init_privateChat()
{
    privateButtonWidget = new QWidget;
    ui->stackedWidget_button->addWidget(privateButtonWidget);
    privateButtonWidgetLayout = new QVBoxLayout;
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
            if(friendButtonList[i]->get_num_not_read() != 0)
            {
                friendButtonList[i]->num_not_read_zero();
            }
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

    //发出登出请求
    sendMsg(NULL, 0, command_logout);
    //TODO
    //优化登出请求发出形式
    QTime timeout = QTime::currentTime().addMSecs(300);    //0.3s超时
    while(QTime::currentTime() < timeout)   //如果服务端3s没有作出答复，则直接退出程序
    {
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
    case command_refreshFriendList:
        refreshFriendListHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_createGroup:
        createGroupHandle(msgPacket->body + sizeof(messageBody));
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
        int num_new_friend = new_friend_button_list.size();
        for(int i = 0; i < m_friendInfoMap.size()-num_new_friend; i++)
        {
            if(sendAccount == friendButtonList[i]->getFriendAccount())
            {
                LOGINFO() << "找到了对应的私聊按钮";
                button_to_change = friendButtonList[i];
            }
        }
        for(int i = 0; i < num_new_friend; i++)
        {
            if(sendAccount == new_friend_button_list[i]->getFriendAccount())
            {
                LOGINFO() << "找到了对应的私聊按钮";
                button_to_change = new_friend_button_list[i];
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

    //将收到的请求同步到tododlg中
    tododlg->insert_addFriendReqRecv(addFriReq);

    return true;
}

int Widget::addFriendReplyHandle(void* message)
{
    addFriendInfoReply* addFriReply = (addFriendInfoReply*)message;
    LOGINFO() << "receive addFriendInfoReply, senderAccount: " << addFriReply->senderAccount << " receiverAccount: " << addFriReply->receiverAccount << "是否同意: " << addFriReply->is_agree;

    if(addFriReply->is_agree == true)
    {
        //todo
        //向服务端发送更新好友表信息
        refreshFriendReq* refreshFriReq = new refreshFriendReq;
        refreshFriReq->account = addFriReply->senderAccount;
        sendMsg((char*)refreshFriReq, sizeof(refreshFriendReq), command_refreshFriendList);
        if(refreshFriReq != NULL)
        {
            delete refreshFriReq;
            refreshFriReq = NULL;
        }
        LOGINFO() << "发送更新好友信息请求...";
        //todo
        //更新tododlg ui
        QString str = "state: 已同意";
        tododlg->finish_addFriendReqSend(addFriReply->senderAccount, str);
    }
    else
    {
        QString str = "state: 已拒绝";
        tododlg->finish_addFriendReqSend(addFriReply->senderAccount, str);
    }

    return true;
}

int Widget::refreshFriendListHandle(void* message)
{
    refreshFriendReply* refreshFriReply = (refreshFriendReply*)message;
    LOGINFO() << "receive refreshFriendReply account = " << refreshFriReply->account << " name = " << refreshFriReply->name;

    //todo
    //将好友信息插入到friendInfoMap
    friendInfo* new_friend = new friendInfo;
    new_friend->m_account = refreshFriReply->account;
    memcpy(new_friend->name, refreshFriReply->name, sizeof(refreshFriReply->name));
    new_friend->status = true;
    m_friendInfoMap.insert(std::pair(refreshFriReply->account, new_friend));

    //检查是否插入好友表成功
    friendInfoMap::iterator ite_friend = m_friendInfoMap.find(new_friend->m_account);
    if(ite_friend != m_friendInfoMap.end())
    {
        LOGINFO() << "插入好友表成功";
    }

    //创建好友聊天界面
    //创建按钮
    privateChatButton* new_friend_button = new privateChatButton(new_friend);
    new_friend_button->setText(refreshFriReply->name);
    privateButtonWidgetLayout->addWidget(new_friend_button);
    new_friend_button_list.push_back(new_friend_button);
    //创建对话框
    privateChatPage* new_friend_page = new privateChatPage(new_friend);
    ui->stackedWidget_chat->addWidget(new_friend_page);
    new_friend_page_list.push_back(new_friend_page);
    QVBoxLayout* new_friend_chat_layout = new QVBoxLayout;
    new_friend_chat_layout->setAlignment(Qt::AlignTop);
    new_friend_page->setLayout(new_friend_chat_layout);
    new_friend_chat_layout_list.push_back(new_friend_chat_layout);

    //加入textedit和textbrowser组件
    chatTextBrowser* new_friend_browser = new chatTextBrowser(new_friend);
    new_friend_chat_layout->addWidget(new_friend_browser);
    new_friend_browser_list.push_back(new_friend_browser);
    chatTextEdit* new_friend_edit = new chatTextEdit(new_friend);
    //new_friend_edit->installEventFilter(this);
    new_friend_chat_layout->addWidget(new_friend_edit);
    new_friend_edit_list.push_back(new_friend_edit);
    new_friend_chat_layout->setStretch(0, 3);
    new_friend_chat_layout->setStretch(1, 1);

    int num = new_friend_button_list.size();
    LOGINFO() << "新建好友信息数量： " << num;
    connect(new_friend_button_list[num - 1], &QPushButton::clicked, [this, i = num-1](){
        chat_type = 0;
        friendAccount_to_chat = new_friend_button_list[i]->getFriendAccount();
        LOGINFO() << "当前要发送数据的好友账号是： " << friendAccount_to_chat;
        ui->stackedWidget_chat->setCurrentWidget(new_friend_page_list[i]);
        LOGINFO() << "设置聊天框标签为" << new_friend_button_list[i]->getFriendName();
        ui->chatNameLabel->setText(new_friend_button_list[i]->getFriendName());
        if(new_friend_button_list[i]->getFriendStatus() == true)
        {
            ui->status_label->setText("在线");
        }
        else if(new_friend_button_list[i]->getFriendStatus() == false)
        {
            ui->status_label->setText("离线");
        }
        edit_to_chat = new_friend_edit_list[i];
        //将消息未读数置零
        if(new_friend_button_list[i]->get_num_not_read() != 0)
        {
            new_friend_button_list[i]->num_not_read_zero();
        }
    });

    LOGINFO() << "新建好友界面完成";

    return true;
}

int Widget::createGroupHandle(void* message)
{
    createGroupReply* createGrpReply = (createGroupReply*)message;
    LOGINFO() << "收到创建群聊的回复...";
    LOGINFO() << "group_account: " << createGrpReply->group_account << " group_name: " << createGrpReply->group_name << " group_size: " << createGrpReply->size << " master_account: " << createGrpReply->master_account;

    //向m_groupInfoMap插入群聊信息
    groupInfo* new_group = new groupInfo;
    new_group->account = createGrpReply->group_account;
    memcpy(new_group->name, createGrpReply->group_name, sizeof(createGrpReply->group_name));
    new_group->size = createGrpReply->size;

    return 0;
}

void Widget::displayUserMessage(chatTextBrowser* textBrowser, QString* message)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString uAccount = QString::number(m_uInfo.m_account);
    QString uName = QString(QLatin1String(m_uInfo.m_userName));
    QString uinfo = "<font color=\'#00FF00\'>"+uName + "(" + uAccount + ") " + timeStr+"</font>";
    textBrowser->append(uinfo);
    textBrowser->append(*message);
}

void Widget::displayMessage(chatTextBrowser* browser, QString* message, friendInfo* friInfo)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString friAccount = QString::number(friInfo->m_account);
    QString friName = QString(QLatin1String(friInfo->name));
    QString friendInfo = "<font color=\'#0000FF\'>"+friName + "(" + friAccount + ") " + timeStr+"</font>";
    browser->append(friendInfo);
    browser->append(*message);
}

void Widget::displayGroupMessage(chatTextBrowser* browser, QString* message, groupInfo* gInfo, groupMemInfo* gmInfo)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString gmAccount = QString::number(gmInfo->account);
    QString gmName = QString(QLatin1String(gmInfo->name));
    QString info = "<font color=\'#0000FF\'>"+gmName + "(" + gmAccount + ") " + timeStr+"</font>";
    browser->append(info);
    browser->append(*message);
}

void Widget::searchPrivateBrowser(int account)
{
    int num_new_friend = new_friend_browser_list.size();
    for(int i = 0; i < m_friendInfoMap.size()-num_new_friend; i++)
    {
        if(privateChatTextBrowserList[i]->getAccount() == account)
        {
            browser_to_send =  privateChatTextBrowserList[i];
        }
    }
    for(int i = 0; i < num_new_friend; i++)
    {
        if(new_friend_browser_list[i]->getAccount() == account)
        {
            browser_to_send = new_friend_browser_list[i];
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
    LOGINFO() << "将要发送的信息是： " << str;

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
        LOGINFO() << "向服务器发送私聊请求";

        //将用户的聊天信息显示在textbrowser内
        emit emit_friendAccount(edit_to_chat->getAccount());
        LOGINFO() << "将要发送消息的聊天框 account: " << browser_to_send->getAccount();
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
    memset(addFriReq->message, 0, sizeof(addFriReq->message));
    memmove(addFriReq->message, message, strlen(message));
    sendMsg(addFriReq, sizeof(addFriendInfoReq), command_addFriend);
    if(message != NULL)
    {
        free(message);
        message = NULL;
    }

    //todo
    //将添加好友请求显示在todoDlg中，并实时更新状态
    //接受者未受理，显示未回复
    //接收者回复，显示删除按钮，删除该条记录
    //接收者拒绝，显示已拒绝
    //接收者同意，显示已同意
    tododlg->insert_addFriendReqSend(addFriReq);
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
    tododlg->show();
}

void Widget::addFriend_agree(addFriendInfoReq* addFriReq)
{
    LOGINFO() << "同意好友请求";
    addFriendInfoReply* addFriReply = new addFriendInfoReply;
    addFriReply->senderAccount = m_uInfo.m_account;
    addFriReply->receiverAccount = addFriReq->senderAccount;
    addFriReply->is_agree = true;
    LOGINFO() << "发送好友请求的回复";
    sendMsg(addFriReply, sizeof(addFriendInfoReply), command_addFriend, 0, 2);
    //并发送refreshFriend请求
    refreshFriendReq* refreshFriReq = new refreshFriendReq;
    refreshFriReq->account = addFriReq->senderAccount;
    sendMsg((char*)refreshFriReq, sizeof(refreshFriendReq), command_refreshFriendList);
    if(refreshFriReq != NULL)
    {
        delete refreshFriReq;
        refreshFriReq = NULL;
    }
    LOGINFO() << "发送更新好友信息请求...";
    if(addFriReply != NULL)
    {
        delete addFriReply;
        addFriReply = NULL;
    }
    //todo
    //更新tododlg ui
    QString str = "state: 已同意";
    tododlg->finish_addFriendReqRecv(addFriReq->senderAccount, str);
}

void Widget::addFriend_reject(addFriendInfoReq* addFriReq)
{
    LOGINFO() << "拒绝好友请求";
    addFriendInfoReply* addFriReply = new addFriendInfoReply;
    addFriReply->senderAccount = m_uInfo.m_account;
    addFriReply->receiverAccount = addFriReq->senderAccount;
    addFriReply->is_agree = false;
    sendMsg(addFriReply, sizeof(addFriendInfoReply), command_addFriend, 0, 2);
    LOGINFO() << "发送好友请求的回复";
    if(addFriReply != NULL)
    {
        delete addFriReply;
        addFriReply = NULL;
    }
    //todo
    //更新tododlg ui
    QString str = "state: 已拒绝";
    tododlg->finish_addFriendReqRecv(addFriReq->senderAccount, str);
}
