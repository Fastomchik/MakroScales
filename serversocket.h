#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject* parent = nullptr);
    ~Server();


signals:
    void logMessage(const QString &message);
    void connectionChanged(bool connected);
    void commandReceived(const QByteArray &command); // Команды от ПО Makroline
    void plcDataReceived(const QByteArray &data);    // Данные от ПЛК

public slots:
    void ResponseMakroline(const QByteArray &response);
    void ResponsePLC(const QByteArray &data);
    void startServer();
    void setConnectionParams(const QString &ip, const QString &port); // Для ПО Makroline

private slots:
    void onNewMakrolineConnection();
    void onNewPlcConnection();
    void onReadyRead();
    void onDisconnected();
    void onBytesWritten(qint64 bytes);

private:
    void disconnectServer();
    void processNext();
    void processNextOut();
    bool isMakrolineData(const QByteArray &data);
    bool isPlcData(const QByteArray &data);

    QTcpServer *makrolineServer; // Сервер для ПО Makroline
    QTcpServer *plcServer;       // Сервер для ПЛК (порт 8090 статический)
    QTcpSocket *makrolineSocket;
    QTcpSocket *plcSocket;

    QString server_ip;
    QString server_port; // Порт для ПО Makroline
    int plc_port = 9090; // Статический порт для ПЛК

    bool isPrinting;
    bool isProcessingOut;
    QList<QByteArray> queue;
    QList<QByteArray> queueOut;
};

#endif // SERVERSOCKET_H
