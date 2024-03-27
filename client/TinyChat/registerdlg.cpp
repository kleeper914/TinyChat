#include "registerdlg.h"
#include "ui_registerdlg.h"
#include <QNetworkProxy>

registerDlg::registerDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::registerDlg)
{
    ui->setupUi(this);
    init();
}

registerDlg::~registerDlg()
{
    delete ui;
}

void registerDlg::init()
{
    this->setWindowTitle("注册");
    memset(&m_userInfo, 0, sizeof(userInfo));
    m_status = false;
}

void registerDlg::on_pushButton_clicked()
{
    //使用户能够在使用代理的情况下连接至服务器
    QNetworkProxyFactory::setUseSystemConfiguration(false);
    m_socket = new QTcpSocket;
    QString ipAddr = SERVER_IP;
    quint16 port = SERVER_PORT;

    m_socket->connectToHost(ipAddr, port);
    if(m_socket->waitForConnected())
    {
        LOGINFO() << "注册，连接服务器成功";
    }
    else
    {
        LOGINFO() << m_socket->error();
        return;
    }

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));

    QString password = ui->lineEdit_password->text();
    QString passwordConfirm = ui->lineEdit_passwordConfrim->text();
    if(password != passwordConfirm)
    {
        QMessageBox::information(this, tr("提示"), "两次输入密码不相同");
        return;
    }
    LOGINFO() << "开始注册...";
    registerInfoReq* regReq;
    char* p = (char*)malloc(sizeof(registerInfoReq));
    memset(p, 0, sizeof(registerInfoReq));
    regReq = (registerInfoReq*) p;
    memmove(regReq->m_password, password.toStdString().c_str(), password.toStdString().size());
    memmove(regReq->m_name, ui->lineEdit_username->text().toStdString().c_str(), ui->lineEdit_username->text().toStdString().size());
    LOGINFO() << "注册信息 用户名: " << regReq->m_name << " 密码: " << regReq->m_password;
    sendMsg(regReq, sizeof(registerInfoReq), command_register, 0, 0);
    if(p != NULL)
    {
        free(p);
        p = NULL;
    }
}

void registerDlg::readyReadSlot()
{
    messageHead header;
    /*接收注册响应*/
    memset(&header,'\0',sizeof(messageHead));
    int len = m_socket->read((char*)&header,sizeof(messageHead));
    LOGINFO()<<"readLen:"<<len;
    LOGINFO()<<"length:"<<header.len;
    char *p = (char*)malloc(header.len);
    messageBody* pBody = (messageBody*)p;
    m_socket->read((char*)pBody,header.len);

    if(pBody->error == 0){
        m_status = true;
    }

    registerInfoReply* resp = (registerInfoReply*)(p+sizeof(messageBody));
    LOGINFO()<<"account:"<<resp->m_account;

    m_userInfo.m_account = resp->m_account;
    memmove(m_userInfo.m_userName, ui->lineEdit_username->text().toStdString().c_str(), ui->lineEdit_username->text().toStdString().size());
    memmove(m_userInfo.m_password, ui->lineEdit_password->text().toStdString().c_str(), ui->lineEdit_password->text().toStdString().size());

    free(p);
    return accept();    //Closes the dialog and emits the accepted() signal.
}

void registerDlg::sendMsg(void* buf, int bufLen, int command, int error, int type)
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

bool registerDlg::getStatus()
{
    return m_status;
}

userInfo registerDlg::getUserInfo()
{
    return m_userInfo;
}
