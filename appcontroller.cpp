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

    if (m_bridgeThread && m_bridgeThread->isRunning()){
        m_bridgeThread->quit();
        m_bridgeThread->wait();
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
    delete m_bridgeThread;
}

void AppController::initialize()
{
    initializePages();      // 1. Создаем UI страницы
    loadSettings();         // 2. Загружаем настройки

    // 3. Создаем рабочие объекты (пока в главном потоке)
    m_bridgeWorkerCab = new BridgeLinxtoCab();
    m_printerWorker = new ClientSocket();
    m_serverWorker = new Server();

    // 4. Создаем потоки и перемещаем объекты
    initializeThreads();

    // 5. Применяем настройки к объектам (они уже в своих потоках)
    applySettings();

    // 6. Подключаем сигналы
    initializeConnections();
}

void AppController::initializeThreads()
{
    // Создаем потоки
    m_serverThread = new QThread(this);
    m_printerThread = new QThread(this);
    m_bridgeThread = new QThread(this);

    // Перемещаем объекты в потоки
    m_serverWorker->moveToThread(m_serverThread);
    m_printerWorker->moveToThread(m_printerThread);
    m_bridgeWorkerCab->moveToThread(m_bridgeThread);

    // Запускаем потоки
    m_serverThread->start();
    m_printerThread->start();
    m_bridgeThread->start();

    qDebug() << "После перемещения:";

    qDebug() << "Server worker thread:" << m_serverWorker->thread();
    qDebug() << "Server thread:" << m_serverThread;
    qDebug() << "Equal:" << (m_serverWorker->thread() == m_serverThread);

    qDebug() << "Printer worker thread:" << m_printerWorker->thread();
    qDebug() << "Printer thread:" << m_printerThread;
    qDebug() << "Equal:" << (m_printerWorker->thread() == m_printerThread);

    qDebug() << "Bridge worker thread:" << m_bridgeWorkerCab->thread();
    qDebug() << "Bridge thread:" << m_bridgeThread;
    qDebug() << "Equal:" << (m_bridgeWorkerCab->thread() == m_bridgeThread);
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
    connect(m_bridgeWorkerCab, &QObject::destroyed, m_bridgeThread, &QThread::quit);

    // Сигналы логов
    connect(m_printerWorker, &ClientSocket::logMessage, m_logsPage, &LogsPage::addLogMessage, Qt::QueuedConnection);
    connect(m_serverWorker, &Server::logMessage, m_logsPage, &LogsPage::addLogMessage, Qt::QueuedConnection);
    connect(m_bridgeWorkerCab, &BridgeLinxtoCab::logMessage, m_logsPage, &LogsPage::addLogMessage, Qt::QueuedConnection);


    // Сигналы получения данных от makroline и plc
    connect(m_serverWorker, &Server::commandReceived, m_bridgeWorkerCab, &BridgeLinxtoCab::processLinxCommand,
            Qt::QueuedConnection);
    connect(m_serverWorker, &Server::plcDataReceived, m_bridgeWorkerCab, &BridgeLinxtoCab::setWeightFromPLC,
            Qt::QueuedConnection);

    // Сигналы старта и стопа сервера, клиента
    connect(m_homePage, &HomePage::startServerRequested, this, &AppController::startServer);
    connect(m_homePage, &HomePage::startClientRequested, this, &AppController::startClient);
    connect(m_homePage, &HomePage::stopServerRequested, this, &AppController::stopServer);
    connect(m_homePage, &HomePage::stopClientRequested, this, &AppController::stopClient);

    // Сигналы обновления интерфейса при измении статуса
    connect(m_serverWorker, &Server::connectionChanged, m_homePage, &HomePage::setServerStatus, Qt::QueuedConnection);
    connect(m_printerWorker, &ClientSocket::connectionChanged, m_homePage, &HomePage::setClientStatus, Qt::QueuedConnection);
    connect(m_bridgeWorkerCab, &BridgeLinxtoCab::CheckServerStatus, m_homePage, &HomePage::getServerStatus, Qt::QueuedConnection);
    connect(m_bridgeWorkerCab , &BridgeLinxtoCab::CheckClientStatus, m_homePage, &HomePage::getClientStatus, Qt::QueuedConnection);
    connect(m_homePage, &HomePage::clientStatusChanged, m_bridgeWorkerCab, &BridgeLinxtoCab::updateClientStatus, Qt::QueuedConnection);
    connect(m_homePage, &HomePage::serverStatusChanged, m_bridgeWorkerCab, &BridgeLinxtoCab::updateServerStatus, Qt::QueuedConnection);
    connect(m_bridgeWorkerCab, &BridgeLinxtoCab::updateDisplayWeightCounter, m_countersPage, &CountersPage::setLastWeight, Qt::QueuedConnection);
    connect(m_bridgeWorkerCab, &BridgeLinxtoCab::updateDisplayBufferCodesCount, m_countersPage, &CountersPage::setBufferCodesCount, Qt::QueuedConnection);
    connect(m_printerWorker, &ClientSocket::updateDisplayPrintedCounter, m_countersPage, &CountersPage::setCountPrintedCounter, Qt::QueuedConnection);
    connect(m_bridgeWorkerCab, &BridgeLinxtoCab::updateDisplayTotalCountCounter, m_countersPage, &CountersPage::setTotalCountСounter, Qt::QueuedConnection);

    // Добавляем подключение сигнала сохранения настроек
    connect(m_settingsPage, &SettingsPage::settingsSaved, this, &AppController::onSettingsSaved);

    // Сигналы из bridgelinxtocab
    connect(m_bridgeWorkerCab, &BridgeLinxtoCab::commandToPrinter, m_printerWorker, &ClientSocket::sendCommandPrinter, Qt::QueuedConnection);
    connect(m_bridgeWorkerCab, &BridgeLinxtoCab::responseToMakroline, m_serverWorker, &Server::ResponseMakroline, Qt::QueuedConnection);

    // Сигнал об отпечатке
    connect(m_printerWorker, &ClientSocket::successfulPrintedInMakroline, m_serverWorker, &Server::ResponseMakroline, Qt::QueuedConnection);
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
    QMetaObject::invokeMethod(m_serverWorker, "disconnectServer", Qt::QueuedConnection);

    // ОБНОВЛЯЕМ СТАТУС ЧЕРЕЗ СИГНАЛ, а не напрямую
    // (если homePage в другом потоке)
    QMetaObject::invokeMethod(m_homePage, "setServerStatus",
                              Qt::QueuedConnection, Q_ARG(bool, false));
}

void AppController::startServer()
{
    qDebug() << "Запуск сервера..." << m_serverIp << ":" << m_serverPort;

    if (!m_serverThread->isRunning()) {
        m_serverThread->start();
        qDebug() << "Поток сервера запущен";

        // АСИНХРОННАЯ задержка вместо блокирующего sleep
        QTimer::singleShot(100, this, [this]() {
            QMetaObject::invokeMethod(m_serverWorker, "startServer", Qt::QueuedConnection);
        });
    } else {
        QMetaObject::invokeMethod(m_serverWorker, "startServer", Qt::QueuedConnection);
    }
}

void AppController::startClient()
{
    qDebug() << "Запуск клиента..." << m_clientIp << ":" << m_clientPort;

    if (!m_printerThread->isRunning()) {
        m_printerThread->start();
        qDebug() << "Поток клиента запущен";
    }

    // ПЕРЕНЕСТИ setConnectionParams в поток клиента
    QMetaObject::invokeMethod(m_printerWorker, "setConnectionParams",
                              Qt::QueuedConnection,
                              Q_ARG(QString, m_clientIp),
                              Q_ARG(QString, QString::number(m_clientPort)));

    // АСИНХРОННЫЙ запуск после задержки
    QTimer::singleShot(100, this, [this]() {
        QMetaObject::invokeMethod(m_printerWorker, "connectToServer", Qt::QueuedConnection);
    });
}

void AppController::stopClient()
{
    qDebug() << "Остановка клиента...";

    // Отключаем клиента в его потоке
    QMetaObject::invokeMethod(m_printerWorker, "disconnectFromServer", Qt::QueuedConnection);

    // ОБНОВЛЯЕМ СТАТУС ЧЕРЕЗ СИГНАЛ
    QMetaObject::invokeMethod(m_homePage, "setClientStatus",
                              Qt::QueuedConnection, Q_ARG(bool, false));
}
