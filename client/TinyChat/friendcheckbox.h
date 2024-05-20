#ifndef FRIENDCHECKBOX_H
#define FRIENDCHECKBOX_H

#include <QCheckBox>
#include "basic.h"
#include "msgDef.h"

class friendCheckBox : public QCheckBox
{
public:
    friendCheckBox();
    friendCheckBox(friendInfo*);
    int getFriendAccount();
    QString getFriendName();
private:
    friendInfo* m_friendInfo;
};

#endif // FRIENDCHECKBOX_H
