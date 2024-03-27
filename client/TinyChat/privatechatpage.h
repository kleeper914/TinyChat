#ifndef PRIVATECHATPAGE_H
#define PRIVATECHATPAGE_H

#include <QWidget>
#include "basic.h"
#include "msgDef.h"

class privateChatPage : public QWidget
{
    Q_OBJECT
public:
    explicit privateChatPage(QWidget *parent = nullptr);
    privateChatPage(friendInfo*);
signals:

private:
    friendInfo* friInfo;
};

#endif // PRIVATECHATPAGE_H
