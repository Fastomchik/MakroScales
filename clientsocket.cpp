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
    if (printer_ip.isEmpty() || printer_port.isEmpty())
    {
        emit logMessage("[PrinterWorker] IP или порт не установлены!");
        return;
    }

    bool ok;
    int port = printer_port.toInt(&ok);
    if(!ok || port < 1 || port > 65535){
        emit logMessage("[PrinterWorker] Неверный номер порта!");
        return;
    }

    if (socket){
        socket->deleteLater();
    }

    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, &ClientSocket::onConnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &ClientSocket::onErrorOccurred);
    connect(socket, &QTcpSocket::readyRead, this, &ClientSocket::handleAnswer);


    emit logMessage(QString("[PrinterWorker] Подключаемся к %1, %2")
                        .arg(printer_ip).arg(printer_port));
    socket->connectToHost(printer_ip, port);


}

void ClientSocket::setConnectionParams(const QString &ip, const QString &port)
{
    if (printer_ip == ip && printer_port == port) return;
    printer_ip = ip;
    printer_port = port;
    emit logMessage(QString("[PrinterWorker] Параметры подключения установлены ip: %1 port: %2").arg(ip, port));
}

void ClientSocket::disconnectSocket()
{
    if ((socket && socket->isOpen())) {
        if (socket)  socket->disconnectFromHost();
        emit logMessage("[PrinterWorker] отключение от сокета");
        emit connectionChanged(false);
        isConnected = false;
    } else {
        emit logMessage("[PrinterWorker] сокет уже отключён");
    }
}

void ClientSocket::onErrorOccurred()
{
    isConnected = false;
    emit logMessage(QString("[PrinterWorker] Ошибка подключения: %1")
                        .arg(socket  ? socket->errorString()  : QStringLiteral("<нет>")));
    emit connectionChanged(false);
}

void ClientSocket::onConnected()
{
    if (socket->state() == QTcpSocket::ConnectedState){
        isConnected = true;
        emit logMessage("[PrinterWorker] Успешно подключено к принтеру!");
        emit connectionChanged(true);
    }
}

/*void ClientSocket::receiveCommandPrinter()
{

}*/

void ClientSocket::handleAnswer()
{

}
