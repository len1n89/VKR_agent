#include "client.h"

#include <QTcpSocket>
#include <QDataStream>
#include <QTimer>

#include <QDebug>

Client::Client()
    : QObject()
    , m_status(false)
    , m_nNextBlockSize(0)
    , m_tcpSocket(new QTcpSocket())
    , m_timeoutTimer(new QTimer())
{
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &Client::connectionTimeout);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &Client::closeConnection);
}

void Client::connectToServer(const QString address, int port)
{
    m_timeoutTimer->start(3000);
    m_tcpSocket->connectToHost(address, port);

    connect(m_tcpSocket, &QTcpSocket::connected, this, &Client::connected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &Client::readyRead);
}

void Client::closeConnection()
{
    qDebug() << m_tcpSocket->state();
    disconnect(m_tcpSocket, &QTcpSocket::connected, 0, 0);
    disconnect(m_tcpSocket, &QTcpSocket::readyRead, 0, 0);

    if(m_tcpSocket->state() == QTcpSocket::UnconnectedState) {
        m_tcpSocket->disconnectFromHost();
        m_status = false;
        emit statusChanged(m_status);
    }
    else if(m_tcpSocket->state() == QTcpSocket::ConnectingState) {
        m_tcpSocket->abort();
        m_status = false;
        emit statusChanged(m_status);
    }
    else
        m_tcpSocket->abort();
}

qint64 Client::sendToServer(QTcpSocket *socket, const QString &str)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);

    out << quint16(0) << str;
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    return socket->write(arrBlock);
}

void Client::readyRead()
{
    QDataStream in(m_tcpSocket);

    for (;;)
    {
        if (!m_nNextBlockSize)
        {
            if (m_tcpSocket->bytesAvailable() < sizeof(quint16))
                break;
            in >> m_nNextBlockSize;
        }

        if (m_tcpSocket->bytesAvailable() < m_nNextBlockSize)
            break;

        QString str;
        in >> str;

        if (str == "0")
        {
            str = "Connection closed";
            closeConnection();
        }

//        emit hasReadSome(str);
        qDebug()<<"FROM SERVER: "<<str;
        m_nNextBlockSize = 0;
    }
}

void Client::connected()
{
    m_status = true;
    sendToServer(m_tcpSocket, "STRING from client");
    emit statusChanged(m_status);
}

void Client::connectionTimeout()
{
    qDebug()<<"connectionTimeout "<<m_tcpSocket->state();

    if(m_tcpSocket->state() == QAbstractSocket::ConnectingState)
    {
        qDebug() <<"connectionTimeout ConnectingState";
        m_tcpSocket->abort();
        emit m_tcpSocket->error(QAbstractSocket::SocketTimeoutError);
    }
}
