#include "serversocket.h"
#include <QNetworkInterface>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QDataStream>

Server::Server(QObject* parent)
    : QObject(parent),
    makrolineServer(nullptr),
    plcServer(nullptr),
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
    emit logMessage(QString("[Server] Параметры подключения: %1 - ПО: %2, ПЛК: %3")
                        .arg(ip, port, QString::number(plc_port)));
}

void Server::startServer()
{
    qDebug() << "Запуск серверов в потоке:" << QThread::currentThread();

    // Останавливаем серверы если они уже запущены
    disconnectServer();

    // Создаем сервер для ПО Makroline
    if (!makrolineServer) {
        makrolineServer = new QTcpServer(this);
        connect(makrolineServer, &QTcpServer::newConnection, this, &Server::onNewMakrolineConnection);
    }

    // Создаем сервер для ПЛК (статический порт 8090)
    if (!plcServer) {
        plcServer = new QTcpServer(this);
        connect(plcServer, &QTcpServer::newConnection, this, &Server::onNewPlcConnection);
    }

    // Запускаем сервер для ПО Makroline
    bool ok;
    quint16 makrolinePort = server_port.toUShort(&ok);
    if (!ok || makrolinePort == 0) {
        emit logMessage("[Server] Неверный порт для ПО Makroline");
        return;
    }

    if (!makrolineServer->listen(QHostAddress(server_ip), makrolinePort)) {
        emit logMessage(QString("[Server] Ошибка запуска сервера ПО Makroline: %1").arg(makrolineServer->errorString()));
    } else {
        emit logMessage(QString("[Server] Сервер ПО Makroline запущен на %1:%2").arg(server_ip).arg(makrolinePort));
    }

    // Запускаем сервер для ПЛК на статическом порту 8090
    if (!plcServer->listen(QHostAddress(server_ip), plc_port)) {
        emit logMessage(QString("[Server] Ошибка запуска сервера ПЛК: %1").arg(plcServer->errorString()));
    } else {
        emit logMessage(QString("[Server] Сервер ПЛК запущен на %1:%2").arg(server_ip).arg(plc_port));
    }
}

void Server::onNewMakrolineConnection()
{
    if (makrolineSocket) {
        QTcpSocket *newSocket = makrolineServer->nextPendingConnection();
        QString clientInfo = QString("%1:%2").arg(newSocket->peerAddress().toString()).arg(newSocket->peerPort());
        emit logMessage(QString("[Server] Отклонено подключение ПО Makroline от %1 - уже подключено").arg(clientInfo));
        newSocket->disconnectFromHost();
        newSocket->deleteLater();
        return;
    }

    makrolineSocket = makrolineServer->nextPendingConnection();
    QString clientInfo = QString("%1:%2").arg(makrolineSocket->peerAddress().toString()).arg(makrolineSocket->peerPort());

    emit logMessage(QString("[Server] Подключено ПО Makroline: %1").arg(clientInfo));

    connect(makrolineSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(makrolineSocket, &QTcpSocket::disconnected, this, &Server::onDisconnected);
    connect(makrolineSocket, &QTcpSocket::bytesWritten, this, &Server::onBytesWritten);

    emit connectionChanged(true);
}

void Server::onNewPlcConnection()
{
    if (plcSocket) {
        QTcpSocket *newSocket = plcServer->nextPendingConnection();
        QString clientInfo = QString("%1:%2").arg(newSocket->peerAddress().toString()).arg(newSocket->peerPort());
        emit logMessage(QString("[Server] Отклонено подключение ПЛК от %1 - уже подключено").arg(clientInfo));
        newSocket->disconnectFromHost();
        newSocket->deleteLater();
        return;
    }

    plcSocket = plcServer->nextPendingConnection();
    QString clientInfo = QString("%1:%2").arg(plcSocket->peerAddress().toString()).arg(plcSocket->peerPort());

    emit logMessage(QString("[Server] Подключен ПЛК: %1").arg(clientInfo));

    connect(plcSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(plcSocket, &QTcpSocket::disconnected, this, &Server::onDisconnected);
    connect(plcSocket, &QTcpSocket::bytesWritten, this, &Server::onBytesWritten);

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
        // Логирование
        QString hex;
        for (char byte : data)
            hex += QString("%1 ").arg((quint8)byte, 2, 16, QLatin1Char('0')).toUpper();
        QString text = QString::fromUtf16(reinterpret_cast<const char16_t*>(data.constData()), data.size() / 2);

        emit logMessage(QString("[Server] Получено от %1: %2 (HEX: %3)")
                            .arg(clientType, text, hex.trimmed()));
        emit commandReceived(data);
    } else if (socket == plcSocket) {
        clientType = "PLC";
        // Логирование
        QString hex;
        for (char byte : data)
            hex += QString("%1 ").arg((quint8)byte, 2, 16, QLatin1Char('0')).toUpper();
        QString text = QString::fromUtf8(data);

        emit logMessage(QString("[Server] Получено от %1: %2 (HEX: %3)")
                            .arg(clientType, text, hex.trimmed()));
        emit plcDataReceived(data);
    }

}

void Server::ResponseMakroline(const QByteArray &response)
{
    if (!makrolineSocket || makrolineSocket->state() != QTcpSocket::ConnectedState) {
        emit logMessage("[Server] Нет подключения к ПО Makroline для отправки ответа");
        return;
    }

    makrolineSocket->write(response);

    QString hex;
    for (char byte : response) {
        hex += QString("%1 ").arg((quint8)byte, 2, 16, QLatin1Char('0')).toUpper();
    }

    emit logMessage(QString("[Server] Отправлено ПО Makroline: HEX %1").arg(hex.trimmed()));
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

void Server::disconnectServer()
{
    if (makrolineServer && makrolineServer->isListening()) {
        makrolineServer->close();
    }

    if (plcServer && plcServer->isListening()) {
        plcServer->close();
    }

    if (makrolineSocket) {
        makrolineSocket->disconnect();
        makrolineSocket->abort();
        makrolineSocket->deleteLater();
        makrolineSocket = nullptr;
    }

    if (plcSocket) {
        plcSocket->disconnect();
        plcSocket->abort();
        plcSocket->deleteLater();
        plcSocket = nullptr;
    }

    emit connectionChanged(false);
    emit logMessage("[Server] Все серверы остановлены, подключения закрыты");
}

// Остальные методы остаются без изменений...
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

    // Определяем кому отправлять данные
    if (isMakrolineData(data) && makrolineSocket && makrolineSocket->state() == QTcpSocket::ConnectedState) {
        makrolineSocket->write(data);
    } else if (isPlcData(data) && plcSocket && plcSocket->state() == QTcpSocket::ConnectedState) {
        plcSocket->write(data);
    }
}

void Server::onBytesWritten(qint64)
{
    if (!queueOut.isEmpty()) processNextOut();
    else isProcessingOut = false;
}

bool Server::isMakrolineData(const QByteArray &data)
{
    // Логика определения данных для ПО Makroline
    QString text = QString::fromUtf8(data);
    return text.contains("PRINT") || text.contains("STATE") ||
           text.contains("QUEUE") || text.contains("JOB");
}

bool Server::isPlcData(const QByteArray &data)
{
    // Логика определения данных для ПЛК
    // Пример: если данные содержат специфичные для ПЛК маркеры
    return data.size() >= 2 &&
           static_cast<quint8>(data[0]) == 0x01 &&
           static_cast<quint8>(data[1]) == 0x02;
}
