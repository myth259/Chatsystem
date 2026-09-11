#include "UserManager.h"
#include <iostream>

UserManager::UserManager(MYSQL* db)
{
    connection = db;
}

bool UserManager::registerUser(
    const std::string& username,
    const std::string& password
) {
    // 1. 查询用户名是否已经存在
    std::string sql =
        "SELECT id FROM users WHERE username = '"
        + username + "'";

    if (mysql_query(connection, sql.c_str()) != 0) {

        std::cout << "Register query failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    // 2. 获取查询结果
    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {

        std::cout << "Get query result failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    // 3. 尝试取出一行
    MYSQL_ROW row = mysql_fetch_row(result);

    if (row != nullptr) {

        // 用户名已经存在
        mysql_free_result(result);

        std::cout << "Register failed: username already exists!"
                  << std::endl;

        return false;
    }

    // 4. 用户名不存在，可以注册
    mysql_free_result(result);

    std::string insertSql =
        "INSERT INTO users (username, password) VALUES ('"
        + username + "', '"
        + password + "')";

    if (mysql_query(connection, insertSql.c_str()) != 0) {

        std::cout << "Register failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    std::cout << "Register success: "
              << username
              << std::endl;

    return true;
}

bool UserManager::loginUser(
    const std::string& username,
    const std::string& password
) {
    std::cout << "Entering loginUser: "
          << username
          << std::endl;
          
    std::string sql =
        "SELECT password FROM users WHERE username = '"
        + username + "'";

    if (mysql_query(connection, sql.c_str()) != 0) {

        std::cout << "Login query failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {

        std::cout << "Get query result failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row == nullptr) {

        mysql_free_result(result);

        std::cout << "Login failed: user not found!"
                  << std::endl;

        return false;
    }

    bool success = (password == row[0]);

    mysql_free_result(result);

    if (!success) {
        std::cout << "Login failed: wrong password!"
                  << std::endl;
        return false;
    }

    std::cout << "Login success: "
              << username
              << std::endl;

    return true;
}

std::vector<std::string> UserManager::searchUser(
    const std::string& keyword
)
{
    std::vector<std::string> users;

    std::string sql =
        "SELECT username "
        "FROM users "
        "WHERE username LIKE '%" + keyword + "%'";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Search user failed: "
                  << mysql_error(connection)
                  << std::endl;

        return users;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cout << "Get search result failed: "
                  << mysql_error(connection)
                  << std::endl;

        return users;
    }

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != nullptr) {
        users.push_back(row[0]);
    }

    mysql_free_result(result);

    return users;
}

