#include "Server.h"
#include <iostream>

int main()
{
    std::cout << "Server program started!" << std::endl;

    Server server;

    std::cout << "Server object created!" << std::endl;

    if (!server.start()) {
        std::cout << "Server start failed!" << std::endl;
        return 1;
    }

    return 0;
}