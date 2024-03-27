#ifndef _MYSQLHANDLE_H_
#define _MYSQLHANDLE_H_

#include "basic.h"
#include <mysql/mysql.h>

class myDB
{
public:
    myDB();
    ~myDB();
    bool myDB_init();
    bool myDB_exe(char* sql);                       //execute insert, update, delete statement, no return
    bool myDB_query(char* sql);                     //execute query statement, return two-dimensional array
    int getNumRow();
    int getNumField();
    string** getResult();
    void cleanResult();
private:
    MYSQL mysqlCon;
    MYSQL_RES* res;
    MYSQL_ROW row;
    int num_row;
    int num_field;
    string** result;
};

#endif