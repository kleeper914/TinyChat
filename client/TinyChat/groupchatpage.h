#ifndef GROUPCHATPAGE_H
#define GROUPCHATPAGE_H

#include <QWidget>
#include "basic.h"
#include "msgDef.h"

class groupChatPage : public QWidget
{
    Q_OBJECT
public:
    explicit groupChatPage(QWidget *parent = nullptr);
    groupChatPage(groupInfo*);

signals:

private:
    groupInfo* gInfo;
};

#endif // GROUPCHATPAGE_H
