#include "chattextbrowser.h"

chatTextBrowser::chatTextBrowser(friendInfo* info)
{
    friInfo = info;
}

chatTextBrowser::chatTextBrowser(groupInfo* info)
{
    gInfo = info;
}

int chatTextBrowser::getAccount()
{
    return friInfo->m_account;
}

char* chatTextBrowser::getName()
{
    return friInfo->name;
}

int chatTextBrowser::getGroupAccount()
{
    return gInfo->account;
}

char* chatTextBrowser::getGroupName()
{
    return gInfo->name;
}

int chatTextBrowser::getGroupSize()
{
    return gInfo->size;
}
