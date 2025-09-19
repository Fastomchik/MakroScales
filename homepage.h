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
    ~HomePage();
    // Методы для обновления статуса
    void resetStatus();
    void setServerStatus(bool connected);
    void setClientStatus(bool connected);
    bool getServerStatus();
    bool getClientStatus();
public slots:
    void on_btn_start_server_clicked();
    void on_btn_connect_client_clicked();
    void on_btn_disconnect_server_clicked();
    void on_btn_disconnect_client_clicked();
signals:
    void startServerRequested();
    void startClientRequested();
    void stopServerRequested();
    void stopClientRequested();
    void clientStatusChanged(bool clientStatus);
    void serverStatusChanged(bool serverStatus);
private:
    void setupUI();
    void updateStatusDisplays();
    void updateButtonTexts();

    // Кнопки
    QPushButton *btnStartServer;
    QPushButton *btnConnectClient;
    QPushButton *btnDisconnectServer;
    QPushButton *btnDisconnectClient;
    // Кружки статуса
    QLabel *serverStatusCircle;
    QLabel *clientStatusCircle;

    // Подписи
    QLabel *serverLabel;
    QLabel *clientLabel;

    // Текущий статус
    bool serverConnected;
    bool clientConnected;
    bool serverRunning;
    bool clientRunning;
};

#endif // HOMEPAGE_H
