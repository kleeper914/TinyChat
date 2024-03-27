#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>
#include "basic.h"
#include "msgDef.h"
#include "registerdlg.h"

namespace Ui {
class LoginDlg;
}

class LoginDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDlg(QWidget *parent = nullptr);
    ~LoginDlg();
private:
    void init();
public:
    userInfo* getUInfo()
    {
        return &m_uInfo;
    }
private slots:
    void on_pushButton_login_clicked();

    void on_pushButton_regist_clicked();

private:
    Ui::LoginDlg *ui;
    userInfo m_uInfo;
};

#endif // LOGINDLG_H
