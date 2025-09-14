#include "clientsocket.h"
#include <QDebug>


ClientSocket::ClientSocket(QObject *parent)
    : QObject(parent),
    socket(nullptr),
    isConnected(false)
{
}

void ClientSocket::doWork()
{

}

void ClientSocket::setConnectionParams(const QString &ip, const QString &port)
{

}

void ClientSocket::disconnectSocket()
{

}

void ClientSocket::receiveCommandPrinter()
{

}

void ClientSocket::onConnected()
{

}

void ClientSocket::onErrorOccurred()
{

}


void connectionChanged(bool checked)
{

}

void logMessage(const QString& message)
{

}
