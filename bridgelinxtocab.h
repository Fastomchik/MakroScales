#ifndef BRIDGELINXTOCAB_H
#define BRIDGELINXTOCAB_H

#include "constants.h"
#include <QObject>
#include <QByteArray>
#include <QSettings>
#include <QDate>
#include <QTimer>
#include <QDebug>
#include <QRegularExpression>
#include <QWaitCondition>
#include <QMutex>
#include <QQueue>
#include <QHash>

class BridgeLinxtoCab : public QObject
{
    Q_OBJECT
public:
    explicit BridgeLinxtoCab(QObject *parent = nullptr);
    ~BridgeLinxtoCab();

signals:
    void commandToPrinter(const QByteArray &docodCommand, Constants::TypeCommandCab commandtype);
    void responseToMakroline(const QByteArray &response);
    void logMessage(const QString &message);
    void updateDisplayWeightCounter(float lastWeight);
    void updateDisplayBufferCodesCount(int codesInBuffer);
    void updateDisplayTotalCountCounter(int count);
    void CheckServerStatus(bool status);
    void CheckClientStatus(bool status);
    //void updateSpinBox(Constants::SpinBoxType type, int value);

public slots:
    void manualPrint();
    void changeAutoAndManualModes(bool checked);
    void processLinxCommand(const QByteArray &command);
    //void updateAllCount();
    void handlePrinterState(Constants::CabState);
    void setWeightFromPLC(const QByteArray &data);

    void updateClientStatus(bool connected);
    void updateServerStatus(bool connected);

private:
    void autoPrinted();
    void processPrintQueue();
    bool isValidStateTransition(Constants::LinxState current, Constants::LinxState target) const;

    // Обработчики команд Linx TTO
    void handleRequestState();
    void handleRequestQueueSize();
    void handlePrint();
    void handleSelectJob(const QStringList &parts);
    void handleRequestAsyncState();
    void handleClearFaults();
    void handleAddToBuffer(const QStringList &parts);
    void handleAddToQueue(const QStringList &parts);
    void handleSetState(const QStringList &parts);
    void handleClearQueue();
    void handleUnknownCommand();
    void handleUpdateJobNamed(const QStringList &parts);

    // Трансформация Linx → CAB
    QByteArray transformLinxToCab(const QString &linxCommand);

    // Вспомогательные методы
    QByteArray getConvertStringToByte(const QString& string);
    QString getErrorState() const;
    QString getCurrentJob() const;

    QTimer* m_updateTimer;
    QWaitCondition m_queueCondition;
    QMutex m_queueMutex;
    QQueue<QByteArray> pendingCabQueue;    // Отложенная команда, ждет вес
    QQueue<QString> makrolineQueue;

    QHash<QString, QString> lastValues;
    QString lastRawCode;

    bool m_clientConnected;
    bool m_serverConnected;
};

#endif // BRIDGELINXTOCAB_H
