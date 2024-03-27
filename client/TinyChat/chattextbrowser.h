#ifndef CHATTEXTBROWSER_H
#define CHATTEXTBROWSER_H

#include <QTextBrowser>
#include "basic.h"
#include "msgDef.h"

class chatTextBrowser : public QTextBrowser
{
public:
    chatTextBrowser(friendInfo*);
public:
    int getAccount();
    char* getName();
private:
    friendInfo* friInfo;
};

#endif // CHATTEXTBROWSER_H
