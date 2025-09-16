#include "serversocket.h"
#include <QNetworkInterface>

Server::Server(QObject *parent)
    : QObject(parent),
    server(new QTcpServer(this)),
    makrolineSocket(nullptr),
    plcSocket(nullptr),
    isPrinting(false),
    isProcessingOut(false)
{
    connect(server, &QTcpServer::newConnection, this, &Server::onNewConnection);
}

Server::~Server()
{
    disconnectServer();
}

void Server::setConnectionParams(const QString &ip, const QString &port)
{
    if(server_ip == ip && server_port == port) return;
    server_ip = ip;
    server_port = port;
    emit logMessage(QString("[Server] Параметры подключения: %1:%2").arg(ip,port));
}

void Server::doWork()
{
    if (!server) return;

    if (server->isListening()){
        emit logMessage ("[Server] Сервер уже запущен");
        return;
    }

    bool ok;
    quint16 port = server_port.toUShort(&ok);
    if (!ok || port == 0){
        emit logMessage("[Server] Неверный порт");
        return;
    }

    if (!server->listen(QHostAddress::Any, port)) {
        emit logMessage(QString("[Server] Ошибка запуска: %1").arg(server->errorString()));
        return;
    }

    emit logMessage(QString("[Server] Сервер запущен на порту %1").arg(port));
    emit logMessage("[Server] Ожидание подключения ПО Makroline и ПЛК...");
}

void Server::onNewConnection()
{
    QTcpSocket *newSocket = server->nextPendingConnection();
    QString clientInfo = QString("%1:%2").arg(newSocket->peerAddress().toString()).arg(newSocket->peerPort());

    // Если оба сокета уже заняты, отказываем в подключении
    if (makrolineSocket && plcSocket) {
        emit logMessage(QString("[Server] Отклонено подключение от %1 - максимальное количество клиентов достигнуто").arg(clientInfo));
        newSocket->disconnectFromHost();
        newSocket->deleteLater();
        return;
    }

    // Определяем, какой сокет назначить
    if (!makrolineSocket) {
        makrolineSocket = newSocket;
        emit logMessage(QString("[Server] Подключено ПО Makroline: %1").arg(clientInfo));
    } else if (!plcSocket) {
        plcSocket = newSocket;
        emit logMessage(QString("[Server] Подключен ПЛК: %1").arg(clientInfo));
    }

    connect(newSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(newSocket, &QTcpSocket::disconnected, this, &Server::onDisconnected);
    connect(newSocket, &QTcpSocket::bytesWritten, this, &Server::onBytesWritten);

    emit connectionChanged(true);
}

void Server::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QString clientInfo = QString("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());

    if (socket == makrolineSocket) {
        emit logMessage(QString("[Server] Отключено ПО Makroline: %1").arg(clientInfo));
        makrolineSocket = nullptr;
    } else if (socket == plcSocket) {
        emit logMessage(QString("[Server] Отключен ПЛК: %1").arg(clientInfo));
        plcSocket = nullptr;
    }

    socket->deleteLater();

    // Если оба отключились, меняем статус
    if (!makrolineSocket && !plcSocket) {
        emit connectionChanged(false);
    }
}

void Server::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray data = socket->readAll();
    QString clientType;

    if (socket == makrolineSocket) {
        clientType = "PO Makroline";
        emit commandReceived(data);
    } else if (socket == plcSocket) {
        clientType = "PLC";
        emit plcDataReceived(data); // Новый сигнал для данных от ПЛК
    }

    // Логирование
    QString hex;
    for (char byte : data) {
        hex += QString("%1 ").arg((quint8)byte, 2, 16, QLatin1Char('0')).toUpper();
    }

    QString text = QString::fromUtf8(data); // Более безопасное преобразование

    emit logMessage(QString("[Server] Получено от %1: %2 (HEX: %3)")
                        .arg(clientType, text, hex.trimmed()));
}

void Server::disconnectServer()
{
    if (server && server->isListening()) {
        server->close();
        emit logMessage("[Server] Сервер остановлен");
    }

    if (makrolineSocket) {
        makrolineSocket->disconnect(this);
        makrolineSocket->abort();
        makrolineSocket->deleteLater();
        makrolineSocket = nullptr;
    }

    if (plcSocket) {
        plcSocket->disconnect(this);
        plcSocket->abort();
        plcSocket->deleteLater();
        plcSocket = nullptr;
    }

    emit connectionChanged(false);
    emit logMessage("[Server] Все подключения закрыты");
}

void Server::ResponseMakroline(const QString &data_response)
{
    if (!makrolineSocket || makrolineSocket->state() != QTcpSocket::ConnectedState) {
        emit logMessage("[Server] Нет подключения к ПО Makroline для отправки ответа");
        return;
    }

    QByteArray data = data_response.toUtf8();
    makrolineSocket->write(data);
    emit logMessage(QString("[Server] Отправлено ПО Makroline: %1").arg(data_response));
}

void Server::ResponsePLC(const QByteArray &data)
{
    if (!plcSocket || plcSocket->state() != QTcpSocket::ConnectedState) {
        emit logMessage("[Server] Нет подключения к ПЛК для отправки ответа");
        return;
    }

    plcSocket->write(data);

    QString hex;
    for (char byte : data) {
        hex += QString("%1 ").arg((quint8)byte, 2, 16, QLatin1Char('0')).toUpper();
    }

    emit logMessage(QString("[Server] Отправлено ПЛК: HEX %1").arg(hex.trimmed()));
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
    if (queueOut.isEmpty()) {
        isProcessingOut = false;
        return;
    }

    isProcessingOut = true;
    QByteArray data = queueOut.takeFirst();

    // Отправляем данные соответствующему устройству
    // Здесь нужно определить, кому отправлять
}

void Server::onBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes)
    // Обработка отправки данных
}
