#ifndef REGISTERDLG_H
#define REGISTERDLG_H

#include <QDialog>
#include "basic.h"
#include "msgDef.h"
#include <QMessageBox>

namespace Ui {
class registerDlg;
}

class registerDlg : public QDialog
{
    Q_OBJECT

public:
    explicit registerDlg(QWidget *parent = nullptr);
    ~registerDlg();
private slots:
    void on_pushButton_clicked();
    void readyReadSlot();
private:
    void init();
    void sendMsg(void* buf, int bufLen, int command, int error = 0, int type = 0);
public:
    bool getStatus();
    userInfo getUserInfo();
private:
    Ui::registerDlg *ui;
    QTcpSocket*     m_socket;
    bool m_status;
    userInfo    m_userInfo;
};

#endif // REGISTERDLG_H
