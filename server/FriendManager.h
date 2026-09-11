#pragma once

#include <mysql.h>
#include <string>
#include <vector>

class FriendManager
{
private:
    MYSQL* connection;

public:
    FriendManager(MYSQL* db);

    bool isFriend(
        const std::string& username1,
        const std::string& username2
    );

    bool addFriend(
        const std::string& username,
        const std::string& friendUsername
    );

    std::vector<std::string> getFriends(
        const std::string& username
    );
};