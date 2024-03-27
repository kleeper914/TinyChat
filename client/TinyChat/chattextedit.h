#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <QTextEdit>
#include "basic.h"
#include "msgDef.h"

class chatTextEdit : public QTextEdit
{
public:
    chatTextEdit(friendInfo*);
public:
    int getAccount();
    char* getName();
private:
    friendInfo* friInfo;
};

#endif // CHATTEXTEDIT_H
