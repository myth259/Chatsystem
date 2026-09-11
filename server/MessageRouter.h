#pragma once

#include <winsock2.h>
#include <string>

#include "ClientManager.h"

class MessageRouter
{
private:
    ClientManager* clientManager;

public:
    MessageRouter(ClientManager* manager);

    bool sendMessage(
        const std::string& sender,
        const std::string& receiver,
        const std::string& message
    );

    bool sendGroupMessage(
        const std::vector<std::string>& members,
        const std::string& message
    );
};