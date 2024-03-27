#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "widget.h"
#include "ui_widget.h"
#include "logindlg.h"
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QNetworkProxy>

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

    connect(this, SIGNAL(friendList_finish()), this, SLOT(init_privateChat()));
}

void Widget::init_privateChat()
{
    QWidget* privateButtonWidget = new QWidget;
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
        // connect(friendButtonList[i], &QPushButton::clicked, [this, i](){
        //     friendAccount_to_chat = friendButtonList[i]->getFriendAccount();
        //     LOGINFO() << "当前要发送数据的好友账号是: " << friendAccount_to_chat;
        // });
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
        privateChatLayoutList[i]->addWidget(privateChatTextEditList[i]);
        privateChatLayoutList[i]->setStretch(0, 3);
        privateChatLayoutList[i]->setStretch(1, 1);

        connect(friendButtonList[i], &QPushButton::clicked, [this, i](){
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
        });
    }
    LOGINFO() << "私聊界面初始化完成";
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

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));

    getFriendList();
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
    free(pBody);
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
    LOGINFO() << "mark : " << head->mark << "\tlength : " << head->len << '\n';
    LOGINFO() << "received command: " << body->command << '\n';
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
    case command_privateChat:
        privateChatHandle(msgPacket->body + sizeof(messageBody));
        break;
    case command_refreshFriendStatus:
        refreshFriendStatusHandle(msgPacket->body + sizeof(messageBody));
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

int Widget::privateChatHandle(void* message)
{
    LOGINFO() << "收到聊天消息";
    privateChatReq* priChatReq = (privateChatReq*)message;
    //如果发送请求中的好友账号为用户账号
    if(priChatReq->m_friendAccount == m_uInfo.m_account)
    {
        int sendAccount = priChatReq->m_userAccount;    //发送者账号即为发送请求中的用户账户
        chatTextBrowser* browser = searchTextBrowser(sendAccount);
        char* buf = (char*)malloc(priChatReq->m_msgLen + 1);
        memset(buf, 0, priChatReq->m_msgLen + 1);
        memmove(buf, (char*)message + sizeof(privateChatReq), priChatReq->m_msgLen);
        QString displayMsg = QString(buf);
        friendInfo* friInfo = searchFriendInfo(sendAccount);
        //TODO 将聊天信息存储在本地或数据库

        //将发送的消息显示在对应的聊天框内
        displayMessage(browser, &displayMsg, friInfo);
        LOGINFO() << "[" << friInfo->name << "]" << buf;
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

chatTextBrowser* Widget::searchTextBrowser(int account)
{
    for(int i = 0; i < m_friendInfoMap.size(); i++)
    {
        if(privateChatTextBrowserList[i]->getAccount() == account)
        {
            return privateChatTextBrowserList[i];
        }
    }

    return NULL;
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

void Widget::on_privateChat_clicked()
{
    ui->stackedWidget_button->setCurrentIndex(2);
}


void Widget::on_groupChat_clicked()
{
    ui->stackedWidget_button->setCurrentIndex(1);
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
    chatTextBrowser* sendBrowser = searchTextBrowser(edit_to_chat->getAccount());
    if(sendBrowser != NULL)
    {
        displayUserMessage(sendBrowser, &str);
        edit_to_chat->clear();
    }

    if(p != NULL)
    {
        free(p);
        p = NULL;
    }
}


void Widget::on_userInfo_button_clicked()
{
    userInfoDlg uInfoDlg(&m_uInfo);
    uInfoDlg.show();
    uInfoDlg.exec();
}

