#include "groupchatpage.h"

groupChatPage::groupChatPage(QWidget *parent)
    : QWidget{parent}
{}

groupChatPage::groupChatPage(groupInfo* info)
{
    gInfo = info;
}
