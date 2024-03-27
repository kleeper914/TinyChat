#define _CRT_SECURE_NO_WARNINGS
#include "logindlg.h"
#include "ui_logindlg.h"

LoginDlg::LoginDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDlg)
{
    ui->setupUi(this);
    init();
}

LoginDlg::~LoginDlg()
{
    delete ui;
}

void LoginDlg::init()
{
    this->setWindowTitle("登陆");
    memset(&m_uInfo, 0, sizeof(userInfo));
}

void LoginDlg::on_pushButton_login_clicked()
{
    m_uInfo.m_account = ui->edit_account->text().toInt();
    memset(m_uInfo.m_password, 0, sizeof(m_uInfo.m_password));
    strncpy(m_uInfo.m_password, ui->edit_password->text().toStdString().c_str(), ui->edit_password->text().size());
    //关闭窗口并发送accept()信号
    return accept();
}


void LoginDlg::on_pushButton_regist_clicked()
{
    registerDlg regDlg;
    regDlg.show();
    if(regDlg.exec() == QDialog::Accepted)
    {
        if(regDlg.getStatus())
        {
            userInfo uinfo = regDlg.getUserInfo();
            //注册完成后将得到的账号写在账号框上
            ui->edit_account->setText(QString::number(uinfo.m_account));
            memmove(m_uInfo.m_userName, uinfo.m_userName, sizeof(uinfo.m_userName));
            m_uInfo.m_account = uinfo.m_account;
            memmove(m_uInfo.m_password, uinfo.m_password, sizeof(uinfo.m_password));
            LOGINFO() << "注册成功，账号: " << m_uInfo.m_account;
        }
    }
}

