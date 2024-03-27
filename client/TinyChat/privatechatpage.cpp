#include "privatechatpage.h"

privateChatPage::privateChatPage(QWidget *parent)
    : QWidget{parent}
{}

privateChatPage::privateChatPage(friendInfo* Info)
{
    friInfo = Info;
}
