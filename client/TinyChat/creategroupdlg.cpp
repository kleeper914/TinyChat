#include "creategroupdlg.h"
#include "ui_creategroupdlg.h"
#include <QMessageBox>

createGroupDlg::createGroupDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::createGroupDlg)
{
    ui->setupUi(this);

    layout_add = new QVBoxLayout;
    ui->widget_friend_info->setLayout(layout_add);
    layout_add->setAlignment(Qt::AlignTop);

    layout_selected = new QVBoxLayout;
    ui->widget_selected_friend->setLayout(layout_selected);
    layout_selected->setAlignment(Qt::AlignTop);

    selected_list = new QListWidget;
    layout_selected->addWidget(selected_list);
}

createGroupDlg::~createGroupDlg()
{
    delete ui;
}

void createGroupDlg::init_friend_info(friendInfoMap* friMap)
{
    m_friendInfoMap = friMap;
    //todo
    //遍历friendInfoMap，并动态创建checkbox显示好友信息
    friendInfoMap::iterator ite = m_friendInfoMap->begin();
    for(int i = 0; i < m_friendInfoMap->size(); i++, ite++)
    {
        QString str = QString(ite->second->name) + "(" + QString::number(ite->first) + ")";
        friendCheckBox* friend_button = new friendCheckBox(ite->second);
        friend_button->setText(QString(ite->second->name)+"("+QString::number(ite->first)+")");
        check_list.push_back(friend_button);
        connect(check_list[i], &QCheckBox::stateChanged, [this, i](){
            int account = check_list[i]->getFriendAccount();
            QString name = check_list[i]->getFriendName();
            if(check_list[i]->isChecked())
            {
                QString str = name+"("+QString::number(account)+")";
                QListWidgetItem* item = new QListWidgetItem(str, selected_list);
                m_selected_map.insert(std::pair(account, item));
                selected_list->addItem(item);
            }
            else
            {
                selected_map::iterator ite = m_selected_map.find(account);
                if(ite != m_selected_map.end())
                {
                    int row = selected_list->row(ite->second);
                    delete selected_list->takeItem(row);
                    m_selected_map.erase(ite);
                }
            }
        });
        layout_add->addWidget(friend_button);
    }
}

void createGroupDlg::on_button_create_group_clicked()
{
    if(ui->lineEdit_group_name->text().isEmpty())
    {
        QMessageBox::critical(NULL, "ERROR", "请输入群聊名称");
    }
    else
    {
        QMessageBox::StandardButton messagebox = QMessageBox::question(this, "提示", "是否创建一个新的群组", QMessageBox::Yes | QMessageBox::No);
        if(messagebox == QMessageBox::Yes)
        {
        groupInfo* create_group = new groupInfo;
        create_group->account = 0;      //代表还未创建成功
        memcpy(create_group->name, ui->lineEdit_group_name->text().toStdString().c_str(), ui->lineEdit_group_name->text().toStdString().size());
        create_group->size = m_selected_map.size() + 1;     //被选中的好友数量加上用户本人
        selected_map::iterator ite = m_selected_map.begin();
        for(int i = 0; i < m_selected_map.size(); i++, ite++)
        {
            friendInfoMap::iterator ite_friend = m_friendInfoMap->find(ite->first);
            groupMemInfo* create_group_mem = new groupMemInfo;
            create_group_mem->account = ite_friend->first;
            memcpy(create_group_mem->name, ite_friend->second->name, sizeof(ite_friend->second->name));
            create_group_mem->right = 0;
            create_group->groupMemInfoList.push_back(create_group_mem);
            //LOGINFO() << "member" << i+1 << ": account: " << create_group_mem->account << " name: " << create_group_mem->name << " right: " << create_group_mem->right;
        }
        emit emit_group_mem_info(create_group);
        }
        else
        {
            return;
        }
    }
}

