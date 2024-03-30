#ifndef GROUPCHATBUTTON_H
#define GROUPCHATBUTTON_H

#include <QPushButton>
#include "basic.h"
#include "msgDef.h"

class groupChatButton : public QPushButton
{
public:
    groupChatButton(groupInfo*);
public:
    int getGroupAccount();
    char* getGroupName();
    int getGroupSize();
private:
    groupInfo* gInfo;
};

#endif // GROUPCHATBUTTON_H
