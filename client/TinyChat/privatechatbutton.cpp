#include "privatechatbutton.h"

privateChatButton::privateChatButton(friendInfo* info)
{
    friInfo = info;
    this->setFixedHeight(50);
}

int privateChatButton::getFriendAccount()
{
    return friInfo->m_account;
}

char* privateChatButton::getFriendName()
{
    return friInfo->name;
}

bool privateChatButton::getFriendStatus()
{
    return friInfo->status;
}
