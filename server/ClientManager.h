#pragma once

#include <winsock2.h>
#include <vector>
#include <map>
#include <mutex>
#include <string>

class ClientManager
{
private:
    std::vector<SOCKET> clients;
    std::mutex clientsMutex;

    std::map<std::string, SOCKET> userSockets;
    std::mutex userSocketsMutex;

public:
    void addClient(SOCKET clientSocket);

    void removeClient(SOCKET clientSocket);

    void addUserSocket(
        const std::string& username,
        SOCKET clientSocket
    );

    void removeUserSocket(
        const std::string& username,
        SOCKET clientSocket
    );

    SOCKET getUserSocket(
        const std::string& username
    );

    int getClientCount();
};