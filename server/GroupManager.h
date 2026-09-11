#pragma once

#include <mysql.h>
#include <string>
#include <vector>

class GroupManager
{
private:
    MYSQL* connection;

public:
    // 保存数据库连接
    GroupManager(MYSQL* db);

    // 创建群聊
    // 返回创建成功后的群 ID
    int createGroup(
        const std::string& groupName,
        const std::string& owner
    );

    // 获取群成员用户名列表
    std::vector<std::string> getGroupMembers(
        int groupId
    );

    // 添加群成员
    bool addMember(
        int groupId,
        const std::string& username
    );

    // 判断用户是否属于群聊
    bool isMember(
        int groupId,
        const std::string& username
    );

    // 获取用户加入的所有群聊
    std::vector<std::string> getGroups(
        const std::string& username
    );
};