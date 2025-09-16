#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QAction>
#include <QStackedWidget>

// Предварительные объявления вместо включения constants.h
class HomePage;
class SettingsPage;
class CountersPage;
class LogsPage;
class Constants;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Изменяем конструктор - передаем страницы напрямую
    MainWindow(HomePage *homePage,
               SettingsPage *settingsPage,
               CountersPage *countersPage,
               LogsPage *logsPage,
               QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void changeStatusModes(bool checked);
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

    // Указатели на страницы (теперь они передаются извне)
    HomePage *m_homePage;
    SettingsPage *m_settingsPage;
    CountersPage *m_countersPage;
    LogsPage *m_logsPage;

signals:
};

#endif // MAINWINDOW_H
