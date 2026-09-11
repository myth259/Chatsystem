#pragma once

#include <winsock2.h>
#include <string>

#include "Database.h"
#include "UserManager.h"
#include "FriendManager.h"
#include "ClientManager.h"
#include "MessageRouter.h"
#include "MessagePermission.h"
#include "GroupManager.h"

class Server
{
private:
    SOCKET serverSocket;

    Database database;

    MYSQL* mysqlConnection;

    UserManager* userManager;
    FriendManager* friendManager;
    ClientManager* clientManager;
    MessageRouter* messageRouter;
    MessagePermission* messagePermission;
    GroupManager* groupManager;

    void addOnlineUser(
        const std::string& username,
        SOCKET clientSocket
    );

    void handleMessage(
        const std::string& username,
        const std::string& packet,
        SOCKET clientSocket
    );

    bool handleAuthentication(
        const std::string& type,
        const std::string& username,
        const std::string& password,
        SOCKET clientSocket
    );

    void handleClient(
        SOCKET clientSocket
    );

    // 处理添加好友请求
    bool handleAddFriend(
        const std::string& username,
        const std::string& friendUsername,
        SOCKET clientSocket
    );

    void handleGetFriends(
        const std::string& username,
        SOCKET clientSocket
    );

    void handleSearchUser(
        const std::string& keyword,
        SOCKET clientSocket
    );
    // 创建群聊
    void handleCreateGroup(
        const std::string& username,
        const std::string& groupName,
        SOCKET clientSocket
    );

    // 添加群成员
    void handleAddGroupMember(
        const std::string& username,
        int groupId,
        const std::string& memberUsername,
        SOCKET clientSocket
    );

    // 获取群成员
    void handleGetGroupMembers(
        int groupId,
        SOCKET clientSocket
    );

    // 获取当前用户加入的所有群聊
    void handleGetGroups(
        const std::string& username,
        SOCKET clientSocket
    );

    void sendMessage(
        SOCKET clientSocket,
        const std::string& message
    );

public:
    Server();

    ~Server();

    bool start();
};