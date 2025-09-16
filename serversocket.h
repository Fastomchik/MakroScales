#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QDataStream>

class Server : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    ~Server() override;

public slots:
    void doWork();
    void disconnectServer();
    void setConnectionParams(const QString &ip, const QString &port);
    void ResponseMakroline(const QString &data_response);
    void ResponsePLC(const QByteArray &data); // Добавим параметр

signals:
    void connectionChanged(bool connected);
    void logMessage(const QString &message);
    void commandReceived(const QByteArray &command);
    void plcDataReceived(const QByteArray &data); // Новый сигнал для данных от ПЛК

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    void processNext();
    void processNextOut();
    void onBytesWritten(qint64 bytes);

private:
    QTcpServer *server;
    QTcpSocket *makrolineSocket; // Сокет для ПО Makroline
    QTcpSocket *plcSocket;       // Сокет для ПЛК
    QString server_ip;
    QString server_port;
    QList<QByteArray> queue;
    QList<QByteArray> queueOut;
    bool isPrinting;
    bool isProcessingOut;

    // Метод для идентификации устройств
    QString identifyDevice(const QByteArray &data);
};

#endif // SERVERSOCKET_H
