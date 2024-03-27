#include "chattextbrowser.h"

chatTextBrowser::chatTextBrowser(friendInfo* info)
{
    friInfo = info;
}

int chatTextBrowser::getAccount()
{
    return friInfo->m_account;
}

char* chatTextBrowser::getName()
{
    return friInfo->name;
}
