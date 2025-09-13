#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class HomePage : public QWidget
{
    Q_OBJECT
public:
    explicit HomePage(QWidget *parent = nullptr);

    // Методы для обновления статуса
    void setServerStatus(bool connected);
    void setClientStatus(bool connected);
    void resetStatus();

private:
    void setupUI();
    void updateStatusDisplays();

    // Кнопки
    QPushButton *btnStartServer;
    QPushButton *btnConnectClient;

    // Кружки статуса
    QLabel *serverStatusCircle;
    QLabel *clientStatusCircle;

    // Подписи
    QLabel *serverLabel;
    QLabel *clientLabel;

    // Текущий статус
    bool serverConnected;
    bool clientConnected;
};

#endif // HOMEPAGE_H
