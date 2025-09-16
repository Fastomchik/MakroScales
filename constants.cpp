#include "constants.h"

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
    initializePages();
}

Constants::~Constants()
{
    delete homePage;
    delete settingsPage;
    delete countersPage;
    delete logsPage;
}

void Constants::initializePages()
{
    homePage = new HomePage;
    settingsPage = new SettingsPage;
    countersPage = new CountersPage;
    logsPage = new LogsPage;
}
