#include "mainwindow.h"
#include <QMenuBar>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    constants = new Constants(this);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    this->setWindowIcon(QIcon(":/image/MakroScales.png"));
    this->setWindowTitle("MakroScales");

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
    stackedWidget->addWidget(constants->homePage);
    stackedWidget->addWidget(constants->settingsPage);
    stackedWidget->addWidget(constants->countersPage);
    stackedWidget->addWidget(constants->logsPage);
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


void changeStatusModes(bool checked)
{

}
