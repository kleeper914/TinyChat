#include "tododlg.h"
#include "ui_tododlg.h"

todoDlg::todoDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::todoDlg)
{
    ui->setupUi(this);
}

todoDlg::~todoDlg()
{
    delete ui;
}

void todoDlg::insert_addFriendReqSend(addFriendInfoReq* addFriReq)
{
    LOGINFO() << "发送好友请求，并添加到todoDlg中";
    QWidget* widget_Req = new QWidget(ui->widget_todo_addFriendReq_list);
    QHBoxLayout* layout_Req = new QHBoxLayout;
    widget_Req->setLayout(layout_Req);
    QLabel* label_Req = new QLabel;
    label_Req->setText("account: " + QString::number(addFriReq->receiverAccount));
    QLabel* label_state = new QLabel;
    label_state->setText("state: 未回复");
    m_Req_state_map.insert(std::pair(addFriReq->receiverAccount, label_state));

    //检查是否插入成功
    label_Req_state_map::iterator ite_req_state = m_Req_state_map.find(addFriReq->receiverAccount);
    if(ite_req_state != m_Req_state_map.end())
    {
        LOGINFO() << "插入Req_state_map成功";
    }

    layout_Req->addWidget(label_Req);
    layout_Req->addWidget(label_state);
}

void todoDlg::insert_addFriendReqRecv(addFriendInfoReq* addFriReq)
{
    LOGINFO() << "收到好友请求，并添加到todoDlg中";
    QWidget* widget_Req_Recv = new QWidget(ui->widget_todo_addFriendReply_list);
    QHBoxLayout* layout_Req_Recv = new QHBoxLayout;
    widget_Req_Recv->setLayout(layout_Req_Recv);
    QLabel* label_Req_Recv = new QLabel;
    label_Req_Recv->setText("account: " + QString::number(addFriReq->senderAccount));
    QLabel* label_Req_state = new QLabel;
    label_Req_state->setText("state: 未回复");
    m_Reply_state_map.insert(std::pair(addFriReq->senderAccount, label_Req_state));

    //检查是否插入成功
    label_Reply_state_map::iterator ite_reply_state = m_Reply_state_map.find(addFriReq->senderAccount);
    if(ite_reply_state != m_Reply_state_map.end())
    {
        LOGINFO() << "插入m_Reply_state_map成功";
    }

    QPushButton* button_Reply_agree = new QPushButton;
    QPushButton* button_Reply_reject = new QPushButton;
    button_Reply_agree->setText("同意");
    button_Reply_reject->setText("拒绝");
    connect(button_Reply_agree, &QPushButton::clicked, [this, addFriReq]{
        emit emit_addFriend_agree(addFriReq);
    });
    connect(button_Reply_reject, &QPushButton::clicked, [this, addFriReq]{
        emit emit_addFriend_reject(addFriReq);
    });
    layout_Req_Recv->addWidget(label_Req_Recv);
    layout_Req_Recv->addWidget(label_Req_state);
    layout_Req_Recv->setSpacing(60);
    layout_Req_Recv->addWidget(button_Reply_agree);
    layout_Req_Recv->addWidget(button_Reply_reject);
}

void todoDlg::finish_addFriendReqSend(int account, QString str)
{
    label_Req_state_map::iterator ite_req_state = m_Req_state_map.find(account);
    if(ite_req_state != m_Req_state_map.end())
    {
        ite_req_state->second->setText(str);
    }
    else
    {
        LOGINFO() << "未查找到已发送的好友请求";
    }
}

void todoDlg::finish_addFriendReqRecv(int account, QString str)
{
    label_Reply_state_map::iterator ite_reply_state = m_Reply_state_map.find(account);
    if(ite_reply_state != m_Reply_state_map.end())
    {
        ite_reply_state->second->setText(str);
    }
    else
    {
        LOGINFO() << "未查找到已接受的好友请求";
    }
}
