#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QPushButton>

class HomeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HomeWidget(QWidget *parent = nullptr);
};


class HomePage : public QWidget
{
    Q_OBJECT
public:
    explicit HomePage(QWidget *parent = nullptr);
private:
    void setupUI();
    void updateDisplay();

    // Кнопки
    QPushButton *btnStartServer;
    QPushButton *btnConnectClient;

};

#endif // HOMEPAGE_H
