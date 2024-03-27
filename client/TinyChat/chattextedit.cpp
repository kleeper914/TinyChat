#include "chattextedit.h"

chatTextEdit::chatTextEdit(friendInfo* info)
{
    friInfo = info;
}

int chatTextEdit::getAccount()
{
    return friInfo->m_account;
}

char* chatTextEdit::getName()
{
    return friInfo->name;
}
