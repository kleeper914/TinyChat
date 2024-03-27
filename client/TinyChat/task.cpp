#include "task.h"

Task::Task(QTcpSocket* socket)
{
    m_socket = socket;
    m_type = RECV_HEAD;
    m_head = NULL;
    m_body = NULL;
    m_bufLen = 0;
    m_readNow = 0;
    is_finish = false;
}

Task::~Task()
{
    if (m_head != NULL)
        delete[] m_head;
    if (m_body != NULL)
        delete[] m_body;
    if (m_socket != NULL)
    {
        m_socket->close();
        m_socket = NULL;
    }
}

void Task::startEvent()
{
    if(readEvent() != READ_EXIT)
    {
        LOGINFO() << "readEvent() 出现错误";
    }
    emit finish_emit();
}

int Task::readEvent()
{
    int returnVal = 0;
    switch (m_type)
    {
    case RECV_HEAD:
        //LOGINFO() << "进入readHead()";
        returnVal = readHead();
        break;
    case RECV_BODY:
        //LOGINFO() << "进入readBody()";
        returnVal = readBody();
        break;
    default:
        break;
    }
    //printf("return value : %d\n", returnVal);
    //如果返回值是READ_AGAIN，递归调用readEvent()
    if (returnVal == READ_AGAIN){
        //LOGINFO() << "递归调用readEvent()";
        return readEvent();
    }
    return returnVal;
}

int Task::readHead()
{
    //初始化m_head
    if (m_head == NULL)
    {
        m_head = new char[sizeof(messageHead)];
        assert(m_head != NULL);
        m_bufLen = sizeof(messageHead);
        m_readNow = 0;
    }

    //可能一次无法读取完所有包头数据
    int readLen = m_socket->read(m_head + m_readNow, m_bufLen - m_readNow);
    if (readLen < 0)
    {
        return READ_ERROR;
    }
    if (readLen == 0)
    {
        return READ_EXIT;
    }
    m_readNow += readLen;

    //只有在读取完所有包头数据后才执行
    if (m_readNow == m_bufLen)
    {
        messageHead* tmpHead = (messageHead*)m_head;
        //如果标识符不是ROY
        if (strncmp(tmpHead->mark, "ROY", 3) != 0)
        {
            m_bufLen = 0;
            m_readNow = 0;
            m_type = RECV_HEAD;
            return READ_AGAIN;
        }
        else
        {
            //标识符是ROY，则继续读取包体
            //LOGINFO() << "读取包头完毕";
            m_type = RECV_BODY;
            int bufLen = ((messageHead*)m_head)->len;
            m_body = new char[bufLen];
            assert(m_body != NULL);
            m_bufLen = bufLen;
            m_readNow = 0;
            return READ_AGAIN;
        }
    }
    return 0;
}

int Task::readBody()
{
    //是否读取包头中规定的长度
    //可能出现多次读取包体数据，则最后一次读取m_readNow等于m_bufLen

    //LOGINFO() << "正在执行readBody";
    //LOGINFO() << "预设读取长度 : " << m_bufLen;
    //LOGINFO() << "readNow : " << m_readNow;
    if (m_readNow == m_bufLen)
    {
        //LOGINFO() << "如果已经读取到预设长度";
        m_type = RECV_HEAD;
        messageBasicHandle();
        is_finish = true;
        return READ_AGAIN;
    }
    else
    {
        int readLen = m_socket->read(m_body + m_readNow, m_bufLen - m_readNow);
        //LOGINFO() << "readBody() 读取到的长度: " << readLen;
        if (readLen < 0)
        {
            return READ_ERROR;
        }

        m_readNow += readLen;
        //LOGINFO() << "readNow : " << m_readNow;

        if (m_readNow == m_bufLen)
        {
            m_type = RECV_HEAD;
            //LOGINFO() << "读取包体完毕";
            messageBasicHandle();
            m_bufLen = 0;
            is_finish = true;
            //LOGINFO() << "task结束";
            return READ_AGAIN;
        }
    }
    return READ_OK;
}

int Task::messageBasicHandle()
{
    messagePacket* msgPacket = new messagePacket();
    msgPacket->head = m_head;
    msgPacket->body = m_body;
    msgPacket->bodyLen = m_bufLen;

    //LOGINFO() << "发送数据包";
    emit signals_emit(msgPacket); //发送数据包

    m_head = NULL;
    m_body = NULL;
    return READ_OK;
}

void Task::closeTask()
{

}
