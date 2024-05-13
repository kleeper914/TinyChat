#ifndef TODODLG_H
#define TODODLG_H

#include <QDialog>
#include "msgDef.h"
#include "basic.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace Ui {
class todoDlg;
}

typedef std::map<int, QLabel*> label_Req_state_map;
typedef std::map<int, QLabel*> label_Reply_state_map;

class todoDlg : public QDialog
{
    Q_OBJECT

public:
    explicit todoDlg(QWidget *parent = nullptr);
    ~todoDlg();
    void insert_addFriendReqRecv(addFriendInfoReq*);
    void insert_addFriendReqSend(addFriendInfoReq*);
    void finish_addFriendReqSend(int, QString);
    void finish_addFriendReqRecv(int, QString);
signals:
    void emit_addFriend_agree(addFriendInfoReq*);
    void emit_addFriend_reject(addFriendInfoReq*);

private:
    Ui::todoDlg *ui;
    label_Req_state_map     m_Req_state_map;
    label_Reply_state_map   m_Reply_state_map;
};

#endif // TODODLG_H
