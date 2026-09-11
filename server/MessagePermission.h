#pragma once

#include <mysql.h>
#include <string>
#include "FriendManager.h"

class MessagePermission
{
private:
    MYSQL* connection;
    FriendManager* friendManager;

public:
    MessagePermission(
        MYSQL* db,
        FriendManager* friendManager
    );

    bool canSendMessage(
        const std::string& sender,
        const std::string& receiver
    );

    void increaseStrangerMessageCount(
        const std::string& sender,
        const std::string& receiver
    );
};