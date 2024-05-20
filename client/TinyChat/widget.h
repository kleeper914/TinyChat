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
#include <QMediaPlayer>
#include <QAudioOutput>
#include "registerdlg.h"
#include "userinfodlg.h"
#include "addfrienddlg.h"
#include "creategroupdlg.h"
#include "tododlg.h"
#include <QAction>

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
    void emit_searchAccount(void* message);
    void emit_searchAccountFinish(searchAccountReply*);

public slots:
    void init_privateChat();
    void insert_groupPage();
    void connnect_groupEvent();
    void searchPrivateBrowser(int);
    void searchGroupBrowser(int);

public:
    int getFriendList();
    int getGroupList();
    int loginHandle(void* message);
    int logoutHandle(void* msg);
    int registerHandle(void* message);
    int friendListHandle(void* message);
    int groupListHandle(void* message);
    int privateChatHandle(void* message);
    int groupChatHandle(void* message);
    int refreshFriendStatusHandle(void* message);
    int searchAccountHandle(void* message);
    int addFriendReqHandle(void* message);
    int addFriendReplyHandle(void* message);
    int refreshFriendListHandle(void* message);
    int createGroupHandle(void* message);

    bool eventFilter(QObject* target, QEvent* event) override;
    void messageAlert();

    friendInfo* searchFriendInfo(int account);
    groupInfo* searchGroupInfo(int account);
    groupMemInfo* searchGroupMemInfo(int account, int groupAccount);
    privateChatButton* searchPrivateButton(int account);

    void displayUserMessage(chatTextBrowser*, QString*);       //将用户发出的聊天信息显示在textbrowser内
    void displayMessage(chatTextBrowser*, QString*, friendInfo*);  //将收到的聊天信息显示在textbrowser内
    void displayGroupMessage(chatTextBrowser*, QString*, groupInfo*, groupMemInfo*);    //将收到的群聊消息显示在对应textbrowser内

private:
    void init();
    void initUI();
    void initGroupChat();
    void closeEvent(QCloseEvent* event) override;
    void login();
    void sendMsg(void* buf, int bufLen, int command, int error = 0, int type = 1);
private slots:
    int signals_handle(messagePacket*);
    void send_searchAccount(int account);
    void send_createGroup(groupInfo*);
    void send_addFriend(int account, char* message);
    void readyReadSlot();
    void addFriend_agree(addFriendInfoReq*);
    void addFriend_reject(addFriendInfoReq*);
    void on_privateChat_clicked();
    void on_groupChat_clicked();
    void on_sendData_clicked();
    void on_userInfo_button_clicked();
    //void on_pushButton_add_clicked();
    void on_pushButton_TODO_clicked();
    void add_action_triggered(QAction*);

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
    QMediaPlayer* player;
    QAudioOutput* output;
    todoDlg*    tododlg;
    QAction*    action_addFriend;
    QAction*    action_createGroup;
private:
    QWidget*            privateButtonWidget;
    QVBoxLayout*        privateButtonWidgetLayout;
    privateChatButton** friendButtonList;           //存放好友按钮索引
    privateChatPage**   privateChatPageList;        //存放好友聊天页面索引
    QVBoxLayout**       privateChatLayoutList;      //存放好友聊天页面layout索引
    chatTextEdit**         privateChatTextEditList;    //存放好友编辑框索引
    chatTextBrowser**      privateChatTextBrowserList; //存放好友浏览框索引
    std::vector<privateChatButton*> new_friend_button_list;
    std::vector<privateChatPage*>   new_friend_page_list;
    std::vector<QVBoxLayout*>       new_friend_chat_layout_list;
    std::vector<chatTextEdit*>      new_friend_edit_list;
    std::vector<chatTextBrowser*>   new_friend_browser_list;
private:
    QWidget*            groupButtonWidget;
    QVBoxLayout* groupButtonWidgetLayout;
    std::vector<groupChatButton*>   groupButtonList;        //存放群聊按钮索引
    std::vector<groupChatPage*>     groupChatPageList;      //存放群聊页面索引
    std::vector<QVBoxLayout*>       groupChatLayoutList;    //存放群聊页面layout索引
    std::vector<chatTextEdit*>      groupChatTextEditList;  //存放群聊编辑框索引
    std::vector<chatTextBrowser*>   groupChatTextBrowserList;   //存放群聊浏览框索引
};
#endif // WIDGET_H
