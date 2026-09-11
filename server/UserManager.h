#pragma once

#include <mysql.h>
#include <string>
#include <vector>

class UserManager
{
private:
    MYSQL* connection;

public:
    UserManager(MYSQL* db);

    bool registerUser(
        const std::string& username,
        const std::string& password
    );
    bool loginUser(
        const std::string& username,
        const std::string& password
    );
    std::vector<std::string> searchUser(
        const std::string& keyword
    );
};