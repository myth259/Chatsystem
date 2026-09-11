#pragma once

#include <mysql.h>
#include <string>

class Database
{
private:
    MYSQL* connection;

public:
    Database();

    bool connect();

    MYSQL* getConnection();

    void close();
};