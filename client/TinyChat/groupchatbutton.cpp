#include "groupchatbutton.h"

groupChatButton::groupChatButton(groupInfo* info)
{
    gInfo = info;
    this->setFixedHeight(50);
}

int groupChatButton::getGroupAccount()
{
    return gInfo->account;
}

char* groupChatButton::getGroupName()
{
    return gInfo->name;
}

int groupChatButton::getGroupSize()
{
    return gInfo->size;
}
