#ifndef TASK_H
#define TASK_H

#include <QObject>
#include "basic.h"
#include "msgDef.h"

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QTcpSocket*);
    ~Task();

public slots:
    void startEvent();

public:
    int readEvent();
    int readHead();
    int readBody();
    int messageBasicHandle();

    //qt的信号函数
signals:
    void signals_emit(messagePacket*);
    void finish_emit();
private:
    void closeTask();
private:
    QTcpSocket* m_socket;
    recvType	m_type;
    char* m_head;
    char* m_body;
    int m_bufLen;
    int m_readNow;
    bool is_finish;
};

#endif // TASK_H
