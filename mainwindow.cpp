#include "mainwindow.h"
#include "homepage.h"
#include "settingspage.h"
#include "counterspage.h"
#include "logspage.h"
#include "constants.h" // Теперь только для enum, если нужно
#include <QMenuBar>
#include <QIcon>

MainWindow::MainWindow(HomePage *homePage,
                       SettingsPage *settingsPage,
                       CountersPage *countersPage,
                       LogsPage *logsPage,
                       QWidget *parent)
    : QMainWindow(parent),
    m_homePage(homePage),
    m_settingsPage(settingsPage),
    m_countersPage(countersPage),
    m_logsPage(logsPage)
{
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    this->setWindowIcon(QIcon(":/image/MakroScales.png"));
    this->setWindowTitle("MakroScales");

    // Убираем создание constants, так как он больше не управляет страницами
    // constants = new Constants(this); // УДАЛИТЬ

    setupPages();
    createActions();
    createMenus();
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupPages()
{
    // Добавляем страницы в stacked widget
    if (m_homePage) stackedWidget->addWidget(m_homePage);
    if (m_settingsPage) stackedWidget->addWidget(m_settingsPage);
    if (m_countersPage) stackedWidget->addWidget(m_countersPage);
    if (m_logsPage) stackedWidget->addWidget(m_logsPage);
}

void MainWindow::createActions()
{
    actionHome = new QAction("&Главная", this);
    connect(actionHome, &QAction::triggered, this, &MainWindow::showHome);

    actionSettings = new QAction("&Настройки", this);
    connect(actionSettings, &QAction::triggered, this, &MainWindow::showSettings);

    actionCounters = new QAction("&Счётчики", this);
    connect(actionCounters, &QAction::triggered, this, &MainWindow::showCounters);

    actionLogs = new QAction("&Логи", this);
    connect(actionLogs, &QAction::triggered, this, &MainWindow::showLogs);
}

void MainWindow::createMenus()
{
    menuBar()->addAction(actionHome);
    menuBar()->addAction(actionSettings);
    menuBar()->addAction(actionCounters);
    menuBar()->addAction(actionLogs);
}

void MainWindow::showHome()
{
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::showSettings()
{
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::showCounters()
{
    stackedWidget->setCurrentIndex(2);
}

void MainWindow::showLogs()
{
    stackedWidget->setCurrentIndex(3);
}

// Эта функция должна быть методом класса
void MainWindow::changeStatusModes(bool checked)
{
    // Реализация функции
    Q_UNUSED(checked);
}
