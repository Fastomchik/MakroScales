#include "clientsocket.h"
#include <QDebug>
#include <QQueue>


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
        emit logMessage("[Client] IP или порт не установлены!");
        return;
    }

    bool ok;
    int port = printer_port.toInt(&ok);
    if(!ok || port < 1 || port > 65535){
        emit logMessage("[Client] Неверный номер порта!");
        return;
    }

    if (socket){
        socket->deleteLater();
    }

    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::connected, this, &ClientSocket::onConnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &ClientSocket::onErrorOccurred);
    //connect(socket, &QTcpSocket::readyRead, this, &ClientSocket::handleAnswer);


    emit logMessage(QString("[Client] Подключаемся к %1, %2")
                        .arg(printer_ip).arg(printer_port));
    socket->connectToHost(printer_ip, port);
}

void ClientSocket::onConnected()
{
    if (socket->state() == QTcpSocket::ConnectedState){
        isConnected = true;
        emit logMessage("[Client] Успешно подключено к принтеру!");
        emit connectionChanged(true);
    }
}

void ClientSocket::disconnectSocket()
{
    if ((socket && socket->isOpen())) {
        if (socket)  socket->disconnectFromHost();
        emit logMessage("[Client] отключение от сокета");
        emit connectionChanged(false);
        isConnected = false;
    } else {
        emit logMessage("[Client] сокет уже отключён");
    }
}

void ClientSocket::onErrorOccurred()
{
    isConnected = false;
    emit logMessage(QString("[Client] Ошибка подключения: %1")
                        .arg(socket  ? socket->errorString()  : QStringLiteral("<нет>")));
    emit connectionChanged(false);
}

// Слот получения команды от bridgelinxtocab
void ClientSocket::sendCommandPrinter(const QByteArray &command, const Constants::TypeCommandCab commandtype)
{
    switch (commandtype)
    {
    case Constants::TypeCommandCab::AddCode:
        printQueue.enqueue(command);
        emit logMessage(QString("[Client] Код добавлен: %1 / в очереди %2").arg(QString::fromUtf8(command)).arg(printQueue.size()));
        fillPrinterBuffer();
        break;
    case Constants::TypeCommandCab::ClearBuffers:
        clearBuffers();
        break;
    case Constants::TypeCommandCab::StartPrint:
        startPrint();
        break;
    case Constants::TypeCommandCab::StopPrint:
        stopPrint();
        break;
    case Constants::TypeCommandCab::RequestStatus:
        requestStatus();
        break;
    }
}

// Обработка команд


void ClientSocket::fillPrinterBuffer()
{
    qDebug() << "вызов fillPrinterBuffer";
    if (!isConnected) {
        emit logMessage("[Client] Сокет не подключен, не могу отправить в принтер");
        return;
    }

    while (!printQueue.isEmpty()) {
        QByteArray command = printQueue.dequeue();

        qint64 written = socket->write(command);
        if (written == -1) {
            emit logMessage("[Client] Ошибка отправки команды в принтер");
        } else {
            emit logMessage("[Client] Команда отправлена в принтер: " + QString::fromUtf8(command));
        }

        socket->flush(); // сразу отправляем в сокет

        emit successfulPrintedInMakroline("PRC");
    }

}

void ClientSocket::clearBuffers()
{

}

void ClientSocket::startPrint()
{

}

void ClientSocket::stopPrint()
{

}

void ClientSocket::requestStatus()
{

}

// Сеттеры
void ClientSocket::setConnectionParams(const QString &ip, const QString &port)
{
    if (printer_ip == ip && printer_port == port) return;
    printer_ip = ip;
    printer_port = port;
    emit logMessage(QString("[Client] Параметры подключения установлены ip: %1 port: %2").arg(ip, port));
}

// Слоты
void ClientSocket::connectToServer()
{
    doWork();
}

void ClientSocket::disconnectFromServer()
{
    disconnectSocket();
}
