#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H
#include "constants.h"
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QList>


class ClientSocket : public QObject
{
    Q_OBJECT
public:
    explicit ClientSocket(QObject *parent = nullptr);
    ~ClientSocket();

public slots:
    void doWork();
    void setConnectionParams(const QString &ip, const QString &port);
    void disconnectSocket();
    void connectToServer();
    void disconnectFromServer();
    void sendCommandPrinter(const QByteArray &command, const Constants::TypeCommandCab commandtype);

private slots:
    void onConnected();
    void onErrorOccurred();

signals:
    void connectionChanged(bool checked);
    void logMessage(const QString& message);
    void successfulPrintedInMakroline(const QByteArray &response);

private:
    bool isConnected;
    QTcpSocket* socket;
    QString printer_ip;
    QString printer_port;
    QList<QString> listWeight;

    // Вспомогательные методы
    void fillPrinterBuffer();
    void clearBuffers();
    void startPrint();
    void stopPrint();
    void requestStatus();

};

#endif // CLIENTSOCKET_H
