#include "Database.h"
#include <iostream>

Database::Database()
{
    connection = nullptr;
}

bool Database::connect()
{
    connection = mysql_init(nullptr);

    if (connection == nullptr) {
        std::cout << "MySQL init failed!" << std::endl;
        return false;
    }

    if (mysql_real_connect(
        connection,
        "127.0.0.1",
        "root",
        "Zys0328.",
        "chat_system",
        3306,
        nullptr,
        0
    ) == nullptr) {

        std::cout << "MySQL connect failed: "
                  << mysql_error(connection)
                  << std::endl;

        mysql_close(connection);
        connection = nullptr;

        return false;
    }

    std::cout << "MySQL connected successfully!" << std::endl;

    return true;
}

MYSQL* Database::getConnection()
{
    return connection;
}

void Database::close()
{
    if (connection != nullptr) {
        mysql_close(connection);
        connection = nullptr;
    }
}