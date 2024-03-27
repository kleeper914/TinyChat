#include "userinfodlg.h"
#include "ui_userinfodlg.h"
#include <QMessageBox>

userInfoDlg::userInfoDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::userInfoDlg)
{
    ui->setupUi(this);
}

userInfoDlg::userInfoDlg(userInfo* info)
    :ui(new Ui::userInfoDlg)
{
    ui->setupUi(this);
    uInfo = info;
    init();
}

userInfoDlg::~userInfoDlg()
{
    delete ui;
}

void userInfoDlg::init()
{
    this->setWindowTitle("个人信息");
    ui->label_username->setText(QString(uInfo->m_userName));
    ui->label_account->setText(QString::number(uInfo->m_account));
}

void userInfoDlg::on_pushButton_password_clicked()
{
    QMessageBox::information(this, tr("请妥善保管个人信息"), QString(uInfo->m_password));
}

