#include "FriendManager.h"

#include <iostream>

FriendManager::FriendManager(MYSQL* db)
{
    connection = db;
}

bool FriendManager::isFriend(
    const std::string& username1,
    const std::string& username2
)
{
    std::string sql =
        "SELECT COUNT(*) "
        "FROM friendships "
        "WHERE user_id = "
        "(SELECT id FROM users WHERE username = '" + username1 + "') "
        "AND friend_id = "
        "(SELECT id FROM users WHERE username = '" + username2 + "')";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Check friendship failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cout << "Get friendship result failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row == nullptr) {
        mysql_free_result(result);
        return false;
    }

    int count = std::stoi(row[0]);

    mysql_free_result(result);

    return count > 0;
}

bool FriendManager::addFriend(
    const std::string& username,
    const std::string& friendUsername
)
{
    if (username == friendUsername) {
        return false;
    }

    if (isFriend(username, friendUsername)) {
        return false;
    }

    std::string sql =
        "INSERT INTO friendships (user_id, friend_id) "
        "VALUES ("
        "(SELECT id FROM users WHERE username = '" + username + "'), "
        "(SELECT id FROM users WHERE username = '" + friendUsername + "')"
        ")";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Add friend failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    sql =
        "INSERT INTO friendships (user_id, friend_id) "
        "VALUES ("
        "(SELECT id FROM users WHERE username = '" + friendUsername + "'), "
        "(SELECT id FROM users WHERE username = '" + username + "')"
        ")";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Add reverse friendship failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    std::cout << "Add friend success: "
              << username
              << " <-> "
              << friendUsername
              << std::endl;

    return true;
}

std::vector<std::string> FriendManager::getFriends(
    const std::string& username
)
{
    std::vector<std::string> friends;

    std::string sql =
        "SELECT u2.username "
        "FROM friendships f "
        "JOIN users u1 ON f.user_id = u1.id "
        "JOIN users u2 ON f.friend_id = u2.id "
        "WHERE u1.username = '" + username + "'";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Get friends failed: "
                  << mysql_error(connection)
                  << std::endl;

        return friends;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cout << "Get friends result failed: "
                  << mysql_error(connection)
                  << std::endl;

        return friends;
    }

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != nullptr) {
        friends.push_back(row[0]);
    }

    mysql_free_result(result);

    return friends;
}

