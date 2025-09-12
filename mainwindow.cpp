#include "mainwindow.h"
#include "homepage.h"
#include "settingspage.h"
#include "counterspage.h"
#include "logspage.h"
#include <QMenuBar>
#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setupPages();
    createActions();
    createMenus();

    setWindowTitle("Главная - Мониторинг системы");
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupPages()
{
    // Создаем страницы
    homePage = new HomePage;
    settingsPage = new SettingsPage;
    countersPage = new CountersPage;
    logsPage = new LogsPage;

    // Добавляем страницы в stacked widget
    stackedWidget->addWidget(homePage);
    stackedWidget->addWidget(settingsPage);
    stackedWidget->addWidget(countersPage);
    stackedWidget->addWidget(logsPage);
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
    setWindowTitle("Главная - Мониторинг системы");
}

void MainWindow::showSettings()
{
    stackedWidget->setCurrentIndex(1);
    setWindowTitle("Настройки - Мониторинг системы");
}

void MainWindow::showCounters()
{
    stackedWidget->setCurrentIndex(2);
    setWindowTitle("Счётчики - Мониторинг системы");
}

void MainWindow::showLogs()
{
    stackedWidget->setCurrentIndex(3);
    setWindowTitle("Логи - Мониторинг системы");
}
