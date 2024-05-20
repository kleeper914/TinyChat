#ifndef CREATEGROUPDLG_H
#define CREATEGROUPDLG_H

#include <QDialog>
#include "basic.h"
#include "msgDef.h"
#include <QVBoxLayout>
#include <QCheckBox>
#include <QListWidget>
#include <QListWidgetItem>
#include "friendcheckbox.h"

typedef std::map<int, QListWidgetItem*> selected_map;

namespace Ui {
class createGroupDlg;
}

class createGroupDlg : public QDialog
{
    Q_OBJECT

public:
    explicit createGroupDlg(QWidget *parent = nullptr);
    ~createGroupDlg();
    void init_friend_info(friendInfoMap*);
signals:
    void emit_group_mem_info(groupInfo*);
private slots:
    void on_button_create_group_clicked();

private:
    Ui::createGroupDlg *ui;
    friendInfoMap* m_friendInfoMap;
private:
    QVBoxLayout* layout_add;
    QVBoxLayout* layout_selected;
    std::vector<friendCheckBox*> check_list;
    QListWidget* selected_list;
    selected_map m_selected_map;
};

#endif // CREATEGROUPDLG_H
