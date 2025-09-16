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

class bridgelinxtocab : public QObject
{
    Q_OBJECT
public:
    explicit bridgelinxtocab(QObject *parent = nullptr);
signals:
    void commandToPrinter(const QByteArray &docodCommand, Constants::TypeCommandCab commandtype);
    void responseToMakroline(const QByteArray &response);
    void updateSpinBox(Constants::SpinBoxType type, int value);

public slots:
    void manualPrint();
    void changeAutoAndManualModes(bool checked);
    void processLinxCommand(const QByteArray &command);
    void updateAllCount();
    // Oт AnswerDocod.h
    void handlePrinterState(Constants::CabState);

private:
    void autoPrinted();
    //Обьекты
    QTimer* m_updateTimer;
    QWaitCondition m_queueCondition;
    QMutex m_queueMutex;
    // Вспомогательные методы
    QByteArray generateCommandFromTemplate(); // Преобразование команды печати из софта на целевой принтер
    bool isValidStateTransition(Constants::LinxState current, Constants::LinxState target) const;
    void processPrintQueue();
    // ========================================================
    // Обработчики команд Linx TTO (emulator)
    // ========================================================
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

    // Геттеры
    QByteArray getConvertStringToByte(const QString& string);
    QString getErrorState() const;
    QString getCurrentJob() const;

};


#endif // BRIDGELINXTODOCOD_H
