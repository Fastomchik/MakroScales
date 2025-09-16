#include "appcontroller.h"
#include "bridgelinxtocab.h"
#include "clientsocket.h"
#include "serversocket.h"
#include "homepage.h"
#include "settingspage.h"
#include "counterspage.h"
#include "logspage.h"
#include "constants.h"

#include <QDebug>

AppController::AppController(QObject *parent)
    : QObject(parent),
    m_bridgeWorkerCab(nullptr),
    m_printerWorker(nullptr),
    m_serverWorker(nullptr),
    m_homePage(nullptr),
    m_settingsPage(nullptr),
    m_countersPage(nullptr),
    m_logsPage(nullptr),
    m_printerThread(nullptr),
    m_serverThread(nullptr)
{
}

AppController::~AppController()
{
    // Останавливаем и удаляем потоки
    if (m_printerThread && m_printerThread->isRunning()) {
        m_printerThread->quit();
        m_printerThread->wait();
    }

    if (m_serverThread && m_serverThread->isRunning()) {
        m_serverThread->quit();
        m_serverThread->wait();
    }

    // Удаляем объекты
    delete m_bridgeWorkerCab;
    delete m_printerWorker;
    delete m_serverWorker;
    delete m_homePage;
    delete m_settingsPage;
    delete m_countersPage;
    delete m_logsPage;
    delete m_printerThread;
    delete m_serverThread;
}

void AppController::initialize()
{
    initializePages();
    initializeThreads();
    initializeConnections();
    initializeSettings();
}

void AppController::initializeThreads()
{
    // Создаем рабочие объекты
    m_bridgeWorkerCab = new BridgeLinxtoCab();
    m_printerWorker = new ClientSocket();
    m_serverWorker = new Server();

    // Создаем потоки
    m_serverThread = new QThread(this);
    m_printerThread = new QThread(this);

    // Перемещаем объекты в потоки
    m_serverWorker->moveToThread(m_serverThread);
    m_printerWorker->moveToThread(m_printerThread);

    qDebug() << "Threads initialized:"
             << m_printerThread << m_serverThread
             << "Workers:" << m_printerWorker << m_bridgeWorkerCab << m_serverWorker;
}

void AppController::initializePages()
{
    m_homePage = new HomePage;
    m_settingsPage = new SettingsPage;
    m_countersPage = new CountersPage;
    m_logsPage = new LogsPage;
}

void AppController::initializeConnections()
{
    // Подключение сигналов уничтожения объектов к остановке потоков
    connect(m_serverWorker, &QObject::destroyed, m_serverThread, &QThread::quit);
    connect(m_printerWorker, &QObject::destroyed, m_printerThread, &QThread::quit);

    // Сигналы логов
    connect(m_printerWorker, &ClientSocket::logMessage, m_logsPage, &LogsPage::addLogMessage);
    connect(m_serverWorker, &Server::logMessage, m_logsPage, &LogsPage::addLogMessage);
    connect(m_bridgeWorkerCab, &BridgeLinxtoCab::logMessage, m_logsPage, &LogsPage::addLogMessage);


    connect(m_serverWorker, &Server::commandReceived, m_bridgeWorkerCab, &BridgeLinxtoCab::processLinxCommand);
    connect(m_serverWorker, &Server::plcDataReceived, m_printerWorker, &ClientSocket::receiveLastWeight);

    // Дополнительные подключения сигналов из homepage и других страниц
    // connect(m_homePage, &HomePage::someSignal, ...);
}

void AppController::initializeSettings()
{
    // Инициализация настроек (если необходимо)
}

void AppController::stopServer()
{
    // Реализация остановки сервера
}

void AppController::startServer()
{
    // Реализация запуска сервера
}

void AppController::startClient()
{
    // Реализация запуска клиента
}
