#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QListWidgetItem>
#include <QMap>
#include <QStringList>
#include "../TcpClient.h"
#include "FriendDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        const QString &username,
        TcpClient *tcpClient,
        QWidget *parent = nullptr
        );
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    QString username;
    TcpClient *tcpClient;
    QString currentChatUser;
    QMap<QString, QStringList> chatHistory;
    FriendDialog *friendDialog;

    void onConversationClicked(QListWidgetItem *item);
    void openFriendDialog();

};

#endif // MAINWINDOW_H