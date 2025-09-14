#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H
#include <QObject>
#include <QString>
#include <QTcpSocket>


class ClientSocket : public QObject
{
    Q_OBJECT
public:
    explicit ClientSocket(QObject *parent = nullptr);

public slots:
    void doWork();
    void setConnectionParams(const QString &ip, const QString &port);
    void disconnectSocket();
    void receiveCommandPrinter();

private slots:
    void onConnected();
    void onErrorOccurred();

signals:
    void connectionChanged(bool checked);
    void logMessage(const QString& message);

private:
    bool isConnected;
    QTcpSocket* socket;
    QString printer_ip;
    QString printer_port;

};

#endif // CLIENTSOCKET_H
