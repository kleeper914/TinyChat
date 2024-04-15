#include "privatechatbutton.h"

privateChatButton::privateChatButton(friendInfo* info)
{
    friInfo = info;
    this->setFixedHeight(50);
    num_not_read = 0;
}

int privateChatButton::getFriendAccount()
{
    return friInfo->m_account;
}

char* privateChatButton::getFriendName()
{
    return friInfo->name;
}

bool privateChatButton::getFriendStatus()
{
    return friInfo->status;
}

bool privateChatButton::getState()
{
    return is_not_disturb;
}

int privateChatButton::get_num_not_read()
{
    return num_not_read;
}

void privateChatButton::num_not_read_plus()
{
    num_not_read += 1;
    QPainter painter(this);
    painter.setPen(Qt::white);
    painter.drawText(*rt1, Qt::AlignCenter, QString::number(num_not_read));
}

void privateChatButton::num_not_read_zero()
{
    num_not_read = 0;
    QPainter painter(this);
    painter.setPen(Qt::white);
    painter.drawText(*rt1, Qt::AlignCenter, QString::number(num_not_read));
}

void privateChatButton::set_not_disturb()
{
    QPainter painter(this);
    painter.setPen(Qt::gray);
    painter.setBrush(QBrush(Qt::gray));
    painter.drawEllipse(*rt1);
}

void privateChatButton::unset_not_disturb()
{
    QPainter painter(this);
    painter.setPen(Qt::red);
    painter.setBrush(QBrush(Qt::red));
    painter.drawEllipse(*rt1);
}

void privateChatButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QRect rt = rect();
    rt1 = new QRect(rt.right()-LABEL_WIDTH, rt.top(), LABEL_WIDTH, LABEL_WIDTH);      //消息未读数
    QRect rt2 = QRect(rt.left()+BORDER_WIDTH, rt.top()+BORDER_WIDTH, rt.width()-2*BORDER_WIDTH, rt.height()-2*BORDER_WIDTH);    //按钮文本
    painter.fillRect(rt2, Qt::white);

    painter.setPen(Qt::red);
    painter.setBrush(QBrush(Qt::red));
    painter.drawEllipse(*rt1);

    painter.setPen(Qt::white);
    painter.drawText(*rt1, Qt::AlignCenter, QString::number(num_not_read));
    painter.setPen(Qt::black);
    painter.drawText(rt, Qt::AlignCenter, text());
}
