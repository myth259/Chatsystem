#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include "../TcpClient.h"

class MainWindow;

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginWindow;
}
QT_END_NAMESPACE

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow() override;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::LoginWindow *ui;
    TcpClient tcpClient;
    MainWindow *mainWindow;
};

#endif // LOGINWINDOW_H
