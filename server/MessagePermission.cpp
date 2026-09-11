#include "MessagePermission.h"
#include <iostream>

MessagePermission::MessagePermission(
    MYSQL* db,
    FriendManager* friendManager
) : connection(db), friendManager(friendManager) {}

bool MessagePermission::canSendMessage(
    const std::string& sender,
    const std::string& receiver
) {
    // 好友可以正常发送
    if (friendManager->isFriend(sender, receiver)) {
        return true;
    }

    // 查询双方的用户 ID
    std::string sql =
        "SELECT "
        "(SELECT id FROM users WHERE username = '" + sender + "'), "
        "(SELECT id FROM users WHERE username = '" + receiver + "')";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "User ID query failed: "
                  << mysql_error(connection)
                  << std::endl;
        return false;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cout << "Get user ID result failed: "
                  << mysql_error(connection)
                  << std::endl;
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row == nullptr) {
        mysql_free_result(result);
        return false;
    }

    int senderId = std::stoi(row[0]);
    int receiverId = std::stoi(row[1]);

    mysql_free_result(result);

    // 查询陌生人消息次数
    sql =
        "SELECT message_count FROM stranger_messages "
        "WHERE sender_id = " + std::to_string(senderId) +
        " AND receiver_id = " + std::to_string(receiverId);

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Stranger message query failed: "
                  << mysql_error(connection)
                  << std::endl;
        return false;
    }

    result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cout << "Get stranger message result failed: "
                  << mysql_error(connection)
                  << std::endl;
        return false;
    }

    row = mysql_fetch_row(result);

    // 还没有发送过陌生人消息
    if (row == nullptr) {
        mysql_free_result(result);
        return true;
    }

    int messageCount = std::stoi(row[0]);

    mysql_free_result(result);

    return messageCount < 3;
}

void MessagePermission::increaseStrangerMessageCount(
    const std::string& sender,
    const std::string& receiver
) {
    std::string sql =
        "INSERT INTO stranger_messages "
        "(sender_id, receiver_id, message_count) "
        "VALUES ("
        "(SELECT id FROM users WHERE username = '" + sender + "'), "
        "(SELECT id FROM users WHERE username = '" + receiver + "'), "
        "1) "
        "ON DUPLICATE KEY UPDATE "
        "message_count = message_count + 1";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Increase stranger message count failed: "
                  << mysql_error(connection)
                  << std::endl;
    }
}