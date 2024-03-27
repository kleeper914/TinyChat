#include "../include/mysqlHandle.h"

myDB::myDB()
{
    num_row = 0;
    num_field = 0;
    
    mysql_init(&mysqlCon);
    //cout << "init success" << endl;
}

myDB::~myDB()
{
    if(&mysqlCon != NULL)
    {
        mysql_close(&mysqlCon);
    }
}

bool myDB::myDB_init()
{
    if(mysql_real_connect(&mysqlCon, "localhost", "root", "ly20040822LUOYI..", "TinyChat", 3306, NULL, 0) == NULL)
    {
        cout << "connect error" << endl;
        exit(1);
    }
    //cout << "connect success" << endl;
    return true;
}

int myDB::getNumRow()
{
    return num_row;
}

int myDB::getNumField()
{
    return num_field;
}

void myDB::cleanResult()
{
    for(int i = 0; i < num_row; i++)
    {
        delete[] result[i];
    }
    delete[] result;
}

string** myDB::getResult()
{
    return result;
}

bool myDB::myDB_query(char* query)
{
    if(mysql_query(&mysqlCon, query))
    {
        cout << "Error: " << mysql_error(&mysqlCon);
        return false;
    }
    else
    {
        res = mysql_store_result(&mysqlCon);
        if(res)
        {
            int num_fields = mysql_num_fields(res);
            int num_rows = mysql_num_rows(res);
            //cout << "num_rows:" << num_rows << "\tnum_fields:" << num_fields << endl;

            num_row = num_rows;
            num_field = num_fields;
            if(num_rows) result = new string*[num_rows];
            else result == NULL;

            for(int i = 0; i < num_rows; i++)
            {
                row = mysql_fetch_row(res);
                result[i] = new string[num_fields];
                for(int j = 0; j < num_fields; j++)
                {
                    result[i][j] = row[j] ? row[j] : "";
                    //cout << result[i][j] << endl;
                }
            }
        }
    }
    return true;
}

bool myDB::myDB_exe(char* sql)
{
    if(mysql_query(&mysqlCon, sql))
    {
        cout << "Error: " << mysql_error(&mysqlCon);
        return false;
    }
    else
    {
        return true;
    }
}