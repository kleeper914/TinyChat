#ifndef CHATTEXTBROWSER_H
#define CHATTEXTBROWSER_H

#include <QTextBrowser>
#include "basic.h"
#include "msgDef.h"

class chatTextBrowser : public QTextBrowser
{
public:
    chatTextBrowser(friendInfo*);
    chatTextBrowser(groupInfo*);
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

#endif // CHATTEXTBROWSER_H
