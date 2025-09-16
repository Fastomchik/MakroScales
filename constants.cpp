#include "constants.h"
#include "bridgelinxtocab.h"
#include "clientsocket.h"
#include "serversocket.h"


#include <QDebug>

QQueue<QByteArray> printQueue;       // Теперь переменная существует в памяти
QQueue<QString> makrolineQueue;      // Инициализируется по умолчанию
QMutex printQueueMutex;              // Мьютекс готов к использованию

Constants::Constants(QObject *parent)
    : QObject(parent),
    homePage(nullptr),
    settingsPage(nullptr),
    countersPage(nullptr),
    logsPage(nullptr)

{
    initializeThread();
    initializeSettings();
    initializePages();
    initializeSignal();
}

Constants::~Constants()
{
    delete homePage;
    delete settingsPage;
    delete countersPage;
    delete logsPage;
}

void Constants::initializeThread()
{
    bridgeWorkerCab = new BridgeLinxtoCab();
    printerWorker = new ClientSocket();
    serverWorker = new Server();

    serverThread = new QThread(this);
    printerThread = new QThread(this);

    serverWorker->moveToThread(serverThread);
    printerWorker->moveToThread(printerThread);
    qDebug() << " " <<  printerThread << "\t" << serverThread << "\n"
             << printerWorker << "\t" << bridgeWorkerCab << "\n"
             << serverWorker;
}

void Constants::initializeSettings()
{
}

void Constants::initializeSignal()
{
    // Потоки
    connect(serverWorker, &QObject::destroyed, serverThread, &QThread::quit);
    connect(printerWorker, &QObject::destroyed, printerThread, &QThread::quit);
    // connect(bridgeWorkerCab, &QObject::destroyed, serverThread, &QThread::quit);

    // Сигналы логов и оконных менеджеров
    connect(printerWorker, &ClientSocket::logMessage, logsPage, &LogsPage::addLogMessage);
    connect(serverWorker, &Server::logMessage, logsPage, &LogsPage::addLogMessage);
    connect(bridgeWorkerCab, &BridgeLinxtoCab::logMessage, logsPage, &LogsPage::addLogMessage);

    // Сигналы из homepage
    //connect(homePage, &HomePage::)
}

void Constants::initializePages()
{
    homePage = new HomePage;
    settingsPage = new SettingsPage;
    countersPage = new CountersPage;
    logsPage = new LogsPage;
}


