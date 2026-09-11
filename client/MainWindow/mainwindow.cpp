#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "FriendDialog.h"
#include <QDebug>

MainWindow::MainWindow(
    const QString &username,
    TcpClient *tcpClient,
    QWidget *parent
    )
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , username(username)
    , tcpClient(tcpClient)
    , friendDialog(nullptr)
{
    ui->setupUi(this);
    connect(ui->conversationList, &QListWidget::itemClicked,
            this, &MainWindow::onConversationClicked);
    connect(ui->friendButton, &QPushButton::clicked,
            this, &MainWindow::openFriendDialog);
    connect(ui->sendButton, &QPushButton::clicked,
            this, [this]()
            {
                qDebug() << "发送按钮被点击";
                QString message = ui->messageInput->toPlainText();

                qDebug() << "message =" << message;
                qDebug() << "currentChatUser =" << currentChatUser;

                if (message.trimmed().isEmpty())
                {
                    return;
                }

                if (currentChatUser.isEmpty())
                {
                    return;
                }

                QString packet =
                    currentChatUser + ":" + message;

                qDebug() << "准备发送：" << packet;

                this->tcpClient->sendMessage(packet);

                chatHistory[currentChatUser].append(
                    "我：" + message
                    );

                ui->messageDisplay->append(
                    "我：" + message
                    );

                ui->messageInput->clear();
            });
    connect(tcpClient, &TcpClient::messageReceived,
            this, [this](const QString &message)
            {
                QStringList parts = message.split("|");

                if (parts[0] == "SEARCH_RESULT")
                {
                    qDebug() << "搜索结果：" << parts;

                    parts.removeFirst();

                    if (friendDialog != nullptr)
                    {
                        friendDialog->setSearchResults(parts);
                    }

                    return;
                }

                if (parts.size() < 3)
                {
                    return;
                }

                if (parts[0] != "MESSAGE")
                {
                    return;
                }

                QString sender = parts[1];
                QString content = parts[2];

                chatHistory[sender].append(
                    sender + "：" + content
                    );

                QList<QListWidgetItem*> items =
                    ui->conversationList->findItems(
                        sender,
                        Qt::MatchExactly
                        );

                if (items.isEmpty())
                {
                    ui->conversationList->addItem(sender);
                }

                if (currentChatUser == sender)
                {
                    ui->messageDisplay->append(
                        sender + "：" + content
                        );
                }
            });
    qDebug() << "当前登录用户：" << username;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFriendDialog()
{

    tcpClient->sendMessage("GET_FRIENDS");

    QString response = tcpClient->receiveMessage();

    qDebug() << "服务器返回好友列表：" << response;

    QStringList friends = response.split("|");

    if (!friends.isEmpty() && friends[0] == "FRIEND_LIST")
    {
        friends.removeFirst();
    }

    friendDialog = new FriendDialog(username, this);

    friendDialog->setFriends(friends);

    connect(friendDialog, &FriendDialog::friendSelected,
            this, [this](const QString &friendName)
            {
                qDebug() << "选择了好友：" << friendName;
                currentChatUser = friendName;
                if (ui->conversationList->findItems(friendName, Qt::MatchExactly).isEmpty())
                {
                    ui->conversationList->addItem(friendName);
                }
            });
    connect(friendDialog, &FriendDialog::searchRequested,
            this, [this](const QString &keyword)
            {
                qDebug() << "搜索用户：" << keyword;

                QString packet =
                    "SEARCH_USER|" + keyword;

                tcpClient->sendMessage(packet);
            });

    friendDialog->exec();
    friendDialog = nullptr;
}

void MainWindow::onConversationClicked(QListWidgetItem *item)
{
    QString friendName = item->text();

    currentChatUser = friendName;

    qDebug() << "当前聊天对象：" << currentChatUser;

    ui->messageDisplay->clear();

    ui->messageDisplay->append(
        "正在与 " + currentChatUser + " 聊天"
        );

    for (const QString &message : chatHistory[currentChatUser])
    {
        ui->messageDisplay->append(message);
    }
}