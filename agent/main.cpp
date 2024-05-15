#include <QCoreApplication>

#include "client.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Client *client = new Client();
    client->connectToServer("localhost", 6547);

    return a.exec();
}
