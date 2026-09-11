#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QByteArray>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    TcpClient();

    bool connectToServer(
        const QString& host,
        quint16 port
    );

    void sendMessage(const QString& message);

    QString receiveMessage();

signals:
    void messageReceived(const QString& message);

private:
    QTcpSocket socket;
    QByteArray receiveBuffer;
    bool waitingForResponse;
};

#endif // TCPCLIENT_H