#include "chattextedit.h"

chatTextEdit::chatTextEdit(friendInfo* info)
{
    friInfo = info;
}

chatTextEdit::chatTextEdit(groupInfo* info)
{
    gInfo = info;
}

int chatTextEdit::getAccount()
{
    return friInfo->m_account;
}

char* chatTextEdit::getName()
{
    return friInfo->name;
}

int chatTextEdit::getGroupAccount()
{
    return gInfo->account;
}

char* chatTextEdit::getGroupName()
{
    return gInfo->name;
}

int chatTextEdit::getGroupSize()
{
    return gInfo->size;
}
