#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "constants.h"
#include <QMainWindow>
#include <QThread>
#include <QAction>
#include <QStackedWidget>

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
    Constants *constants;

    QThread *clientSocket;
    QThread *serverSocket;

signals:
    void changeStatusModes(bool checked);
};

#endif // MAINWINDOW_H
