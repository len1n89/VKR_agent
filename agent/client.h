#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>

class QTcpSocket;
class QTimer;

class Client : public QObject
{
    Q_OBJECT

public:
    Client();
    ~Client(){};

public:
    void connectToServer(const QString address, int port);

public slots:
    void closeConnection();
    qint64 sendToServer(QTcpSocket *socket, const QString &str);

signals:
    void statusChanged(bool status);

private slots:
    void readyRead();
    void connected();
    void connectionTimeout();

private:
    bool m_status;
    quint16 m_nNextBlockSize;
    QTcpSocket *m_tcpSocket;
    QTimer *m_timeoutTimer;
};

#endif // CLIENT_H
