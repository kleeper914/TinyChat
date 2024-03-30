#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "basic.h"
#include "task.h"
#include <QCloseEvent>
#include <QSplitter>
#include "privatechatbutton.h"
#include "privatechatpage.h"
#include "groupchatbutton.h"
#include "groupchatpage.h"
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
    void groupList_finish();
    void groupInsert_finish();
    void emit_friendAccount(int);
    void emit_groupAccount(int);

public slots:
    void init_privateChat();
    void insert_groupPage();
    void connnect_groupEvent();
    void searchPrivateBrowser(int);
    void searchGroupBrowser(int);

public:
    int loginHandle(void* message);
    int logoutHandle(void* msg);
    int registerHandle(void* message);
    int friendListHandle(void* message);
    int groupListHandle(void* message);
    int privateChatHandle(void* message);
    int groupChatHandle(void* message);
    int getFriendList();
    int getGroupList();
    int refreshFriendStatusHandle(void* message);

    friendInfo* searchFriendInfo(int account);
    groupInfo* searchGroupInfo(int account);
    groupMemInfo* searchGroupMemInfo(int account, int groupAccount);

    void displayUserMessage(chatTextBrowser*, QString*);       //将用户发出的聊天信息显示在textbrowser内
    void displayMessage(chatTextBrowser*, QString*, friendInfo*);  //将收到的聊天信息显示在textbrowser内
    void displayGroupMessage(chatTextBrowser*, QString*, groupInfo*, groupMemInfo*);    //将收到的群聊消息显示在对应textbrowser内

private:
    void init();
    void initUI();
    void initGroupChat();
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
    int         friendAccount_to_chat;          //将要发送信息的好友，通过privateChatPage中的按钮点击改变
    chatTextEdit*  edit_to_chat;                //将要发送信息的编辑框
    chatTextBrowser*    browser_to_send;        //将要显示信息的browser
    int         chat_type;                      //当前要发生的信息的类型，0-私聊，1-群聊
    int         groupAccount_to_insert;         //要添加的群聊信息的账号
    int         groupAccount_to_chat;           //当前聊天的群聊账号
    friendInfoMap   m_friendInfoMap;            //存放所有好友信息
    groupInfoMap m_groupInfoMap;                //存放所有群聊信息
private:
    QWidget*            privateButtonWidget;
    privateChatButton** friendButtonList;           //存放好友按钮索引
    privateChatPage**   privateChatPageList;        //存放好友聊天页面索引
    QVBoxLayout**       privateChatLayoutList;      //存放好友聊天页面layout索引
    chatTextEdit**         privateChatTextEditList;    //存放好友编辑框索引
    chatTextBrowser**      privateChatTextBrowserList; //存放好友浏览框索引
private:
    QWidget*            groupButtonWidget;
    QVBoxLayout* groupButtonWidgetLayout;
    vector<groupChatButton*>   groupButtonList;        //存放群聊按钮索引
    vector<groupChatPage*>     groupChatPageList;      //存放群聊页面索引
    vector<QVBoxLayout*>       groupChatLayoutList;    //存放群聊页面layout索引
    vector<chatTextEdit*>      groupChatTextEditList;  //存放群聊编辑框索引
    vector<chatTextBrowser*>   groupChatTextBrowserList;   //存放群聊浏览框索引
};
#endif // WIDGET_H
