#include "ClientManager.h"

void ClientManager::addClient(SOCKET clientSocket)
{
    std::lock_guard<std::mutex> lock(clientsMutex);

    clients.push_back(clientSocket);
}

void ClientManager::removeClient(SOCKET clientSocket)
{
    std::lock_guard<std::mutex> lock(clientsMutex);

    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (*it == clientSocket) {
            clients.erase(it);
            break;
        }
    }
}

void ClientManager::addUserSocket(
    const std::string& username,
    SOCKET clientSocket
) {
    std::lock_guard<std::mutex> lock(userSocketsMutex);

    userSockets[username] = clientSocket;
}

void ClientManager::removeUserSocket(
    const std::string& username,
    SOCKET clientSocket
) {
    std::lock_guard<std::mutex> lock(userSocketsMutex);

    auto it = userSockets.find(username);

    if (it != userSockets.end() && it->second == clientSocket) {
        userSockets.erase(it);
    }
}

SOCKET ClientManager::getUserSocket(
    const std::string& username
) {
    std::lock_guard<std::mutex> lock(userSocketsMutex);

    auto it = userSockets.find(username);

    if (it != userSockets.end()) {
        return it->second;
    }

    return INVALID_SOCKET;
}

int ClientManager::getClientCount()
{
    std::lock_guard<std::mutex> lock(clientsMutex);

    return clients.size();
}