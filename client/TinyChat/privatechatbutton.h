#ifndef PRIVATECHATBUTTON_H
#define PRIVATECHATBUTTON_H

#include <QPushButton>
#include "basic.h"
#include "msgDef.h"

class privateChatButton : public QPushButton
{
public:
    privateChatButton(friendInfo*);
public:
    int getFriendAccount();
    char* getFriendName();
    bool getFriendStatus();
private:
    friendInfo* friInfo;
};

#endif // PRIVATECHATBUTTON_H
