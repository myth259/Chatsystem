#include "LoginWindow.h"
#include "./ui_LoginWindow.h"
#include "../MainWindow/mainwindow.h"
#include <QMessageBox>
#include <QKeyEvent>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoginWindow)
    , mainWindow(nullptr)
{
    ui->setupUi(this);
    ui->usernameInput->installEventFilter(this);
    ui->passwordInput->installEventFilter(this);
    connect(ui->loginButton, &QPushButton::clicked, this, [this]()
            {
                QString username = ui->usernameInput->text();
                QString password = ui->passwordInput->text();

                if (username.isEmpty() || password.isEmpty())
                {
                    QMessageBox::warning(
                        this,
                        "登录失败",
                        "用户名或密码不能为空！"
                        );
                    return;
                }

                if (tcpClient.connectToServer("127.0.0.1", 8888))
                {
                    QString message =
                        "LOGIN|" + username + "|" + password;

                    tcpClient.sendMessage(message);
                    QString response = tcpClient.receiveMessage();

                    qDebug() << "服务器返回：" << response;

                    if (response.startsWith("LOGIN_SUCCESS"))
                    {
                        mainWindow = new MainWindow(username, &tcpClient);
                        mainWindow->show();

                        this->close();
                    }
                    else
                    {
                        QMessageBox::warning(
                            this,
                            "登录失败",
                            "用户名或密码错误！"
                            );
                    }
                }
                else
                {
                    qDebug() << "连接服务器失败";
                }
            });

    connect(ui->registerButton, &QPushButton::clicked, this, [this]()
            {
                QString username = ui->usernameInput->text();
                QString password = ui->passwordInput->text();

                if (username.trimmed().isEmpty() || password.trimmed().isEmpty())
                {
                    QMessageBox::warning(
                        this,
                        "注册失败",
                        "用户名或密码不能为空！"
                        );
                    return;
                }

                if (tcpClient.connectToServer("127.0.0.1", 8888))
                {
                    QString message =
                        "REGISTER|" + username + "|" + password;

                    tcpClient.sendMessage(message);

                    QString response = tcpClient.receiveMessage();

                    qDebug() << "服务器返回：" << response;

                    if (response.startsWith("REGISTER_SUCCESS"))
                    {
                        mainWindow = new MainWindow(username, &tcpClient);
                        mainWindow->show();

                        this->close();
                    }
                    else
                    {
                        QMessageBox::warning(
                            this,
                            "注册失败",
                            "用户名已存在或注册失败！"
                            );
                    }
                }
                else
                {
                    qDebug() << "连接服务器失败";
                }
            });
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

bool LoginWindow::eventFilter(QObject *obj, QEvent *event)
{
    if ((obj == ui->usernameInput || obj == ui->passwordInput)
        && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Space)
        {
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}