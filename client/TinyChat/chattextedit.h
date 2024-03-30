#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <QTextEdit>
#include "basic.h"
#include "msgDef.h"

class chatTextEdit : public QTextEdit
{
public:
    chatTextEdit(friendInfo*);
    chatTextEdit(groupInfo*);
public:
    int getAccount();
    char* getName();
public:
    int getGroupAccount();
    char* getGroupName();
    int getGroupSize();
private:
    friendInfo* friInfo;
    groupInfo* gInfo;
};

#endif // CHATTEXTEDIT_H
