cmake_minimum_required(VERSION 3.10)

project(ChatSystem)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# MySQL
include_directories("C:/Program Files/MySQL/MySQL Server 8.0/include")
link_directories("C:/Program Files/MySQL/MySQL Server 8.0/lib")

add_executable(server
    server/Server.cpp
    server/Database.cpp
    server/UserManager.cpp
    server/FriendManager.cpp
    server/ClientManager.cpp
    server/MessageRouter.cpp
    server/MessagePermission.cpp
    server/GroupManager.cpp
    server/main.cpp
)

add_executable(client
    client/client.cpp
    client/TcpClient.cpp
    client/LoginWindow/LoginWindow.cpp
    client/LoginWindow/LoginWindow.ui

    client/MainWindow/mainwindow.cpp
    client/MainWindow/mainwindow.h
    client/MainWindow/mainwindow.ui

    client/MainWindow/FriendDialog.cpp
    client/MainWindow/FriendDialog.h
    client/MainWindow/FriendDialog.ui
)

target_link_libraries(client
    Qt6::Widgets
    Qt6::Network
)

target_link_libraries(server
    libmysql
    ws2_32
)