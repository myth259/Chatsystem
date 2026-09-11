#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpClient tcpClient;

    MainWindow w("test", &tcpClient);
    w.show();
    return QApplication::exec();
}
