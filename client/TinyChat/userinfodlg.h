#ifndef USERINFODLG_H
#define USERINFODLG_H

#include <QDialog>
#include "basic.h"
#include "msgDef.h"

namespace Ui {
class userInfoDlg;
}

class userInfoDlg : public QDialog
{
    Q_OBJECT

public:
    explicit userInfoDlg(QWidget *parent = nullptr);
    userInfoDlg(userInfo*);
    ~userInfoDlg();
private slots:
    void on_pushButton_password_clicked();

private:
    void init();
private:
    Ui::userInfoDlg *ui;
    userInfo* uInfo;
};

#endif // USERINFODLG_H
