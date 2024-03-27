#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "basic.h"
#include "task.h"
#include <QCloseEvent>
#include <QSplitter>
#include "privatechatbutton.h"
#include "privatechatpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "chattextedit.h"
#include "chattextbrowser.h"
#include <QPushButton>
#include <QLabel>
#include <QFont>
#include "registerdlg.h"
#include "userinfodlg.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

signals:
    void friendList_finish();

public slots:
    void init_privateChat();

public:
    int loginHandle(void* message);
    int logoutHandle(void* msg);
    int registerHandle(void* message);
    int friendListHandle(void* message);
    int privateChatHandle(void* message);
    int getFriendList();
    int refreshFriendStatusHandle(void* message);

    chatTextBrowser* searchTextBrowser(int account);
    friendInfo* searchFriendInfo(int account);

    void displayUserMessage(chatTextBrowser*, QString*);       //将用户发出的聊天信息显示在textbrowser内
    void displayMessage(chatTextBrowser*, QString*, friendInfo*);  //将收到的聊天信息显示在textbrowser内

private:
    void init();
    void initUI();
    void closeEvent(QCloseEvent* event);
    void login();
    void sendMsg(void* buf, int bufLen, int command, int error = 0, int type = 0);
private slots:
    int signals_handle(messagePacket*);
    void readyReadSlot();
    void on_privateChat_clicked();
    void on_groupChat_clicked();

    void on_sendData_clicked();

    void on_userInfo_button_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket* m_socket;
    Task*       m_task;
    bool        socketState;
    bool        is_login;
    userInfo    m_uInfo;
    int         friendAccount_to_chat;      //将要发送信息的好友，通过privateChatPage中的按钮点击改变
    chatTextEdit*  edit_to_chat;               //将要发送信息的编辑框
    friendInfoMap   m_friendInfoMap;        //存放所有好友信息
private:
    privateChatButton** friendButtonList;           //存放好友按钮索引
    privateChatPage**   privateChatPageList;        //存放好友聊天页面索引
    QVBoxLayout**       privateChatLayoutList;      //存放好友聊天页面layout索引
    chatTextEdit**         privateChatTextEditList;    //存放好友编辑框索引
    chatTextBrowser**      privateChatTextBrowserList; //存放好友浏览框索引
    //QSplitter*  mainSplitter;
};
#endif // WIDGET_H
