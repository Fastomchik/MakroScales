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
    initializePages();      // 1. Создаем UI страницы
    loadSettings();         // 2. Загружаем настройки

    // 3. Создаем рабочие объекты (пока в главном потоке)
    m_bridgeWorkerCab = new BridgeLinxtoCab();
    m_printerWorker = new ClientSocket();
    m_serverWorker = new Server();

    // 4. Применяем настройки к объектам (они еще в главном потоке)
    applySettings();

    // 5. Создаем потоки и перемещаем объекты
    initializeThreads();

    // 6. Подключаем сигналы
    initializeConnections();
}

void AppController::initializeThreads()
{
    // Создаем потоки
    m_serverThread = new QThread(this);
    m_printerThread = new QThread(this);

    qDebug() << "Созданы рабочие объекты:";
    qDebug() << "  BridgeLinxtoCab thread:" << m_bridgeWorkerCab->thread();
    qDebug() << "  ClientSocket thread:" << m_printerWorker->thread();
    qDebug() << "  Server thread:" << m_serverWorker->thread();

    // Перемещаем объекты в потоки
    m_serverWorker->moveToThread(m_serverThread);
    m_printerWorker->moveToThread(m_printerThread);

    qDebug() << "После перемещения в потоки:";
    qDebug() << "  BridgeLinxtoCab thread:" << m_bridgeWorkerCab->thread();
    qDebug() << "  ClientSocket thread:" << m_printerWorker->thread();
    qDebug() << "  Server thread:" << m_serverWorker->thread();

    // Запускаем потоки
    m_serverThread->start();
    m_printerThread->start();

    qDebug() << "Потоки запущены";
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

    // Сигналы получения данных от makroline и plc
    connect(m_serverWorker, &Server::commandReceived, m_bridgeWorkerCab, &BridgeLinxtoCab::processLinxCommand,
            Qt::QueuedConnection);
    connect(m_serverWorker, &Server::plcDataReceived, m_printerWorker, &ClientSocket::receiveLastWeight,
            Qt::QueuedConnection);

    // Сигналы старта и стопа сервера, клиента
    connect(m_homePage, &HomePage::startServerRequested, this, &AppController::startServer);
    connect(m_homePage, &HomePage::startClientRequested, this, &AppController::startClient);

    // Сигналы обновления интерфейса при измении статуса
    connect(m_serverWorker, &Server::connectionChanged, m_homePage, &HomePage::setServerStatus);
    connect(m_printerWorker, &ClientSocket::connectionChanged, m_homePage, &HomePage::setClientStatus);

    // Добавляем подключение сигнала сохранения настроек
    connect(m_settingsPage, &SettingsPage::settingsSaved, this, &AppController::onSettingsSaved);

    // Сигналы из bridgelinxtocab
    connect(m_bridgeWorkerCab, &BridgeLinxtoCab::commandToPrinter, m_printerWorker, &ClientSocket::sendCommandPrinter,
            Qt::QueuedConnection);
    connect(m_bridgeWorkerCab, &BridgeLinxtoCab::responseToMakroline, m_serverWorker, &Server::ResponseMakroline,
            Qt::QueuedConnection);

}

void AppController::loadSettings()
{
    QSettings settings("Makro", "MakroScales");

    m_serverIp = settings.value("server/ip", "192.168.1.100").toString();
    m_serverPort = settings.value("server/port", 8080).toInt();
    m_clientIp = settings.value("client/ip", "192.168.1.200").toString();
    m_clientPort = settings.value("client/port", 9100).toInt();

    qDebug() << "Загружены настройки:"
             << "Server:" << m_serverIp << ":" << m_serverPort
             << "Client:" << m_clientIp << ":" << m_clientPort;
}

void AppController::applySettings()
{
    qDebug() << "Применение настроек к рабочим объектам...";

    // Применяем настройки к серверу через invokeMethod
    if (m_serverWorker) {
        QMetaObject::invokeMethod(m_serverWorker, "setConnectionParams",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, m_serverIp),
                                  Q_ARG(QString, QString::number(m_serverPort)));
        qDebug() << "Настройки сервера отправлены:" << m_serverIp << ":" << m_serverPort;
    }

    // Применяем настройки к клиенту через invokeMethod
    if (m_printerWorker) {
        QMetaObject::invokeMethod(m_printerWorker, "setConnectionParams",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, m_clientIp),
                                  Q_ARG(QString, QString::number(m_clientPort)));
        qDebug() << "Настройки клиента отправлены:" << m_clientIp << ":" << m_clientPort;
    }
}

void AppController::onSettingsSaved()
{
    // Обновляем настройки при сохранении
    m_serverIp = m_settingsPage->getServerIp();
    m_serverPort = m_settingsPage->getServerPort();
    m_clientIp = m_settingsPage->getClientIp();
    m_clientPort = m_settingsPage->getClientPort();

    // Применяем новые настройки
    applySettings();

    qDebug() << "Настройки обновлены:"
             << "Server:" << m_serverIp << ":" << m_serverPort
             << "Client:" << m_clientIp << ":" << m_clientPort;
}

void AppController::stopServer()
{
    qDebug() << "Остановка сервера...";

    // Останавливаем сервер в его потоке
    QMetaObject::invokeMethod(m_serverWorker, "stopServer", Qt::QueuedConnection);

    // Обновляем статус на домашней странице
    m_homePage->setServerStatus(false);
}

void AppController::startServer()
{
    qDebug() << "Запуск сервера..." << m_serverIp << ":" << m_serverPort;

    if (!m_serverThread->isRunning()) {
        m_serverThread->start();
        qDebug() << "Поток сервера запущен";

        // Даем время потоку инициализироваться
        QThread::msleep(100);
    }

    // Запускаем сервер в его потоке
    QMetaObject::invokeMethod(m_serverWorker, "startServer", Qt::QueuedConnection);
}

void AppController::startClient()
{
    qDebug() << "Запуск клиента..." << m_clientIp << ":" << m_clientPort;

    if (!m_printerThread->isRunning()) {
        m_printerThread->start();
        qDebug() << "Поток клиента запущен";
    }
        QThread::msleep(100);

    // Убеждаемся, что настройки применены перед подключением
    if (m_printerWorker) {
        m_printerWorker->setConnectionParams(m_clientIp, QString::number(m_clientPort));
    }
    // Запускаем клиента в его потоке
    QMetaObject::invokeMethod(m_printerWorker, "connectToServer", Qt::QueuedConnection);
}

void AppController::stopClient()
{
    qDebug() << "Остановка клиента...";

    // Отключаем клиента в его потоке
    QMetaObject::invokeMethod(m_printerWorker, "disconnectFromServer", Qt::QueuedConnection);

    // Обновляем статус на домашней странице
    m_homePage->setClientStatus(false);
}
