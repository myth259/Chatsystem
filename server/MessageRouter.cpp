#include "MessageRouter.h"
#include <iostream>

MessageRouter::MessageRouter(ClientManager* manager)
{
    clientManager = manager;
}

bool MessageRouter::sendMessage(
    const std::string& sender,
    const std::string& receiver,
    const std::string& message
)
{
    SOCKET targetSocket =
        clientManager->getUserSocket(receiver);

    if (targetSocket == INVALID_SOCKET) {
        return false;
    }

    std::string packet =
        "MESSAGE|" + sender + "|" + message + "\n";

    int result = send(
        targetSocket,
        packet.c_str(),
        static_cast<int>(packet.length()),
        0
    );

    if (result == SOCKET_ERROR) {
        return false;
    }

    std::cout << "Message routed: "
              << sender
              << " -> "
              << receiver
              << std::endl;

    return true;
}

bool MessageRouter::sendGroupMessage(
    const std::vector<std::string>& members,
    const std::string& message
)
{
    for (const std::string& username : members) {

        SOCKET targetSocket =
            clientManager->getUserSocket(username);

        // 用户不在线，暂时跳过
        if (targetSocket == INVALID_SOCKET) {
            continue;
        }

        int result = send(
            targetSocket,
            message.c_str(),
            static_cast<int>(message.length()),
            0
        );

        if (result == SOCKET_ERROR) {
            std::cout << "Send group message failed: "
                      << username
                      << std::endl;
        }
    }

    return true;
}