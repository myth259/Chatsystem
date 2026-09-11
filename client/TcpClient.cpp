#include "TcpClient.h"
#include <QDebug>

TcpClient::TcpClient()
    : waitingForResponse(false)
{
    connect(&socket, &QTcpSocket::readyRead,
        this, [this]()
        {
            receiveBuffer += socket.readAll();

            if (waitingForResponse)
            {
                return;
            }

            while (true)
            {
                int pos = receiveBuffer.indexOf('\n');

                if (pos == -1)
                {
                    break;
                }

                QByteArray message =
                    receiveBuffer.left(pos);

                receiveBuffer.remove(0, pos + 1);

                if (!message.isEmpty())
                {
                    emit messageReceived(
                        QString::fromUtf8(message)
                    );
                }
            }
        });
}

bool TcpClient::connectToServer(
    const QString& host,
    quint16 port
)
{
    socket.connectToHost(host, port);

    return socket.waitForConnected(3000);
}

void TcpClient::sendMessage(const QString& message)
{
    qDebug() << "进入 sendMessage";
    if (socket.state() != QAbstractSocket::ConnectedState)
    {
        qDebug() << "Socket未连接，无法发送消息";
        return;
    }

    QByteArray data = (message + "\n").toUtf8();

    qint64 result = socket.write(data);

    qDebug() << "发送消息：" << message;
    qDebug() << "发送字节数：" << result;

    socket.flush();
}

QString TcpClient::receiveMessage()
{
    waitingForResponse = true;

    while (true)
    {
        int pos = receiveBuffer.indexOf('\n');

        if (pos != -1)
        {
            QByteArray message =
                receiveBuffer.left(pos);

            receiveBuffer.remove(0, pos + 1);

            waitingForResponse = false;

            return QString::fromUtf8(message);
        }

        if (!socket.waitForReadyRead(3000))
        {
            waitingForResponse = false;
            return "";
        }

        receiveBuffer += socket.readAll();
    }
}