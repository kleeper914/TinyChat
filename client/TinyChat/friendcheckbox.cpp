#include "friendcheckbox.h"

friendCheckBox::friendCheckBox() {}

friendCheckBox::friendCheckBox(friendInfo* friInfo)
{
    m_friendInfo = friInfo;
}

int friendCheckBox::getFriendAccount()
{
    return m_friendInfo->m_account;
}

QString friendCheckBox::getFriendName()
{
    return QString(m_friendInfo->name);
}
