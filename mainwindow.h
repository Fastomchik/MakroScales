#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "counterspage.h"
#include "homepage.h"
#include "logspage.h"
#include "settingspage.h"
#include <QMainWindow>
#include <QThread>
#include <QAction>
#include <QStackedWidget>
#include <QIcon>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showHome();
    void showSettings();
    void showCounters();
    void showLogs();

private:
    void createMenus();
    void createActions();
    void setupPages();

    QAction *actionHome;
    QAction *actionSettings;
    QAction *actionCounters;
    QAction *actionLogs;

    QStackedWidget *stackedWidget;

    HomePage *homePage;
    SettingsPage *settingsPage;
    CountersPage *countersPage;
    LogsPage *logsPage;

    QThread *clientSocket;
    QThread *serverSocket;

signals:
    void changeStatusModes(bool checked);
};

#endif // MAINWINDOW_H
