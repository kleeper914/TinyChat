#include "addfrienddlg.h"
#include "ui_addfrienddlg.h"
#include "widget.h"
#include <QLabel>

addFriendDlg::addFriendDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addFriendDlg)
{
    ui->setupUi(this);
}

addFriendDlg::~addFriendDlg()
{
    delete ui;
}

void addFriendDlg::on_pushButton_searchFriend_clicked()
{
    LOGINFO() << "searchFriend clicked...";
    int account = ui->lineEdit_searchFriend->text().toInt();
    emit emit_searchButtonClicked(account);
}

void addFriendDlg::displaySearchAccount(searchAccountReply* searchReply)
{
    LOGINFO() << "display search account info";
    ui->label_name->setText("name: "+QString(searchReply->name));
    ui->label_account->setText("account: " + QString::number(searchReply->m_account));
    if(searchReply->is_friend == true)
    {
        ui->label_is_friend->setText("是否为好友: 是");
    }
    else
    {
        ui->label_is_friend->setText("是否为好友: 否");
    }
    if(searchReply->is_online == true)
    {
        ui->label_is_online->setText("是否在线: 是");
    }
    else
    {
        ui->label_is_online->setText("是否在线: 否");
    }
}


void addFriendDlg::on_pushButton_addFriend_clicked()
{
    LOGINFO() << "addFriend clicked...";
    int account = ui->lineEdit_searchFriend->text().toInt();
    char* message = (char*)malloc(ui->lineEdit_confirmInfo->text().toStdString().length());
    memset(message, 0, ui->lineEdit_confirmInfo->text().toStdString().length());
    memmove(message, ui->lineEdit_confirmInfo->text().toStdString().c_str(), ui->lineEdit_confirmInfo->text().toStdString().length());
    emit emit_addButtonClicked(account, message);
}

