#include "GroupManager.h"

#include <iostream>

// 保存数据库连接
GroupManager::GroupManager(MYSQL* db)
{
    connection = db;
}

// 创建群聊
int GroupManager::createGroup(
    const std::string& groupName,
    const std::string& owner
)
{
    // 先找到群主对应的用户 ID
    std::string sql =
        "SELECT id FROM users "
        "WHERE username = '" + owner + "'";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Get owner ID failed: "
                  << mysql_error(connection)
                  << std::endl;

        return -1;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cout << "Get owner result failed: "
                  << mysql_error(connection)
                  << std::endl;

        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row == nullptr) {
        mysql_free_result(result);
        return -1;
    }

    int ownerId = std::stoi(row[0]);

    mysql_free_result(result);

    // 创建群
    sql =
        "INSERT INTO chat_groups "
        "(group_name, owner_id) "
        "VALUES ('" + groupName + "', " +
        std::to_string(ownerId) + ")";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Create group failed: "
                  << mysql_error(connection)
                  << std::endl;

        return -1;
    }

    // 获取刚刚创建的群 ID
    int groupId =
        static_cast<int>(mysql_insert_id(connection));

    // 创建群后，群主自动加入群
    sql =
        "INSERT INTO group_members "
        "(group_id, user_id) "
        "VALUES (" +
        std::to_string(groupId) + ", " +
        std::to_string(ownerId) + ")";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Add owner to group failed: "
                  << mysql_error(connection)
                  << std::endl;

        return -1;
    }

    std::cout << "Create group success: "
              << groupName
              << ", group ID = "
              << groupId
              << std::endl;

    return groupId;
}

// 获取群成员用户名
std::vector<std::string> GroupManager::getGroupMembers(
    int groupId
)
{
    std::vector<std::string> members;

    std::string sql =
        "SELECT u.username "
        "FROM group_members gm "
        "JOIN users u ON gm.user_id = u.id "
        "WHERE gm.group_id = " +
        std::to_string(groupId);

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Get group members failed: "
                  << mysql_error(connection)
                  << std::endl;

        return members;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cout << "Get group members result failed: "
                  << mysql_error(connection)
                  << std::endl;

        return members;
    }

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != nullptr) {
        members.push_back(row[0]);
    }

    mysql_free_result(result);

    return members;
}

// 添加群成员
bool GroupManager::addMember(
    int groupId,
    const std::string& username
)
{
    // 查询用户 ID
    std::string sql =
        "SELECT id FROM users "
        "WHERE username = '" + username + "'";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Get member ID failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cout << "Get member result failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    // 用户不存在
    if (row == nullptr) {
        mysql_free_result(result);
        return false;
    }

    int userId = std::stoi(row[0]);

    mysql_free_result(result);

    // 添加群成员
    sql =
        "INSERT INTO group_members "
        "(group_id, user_id) "
        "VALUES (" +
        std::to_string(groupId) + ", " +
        std::to_string(userId) + ")";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Add group member failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    std::cout << "Add group member success: "
              << username
              << " -> group "
              << groupId
              << std::endl;

    return true;
}

// 判断用户是否属于群聊
bool GroupManager::isMember(
    int groupId,
    const std::string& username
)
{
    std::string sql =
        "SELECT COUNT(*) "
        "FROM group_members gm "
        "JOIN users u ON gm.user_id = u.id "
        "WHERE gm.group_id = " +
        std::to_string(groupId) +
        " AND u.username = '" +
        username + "'";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Check group member failed: "
                  << mysql_error(connection)
                  << std::endl;

        return false;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cout << "Get group member result failed: "
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

// 获取用户加入的所有群聊
std::vector<std::string> GroupManager::getGroups(
    const std::string& username
)
{
    std::vector<std::string> groups;

    std::string sql =
        "SELECT cg.group_name "
        "FROM group_members gm "
        "JOIN users u ON gm.user_id = u.id "
        "JOIN chat_groups cg ON gm.group_id = cg.id "
        "WHERE u.username = '" +
        username + "'";

    if (mysql_query(connection, sql.c_str()) != 0) {
        std::cout << "Get groups failed: "
                  << mysql_error(connection)
                  << std::endl;

        return groups;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cout << "Get groups result failed: "
                  << mysql_error(connection)
                  << std::endl;

        return groups;
    }

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != nullptr) {
        groups.push_back(row[0]);
    }

    mysql_free_result(result);

    return groups;
}