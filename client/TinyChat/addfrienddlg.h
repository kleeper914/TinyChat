#ifndef ADDFRIENDDLG_H
#define ADDFRIENDDLG_H

#include <QDialog>
#include "basic.h"
#include "msgDef.h"
#include "task.h"

namespace Ui {
class addFriendDlg;
}

class addFriendDlg : public QDialog
{
    Q_OBJECT

public:
    explicit addFriendDlg(QWidget *parent = nullptr);
    ~addFriendDlg();
signals:
    void emit_searchButtonClicked(int account);
    void emit_addButtonClicked(int account, char* message);
private slots:
    void on_pushButton_searchFriend_clicked();
    void displaySearchAccount(searchAccountReply*);

    void on_pushButton_addFriend_clicked();

private:
    Ui::addFriendDlg *ui;
};

#endif // ADDFRIENDDLG_H
