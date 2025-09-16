#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QMutex>

// Предварительные объявления классов
class BridgeLinxtoCab;
class ClientSocket;
class Server;
class HomePage;
class SettingsPage;
class CountersPage;
class LogsPage;
class Constants;

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    void initialize();

    // Геттеры для доступа к объектам
    HomePage* homePage() const { return m_homePage; }
    SettingsPage* settingsPage() const { return m_settingsPage; }
    CountersPage* countersPage() const { return m_countersPage; }
    LogsPage* logsPage() const { return m_logsPage; }
    BridgeLinxtoCab* bridgeWorkerCab() const { return m_bridgeWorkerCab; }
    ClientSocket* printerWorker() const { return m_printerWorker; }
    Server* serverWorker() const { return m_serverWorker; }

private:
    void initializeThreads();
    void initializePages();
    void initializeConnections();
    void initializeSettings();

    // Объекты
    BridgeLinxtoCab *m_bridgeWorkerCab;
    ClientSocket *m_printerWorker;
    Server *m_serverWorker;

    // Страницы UI
    HomePage *m_homePage;
    SettingsPage *m_settingsPage;
    CountersPage *m_countersPage;
    LogsPage *m_logsPage;

    // Потоки
    QThread* m_printerThread;
    QThread* m_serverThread;

private slots:
    void stopServer();
    void startServer();
    void startClient();
};

#endif // APPCONTROLLER_H
