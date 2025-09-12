#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QStackedWidget>

// Предварительное объявление классов (чтобы не включать заголовки)
class HomePage;
class SettingsPage;
class CountersPage;
class LogsPage;

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

    // Действия меню
    QAction *actionHome;
    QAction *actionSettings;
    QAction *actionCounters;
    QAction *actionLogs;

    // Виджет для переключения страниц
    QStackedWidget *stackedWidget;

    // Указатели на страницы
    HomePage *homePage;
    SettingsPage *settingsPage;
    CountersPage *countersPage;
    LogsPage *logsPage;
};

#endif // MAINWINDOW_H
