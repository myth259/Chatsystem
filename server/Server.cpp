#include "Server.h"

#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")


Server::Server()
{
    WSADATA wsaData;

    serverSocket = INVALID_SOCKET;

    mysqlConnection = nullptr;
    userManager = nullptr;
    friendManager = nullptr;
    clientManager = nullptr;
    messageRouter = nullptr;
    messagePermission = nullptr;
    groupManager = nullptr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed!" << std::endl;
        return;
    }

    if (!database.connect()) {
        std::cout << "Database connect failed!" << std::endl;
        return;
    }

    mysqlConnection = database.getConnection();

    userManager = new UserManager(mysqlConnection);

    friendManager = new FriendManager(mysqlConnection);

    clientManager = new ClientManager();

    messageRouter = new MessageRouter(clientManager);

    messagePermission = new MessagePermission(
        mysqlConnection,
        friendManager
    );

    groupManager = new GroupManager(mysqlConnection);

    serverSocket = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Socket create failed!" << std::endl;
        return;
    }

    std::cout << "Socket create success!" << std::endl;
}

Server::~Server()
{
    delete groupManager;
    delete messagePermission;
    delete messageRouter;
    delete clientManager;
    delete friendManager;
    delete userManager;

    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
    }

    WSACleanup();
}

void Server::sendMessage(
    SOCKET clientSocket,
    const std::string& message
)
{
    std::string packet = message + "\n";

    send(
        clientSocket,
        packet.c_str(),
        static_cast<int>(packet.length()),
        0
    );
}

void Server::addOnlineUser(
    const std::string& username,
    SOCKET clientSocket
)
{
    clientManager->addUserSocket(
        username,
        clientSocket
    );

    clientManager->addClient(
        clientSocket
    );

    std::cout << "Online clients: "
              << clientManager->getClientCount()
              << std::endl;
}

bool Server::handleAuthentication(
    const std::string& type,
    const std::string& username,
    const std::string& password,
    SOCKET clientSocket
)
{
    std::cout << "Entering handleAuthentication"
              << std::endl;

    // 注册
    if (type == "REGISTER") {

        if (userManager->registerUser(username, password)) {

            std::cout << "Register success: "
                      << username
                      << std::endl;

            // 注册成功后，直接把用户加入在线用户
            addOnlineUser(username, clientSocket);

            std::string response = "REGISTER_SUCCESS";

            sendMessage(
                clientSocket,
                response
            );

            return true;
        }
        else {

            std::string response = "REGISTER_FAILED";

            sendMessage(
                clientSocket,
                response
            );

            return false;
        }
    }

    // 登录
    if (type == "LOGIN") {

        std::cout << "Handling login"
                  << std::endl;

        if (userManager->loginUser(username, password)) {

            // 检查这个账号是否已经在线
            SOCKET oldSocket =
                clientManager->getUserSocket(username);

            // 如果已经在线，踢掉旧连接
            if (oldSocket != INVALID_SOCKET) {

                std::string kickMessage = "LOGIN_KICKED";

                sendMessage(
                    oldSocket,
                    kickMessage
                );

                closesocket(oldSocket);
            }

            std::cout << "Login success: "
                      << username
                      << std::endl;

            // 添加新的在线连接
            addOnlineUser(username, clientSocket);

            std::string response =
                "LOGIN_SUCCESS|" + username;

            sendMessage(
                clientSocket,
                response
            );

            return true;
        }
        else {

            std::string response = "LOGIN_FAILED";

            sendMessage(
                clientSocket,
                response
            );

            return false;
        }
    }

    // 既不是注册，也不是登录
    return false;
}

void Server::handleMessage(
    const std::string& username,
    const std::string& packet,
    SOCKET clientSocket
)
{
    // 先判断是不是带 | 的业务请求
    size_t typePos = packet.find('|');

    // 获取好友列表
    if (packet == "GET_FRIENDS") {

        handleGetFriends(
            username,
            clientSocket
        );

        return;
    }

    // 获取群聊列表
    if (packet == "GET_GROUPS") {

        handleGetGroups(
            username,
            clientSocket
        );

        return;
    }

    if (typePos != std::string::npos) {

        std::string type = packet.substr(0, typePos);

        // 添加好友
        if (type == "ADD_FRIEND") {

            std::string friendUsername =
                packet.substr(typePos + 1);

            handleAddFriend(
                username,
                friendUsername,
                clientSocket
            );

            return;
        }

        // 搜索用户
        if (type == "SEARCH_USER") {

            std::string keyword =
                packet.substr(typePos + 1);

            handleSearchUser(
                keyword,
                clientSocket
            );

            return;
        }

        // 创建群
        if (type == "CREATE_GROUP") {

            std::string groupName =
                packet.substr(typePos + 1);

            handleCreateGroup(
                username,
                groupName,
                clientSocket
            );

            return;
        }

        // 添加群成员
        if (type == "ADD_GROUP_MEMBER") {

            size_t pos2 =
                packet.find('|', typePos + 1);

            if (pos2 == std::string::npos) {
                return;
            }

            int groupId = std::stoi(
                packet.substr(
                    typePos + 1,
                    pos2 - typePos - 1
                )
            );

            std::string memberUsername =
                packet.substr(pos2 + 1);

            handleAddGroupMember(
                username,
                groupId,
                memberUsername,
                clientSocket
            );

            return;
        }

        // 获取群成员
        if (type == "GET_GROUP_MEMBERS") {

            int groupId = std::stoi(
                packet.substr(typePos + 1)
            );

            handleGetGroupMembers(
                groupId,
                clientSocket
            );

            return;
        }

        // 群聊消息
        if (type == "GROUP_MESSAGE") {

            size_t pos2 =
                packet.find('|', typePos + 1);

            if (pos2 == std::string::npos) {
                return;
            }

            int groupId = std::stoi(
                packet.substr(
                    typePos + 1,
                    pos2 - typePos - 1
                )
            );

            std::string message =
                packet.substr(pos2 + 1);

            // 判断发送者是不是群成员
            if (!groupManager->isMember(groupId, username)) {

                std::string response =
                    "GROUP_MESSAGE_FAILED";

                sendMessage(
                    clientSocket,
                    response
                );

                return;
            }

            std::vector<std::string> members =
                groupManager->getGroupMembers(groupId);

            messageRouter->sendGroupMessage(
                members,
                username + ": " + message
            );

            return;
        }
    }

    // 普通聊天消息
    size_t pos = packet.find(':');

    if (pos == std::string::npos) {
        return;
    }

    std::string target = packet.substr(0, pos);
    std::string message = packet.substr(pos + 1);

    std::cout << "Target: " << target << std::endl;
    std::cout << "Message: " << message << std::endl;

    if (messagePermission->canSendMessage(username, target)) {

        if (messageRouter->sendMessage(
            username,
            target,
            message
        )) {

            std::cout << "Message sent successfully."
                      << std::endl;

            if (!friendManager->isFriend(username, target)) {

                messagePermission->increaseStrangerMessageCount(
                    username,
                    target
                );
            }
        }
        else {

            std::cout << "Failed to send message."
                      << std::endl;
        }
    }
    else {

        std::cout << "Cannot send message: "
                  << username
                  << " -> "
                  << target
                  << std::endl;

        std::string response =
            "MESSAGE_LIMIT_REACHED";

        sendMessage(
            clientSocket,
            response
        );
    }
}

void Server::handleClient(SOCKET clientSocket)
{
    char buffer[1024];

    std::string receiveBuffer;
    std::string packet;

    // 接收认证消息
    while (true) {

        int len = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
        );

        if (len <= 0) {
            closesocket(clientSocket);
            return;
        }

        receiveBuffer.append(
            buffer,
            len
        );

        size_t pos =
            receiveBuffer.find('\n');

        if (pos != std::string::npos) {

            packet =
                receiveBuffer.substr(
                    0,
                    pos
                );

            receiveBuffer.erase(
                0,
                pos + 1
            );

            break;
        }
    }

    size_t pos1 =
        packet.find('|');

    size_t pos2 =
        packet.find('|', pos1 + 1);

    // 拆分客户端发送的认证数据
    // 格式：type|username|password
    if (pos1 == std::string::npos ||
        pos2 == std::string::npos) {

        closesocket(clientSocket);
        return;
    }

    std::string type =
        packet.substr(
            0,
            pos1
        );

    std::string username =
        packet.substr(
            pos1 + 1,
            pos2 - pos1 - 1
        );

    std::string password =
        packet.substr(
            pos2 + 1
        );

    std::cout << "Type: "
              << type
              << std::endl;

    std::cout << "Username: "
              << username
              << std::endl;

    // 交给身份认证函数处理登录/注册
    if (!handleAuthentication(
        type,
        username,
        password,
        clientSocket
    )) {

        closesocket(clientSocket);
        return;
    }

    while (true) {

        int len = recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
        );

        std::cout << "Recv bytes: "
                  << len
                  << std::endl;

        if (len > 0) {

            receiveBuffer.append(
                buffer,
                len
            );

            size_t pos;

            while (
                (pos = receiveBuffer.find('\n'))
                != std::string::npos
            ) {

                std::string message =
                    receiveBuffer.substr(
                        0,
                        pos
                    );

                receiveBuffer.erase(
                    0,
                    pos + 1
                );

                if (!message.empty()) {

                    handleMessage(
                        username,
                        message,
                        clientSocket
                    );
                }
            }
        }
        else {
            break;
        }
    }

    clientManager->removeClient(
        clientSocket
    );

    clientManager->removeUserSocket(
        username,
        clientSocket
    );

    std::cout << "Client disconnected."
              << std::endl;

    std::cout << "Online clients: "
              << clientManager->getClientCount()
              << std::endl;

    closesocket(clientSocket);
}

// 处理添加好友请求
bool Server::handleAddFriend(
    const std::string& username,
    const std::string& friendUsername,
    SOCKET clientSocket
)
{
    // 不能添加自己
    if (username == friendUsername) {

        std::string response =
            "ADD_FRIEND_FAILED";

        sendMessage(
            clientSocket,
            response
        );

        return false;
    }

    // 检查好友是否存在
    std::string sql =
        "SELECT id FROM users WHERE username = '" +
        friendUsername + "'";

    if (mysql_query(
        mysqlConnection,
        sql.c_str()
    ) != 0) {

        std::cout << "Check friend user failed: "
                  << mysql_error(mysqlConnection)
                  << std::endl;

        return false;
    }

    MYSQL_RES* result =
        mysql_store_result(mysqlConnection);

    if (result == nullptr) {

        std::cout << "Get friend user result failed: "
                  << mysql_error(mysqlConnection)
                  << std::endl;

        return false;
    }

    MYSQL_ROW row =
        mysql_fetch_row(result);

    // 好友不存在
    if (row == nullptr) {

        mysql_free_result(result);

        std::string response =
            "ADD_FRIEND_FAILED";

        sendMessage(
            clientSocket,
            response
        );

        return false;
    }

    mysql_free_result(result);

    // 调用 FriendManager 添加好友
    if (friendManager->addFriend(
        username,
        friendUsername
    )) {

        std::string response =
            "ADD_FRIEND_SUCCESS";

        sendMessage(
            clientSocket,
            response
        );

        std::cout << "Friend added: "
                  << username
                  << " <-> "
                  << friendUsername
                  << std::endl;

        return true;
    }

    // 添加失败
    std::string response =
        "ADD_FRIEND_FAILED";

    sendMessage(
        clientSocket,
        response
    );

    return false;
}

void Server::handleGetFriends(
    const std::string& username,
    SOCKET clientSocket
)
{
    std::vector<std::string> friends =
        friendManager->getFriends(username);

    std::string response =
        "FRIEND_LIST";

    for (const std::string& friendUsername : friends) {

        response += "|" + friendUsername;
    }

    sendMessage(
        clientSocket,
        response
    );

    std::cout << "Friend list sent: "
              << username
              << std::endl;
}

void Server::handleSearchUser(
    const std::string& keyword,
    SOCKET clientSocket
)
{
    std::vector<std::string> users =
        userManager->searchUser(keyword);

    std::string response =
        "SEARCH_RESULT";

    for (const std::string& username : users) {

        response += "|" + username;
    }

    sendMessage(
        clientSocket,
        response
    );

    std::cout << "Search result sent."
              << std::endl;
}

void Server::handleCreateGroup(
    const std::string& username,
    const std::string& groupName,
    SOCKET clientSocket
)
{
    // 创建群，并让创建者自动成为群主和群成员
    int groupId =
        groupManager->createGroup(
            groupName,
            username
        );

    if (groupId == -1) {

        std::string response =
            "CREATE_GROUP_FAILED";

        sendMessage(
            clientSocket,
            response
        );

        return;
    }

    // 返回创建成功以及群 ID
    std::string response =
        "CREATE_GROUP_SUCCESS|" +
        std::to_string(groupId);

    sendMessage(
        clientSocket,
        response
    );
}

void Server::handleAddGroupMember(
    const std::string& username,
    int groupId,
    const std::string& memberUsername,
    SOCKET clientSocket
)
{
    // 当前版本先直接调用 GroupManager
    // 后续可以增加“只有群主才能邀请成员”的权限判断
    if (groupManager->addMember(
        groupId,
        memberUsername
    )) {

        std::string response =
            "ADD_GROUP_MEMBER_SUCCESS";

        sendMessage(
            clientSocket,
            response
        );

        return;
    }

    std::string response =
        "ADD_GROUP_MEMBER_FAILED";

    sendMessage(
        clientSocket,
        response
    );
}

void Server::handleGetGroupMembers(
    int groupId,
    SOCKET clientSocket
)
{
    std::vector<std::string> members =
        groupManager->getGroupMembers(groupId);

    std::string response =
        "GROUP_MEMBERS";

    for (const std::string& username : members) {

        response += "|" + username;
    }

    sendMessage(
        clientSocket,
        response
    );
}

// 获取当前用户加入的所有群聊
void Server::handleGetGroups(
    const std::string& username,
    SOCKET clientSocket
)
{
    std::vector<std::string> groups =
        groupManager->getGroups(username);

    std::string response =
        "GROUP_LIST";

    for (const std::string& groupName : groups) {

        response += "|" + groupName;
    }

    sendMessage(
        clientSocket,
        response
    );

    std::cout << "Group list sent: "
              << username
              << std::endl;
}

bool Server::start()
{
    if (serverSocket == INVALID_SOCKET) {

        std::cout << "Server socket creation failed!"
                  << std::endl;

        return false;
    }

    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;

    serverAddr.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    serverAddr.sin_port =
        htons(8888);

    int result = bind(
        serverSocket,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)
    );

    if (result == SOCKET_ERROR) {

        std::cout << "Bind failed!"
                  << std::endl;

        return false;
    }

    std::cout << "Bind success!"
              << std::endl;

    if (listen(
        serverSocket,
        5
    ) == SOCKET_ERROR) {

        std::cout << "Listen failed!"
                  << std::endl;

        return false;
    }

    std::cout
        << "Server is listening on 127.0.0.1:8888"
        << std::endl;

    while (true)
    {
        SOCKET clientSocket =
            accept(
                serverSocket,
                nullptr,
                nullptr
            );

        if (clientSocket == INVALID_SOCKET) {

            std::cout << "Accept failed!"
                      << std::endl;

            return false;
        }

        std::cout << "Client connected!"
                  << std::endl;

        std::thread clientThread(
            &Server::handleClient,
            this,
            clientSocket
        );

        clientThread.detach();
    }

    return true;
}