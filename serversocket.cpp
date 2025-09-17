#include "serversocket.h"
#include <QNetworkInterface>
#include <QDebug>
#include <QThread>

Server::Server(QObject* parent)
    : QObject(parent),
    server(nullptr),
    makrolineSocket(nullptr),
    plcSocket(nullptr),
    isPrinting(false),
    isProcessingOut(false)
{
}

Server::~Server()
{
    disconnectServer();
}

void Server::setConnectionParams(const QString &ip, const QString &port)
{
    if (server_ip == ip && server_port == port) return;
    server_ip = ip;
    server_port = port;
    emit logMessage(QString("[Server] Параметры подключения: %1:%2").arg(ip, port));
}

void Server::startServer()
{
    doWork();
}

void Server::stopServer()
{
    disconnectServer();
}

void Server::doWork()
{
    qDebug() << "Запуск сервера в потоке:" << QThread::currentThread();

    // Создаем server в том же потоке, где будет работать
    if (!server) {
        qDebug() << "Создание QTcpServer...";
        server = new QTcpServer(this);

        // ТЕПЕРЬ подключаем сигналы после создания server
        connect(server, &QTcpServer::newConnection, this, &Server::onNewConnection);

        connect(server, &QTcpServer::acceptError, this, [this](QAbstractSocket::SocketError socketError) {
            qDebug() << "Ошибка принятия подключения сервером:" << socketError << server->errorString();
            emit logMessage(QString("[Server] Ошибка принятия подключения: %1").arg(server->errorString()));
        });
        qDebug() << "QTcpServer создан и сигналы подключены";
    }

    if (server->isListening()) {
        emit logMessage("[Server] Сервер уже запущен");
        return;
    }

    bool ok;
    quint16 port = server_port.toUShort(&ok);
    if (!ok || port == 0) {
        emit logMessage("[Server] Неверный порт");
        return;
    }

    if (!server->listen(QHostAddress::Any, port)) {
        emit logMessage(QString("[Server] Ошибка запуска: %1").arg(server->errorString()));
        return;
    }

    emit logMessage(QString("[Server] Сервер запущен на порту %1").arg(port));
}

void Server::onNewConnection()
{
    if (makrolineSocket) {
        makrolineSocket->disconnectFromHost();
        makrolineSocket->deleteLater();
    }

    makrolineSocket = server->nextPendingConnection();
    connect(makrolineSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(makrolineSocket, &QTcpSocket::disconnected, this, &Server::onDisconnected);
    connect(makrolineSocket, &QTcpSocket::bytesWritten, this, &Server::onBytesWritten);

    emit logMessage("[Server] Устройство подключено");
    emit connectionChanged(true);
}

void Server::onDisconnected()
{
    if (!makrolineSocket) return;

    emit logMessage("[Server] Устройство отключено");

    // Отключаем все сигналы, чтобы слоты не вызывались после удаления
    makrolineSocket->disconnect(this);
    makrolineSocket->deleteLater();
    makrolineSocket = nullptr;

    emit connectionChanged(false);
}

void Server::onReadyRead()
{
    if (!makrolineSocket) return;

    QByteArray data = makrolineSocket->readAll();
    QString text = QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()), data.size() / 2);

    QString hex;
    for (char byte : data)
        hex += QString("%1 ").arg((quint8)byte, 2, 16, QLatin1Char('0')).toUpper();

    emit logMessage(QString("[Server] Получено: %1 (HEX: %2)").arg(text, hex.trimmed()));

    queue.append(data);
    if (!isPrinting) processNext();
}

void Server::ResponseMakroline(const QString &data_response)
{
    if (!makrolineSocket || makrolineSocket->state() != QTcpSocket::ConnectedState) {
        emit logMessage("[Server] Сокет не подключен");
        return;
    }

    QByteArray sendData;
    QDataStream stream(&sendData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream.writeRawData(reinterpret_cast<const char*>(data_response.utf16()), data_response.size() * 2);

    const char16_t cr = 0x000D;
    stream.writeRawData(reinterpret_cast<const char*>(&cr), 2);

    QString hexStr = sendData.toHex(' ').toUpper();
    QString textStr = QString::fromUtf16(reinterpret_cast<const char16_t*>(sendData.constData()), (sendData.size() - 2) / 2);
    emit logMessage(QString("[Server] Ответ на команду: %1 (HEX: %2)").arg(textStr.trimmed(), hexStr));

    queueOut.append(sendData);
    if (!isProcessingOut) processNextOut();
}

void Server::ResponsePLC(const QByteArray &data)
{

}

void Server::processNext()
{
    if (queue.isEmpty()) {
        isPrinting = false;
        return;
    }

    isPrinting = true;
    QByteArray command = queue.takeFirst();
    emit commandReceived(command);

    QTimer::singleShot(10, this, [this] {
        isPrinting = false;
        processNext();
    });
}

void Server::processNextOut()
{
    if (!makrolineSocket || makrolineSocket->state() != QTcpSocket::ConnectedState || queueOut.isEmpty()) {
        isProcessingOut = false;
        return;
    }

    isProcessingOut = true;
    QByteArray data = queueOut.takeFirst();
    makrolineSocket->write(data);
}

void Server::onBytesWritten(qint64)
{
    if (!queueOut.isEmpty()) processNextOut();
    else isProcessingOut = false;
}

void Server::disconnectServer()
{
    if (server && server->isListening()) {
        server->close();
        emit logMessage("[Server] Сервер остановлен");
    }

    if (makrolineSocket) {
        makrolineSocket->disconnect(this); // отключаем все сигналы
        makrolineSocket->abort();          // безопасно закрывает сокет
        makrolineSocket->deleteLater();
        makrolineSocket = nullptr;
    }

    emit connectionChanged(false);
    emit logMessage("[Server] Сервер и клиент отключены");
}

