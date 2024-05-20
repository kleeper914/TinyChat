#ifndef BASIC_H
#define BASIC_H

#include <qdebug.h>
#include <qdatetime.h>
#include <QThread>
//#include <WinSock2.h>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include "json/json.h"
#include <QTcpSocket>
#include <QDateTime>

#define SERVER_IP	"114.116.255.15"
#define SERVER_PORT	 9190

#define LOGINFO() qDebug().noquote()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss zzz")\
<<"["<<__FILE__<<":"<<__LINE__<<"]["<<__FUNCTION__<<"]"

int getSequence();

#endif // BASIC_H
