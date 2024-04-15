#ifndef PRIVATECHATBUTTON_H
#define PRIVATECHATBUTTON_H

#include <QPushButton>
#include <QPainter>
#include <QRect>
#include "basic.h"
#include "msgDef.h"

#define LABEL_WIDTH 20
#define BORDER_WIDTH 5

class privateChatButton : public QPushButton
{
public:
    privateChatButton(friendInfo*);
    void paintEvent(QPaintEvent* event);
public:
    int getFriendAccount();
    char* getFriendName();
    bool getFriendStatus();
    bool getState();
    int get_num_not_read();
    void num_not_read_plus();       //使消息未读数加一
    void num_not_read_zero();       //小心未读数清零
    void set_not_disturb();         //设置消息免打扰
    void unset_not_disturb();       //取消小心免打扰
private:
    friendInfo* friInfo;
    bool is_not_disturb;    //用户是否设置免打扰
    int num_not_read;       //消息未读数
    QRect* rt1;
};

#endif // PRIVATECHATBUTTON_H
