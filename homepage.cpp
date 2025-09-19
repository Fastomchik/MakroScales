#include "homepage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

HomePage::HomePage(QWidget *parent) : QWidget(parent),
    serverConnected(false),
    clientConnected(false)
{
    setupUI();
}

HomePage::~HomePage()
{

}

void HomePage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30,30,30,30);

    QLabel *titleLabel = new QLabel("Статус подключения");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setSpacing(30);
    gridLayout->setContentsMargins(0,0,0,0);

    // Создаем кнопки
    btnStartServer = new QPushButton("Старт сервера");
    btnConnectClient = new QPushButton("Подключиться к клиенту");
    btnDisconnectServer = new QPushButton("Стоп сервера");
    btnDisconnectClient = new QPushButton("Отключиться от клиента");
    btnStartServer->setFixedSize(200, 50);
    btnConnectClient->setFixedSize(200, 50);
    btnDisconnectServer->setFixedSize(200, 50);
    btnDisconnectClient->setFixedSize(200, 50);



    connect(btnStartServer, &QPushButton::clicked, this, &HomePage::on_btn_start_server_clicked);
    connect(btnConnectClient, &QPushButton::clicked, this, &HomePage::on_btn_connect_client_clicked);
    connect(btnDisconnectServer, &QPushButton::clicked, this, &HomePage::on_btn_disconnect_server_clicked);
    connect(btnDisconnectClient, &QPushButton::clicked, this, &HomePage::on_btn_disconnect_client_clicked);


    // Создаем кружки статуса
    serverStatusCircle = new QLabel();
    clientStatusCircle = new QLabel();
    serverStatusCircle->setFixedSize(60, 60);
    clientStatusCircle->setFixedSize(60, 60);
    serverStatusCircle->setAlignment(Qt::AlignCenter);
    clientStatusCircle->setAlignment(Qt::AlignCenter);

    // Создаем подписи
    serverLabel = new QLabel("Сервер");
    clientLabel = new QLabel("Клиент");
    serverLabel->setAlignment(Qt::AlignCenter);
    clientLabel->setAlignment(Qt::AlignCenter);
    serverLabel->setStyleSheet("font: 600 14pt Segoe UI; font-weight: bold;");
    clientLabel->setStyleSheet("font: 600 14pt Segoe UI; font-weight: bold;");

    // Обновляем отображение статуса
    updateStatusDisplays();

    // Добавляем виджеты в grid layout
    gridLayout->addWidget(serverLabel, 0, 0);
    gridLayout->addWidget(clientLabel, 0, 1);
    gridLayout->addWidget(btnStartServer, 1, 0);
    gridLayout->addWidget(btnConnectClient, 1, 1);
    gridLayout->addWidget(serverStatusCircle, 2, 0, Qt::AlignCenter);
    gridLayout->addWidget(clientStatusCircle, 2, 1, Qt::AlignCenter);
    gridLayout->addWidget(btnDisconnectServer, 3, 0);
    gridLayout->addWidget(btnDisconnectClient, 3, 1);

    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch();
}

void HomePage::updateStatusDisplays()
{
    // Обновляем кружок сервера
    if (serverConnected) {
        serverStatusCircle->setStyleSheet(
            "border-radius: 30px;"
            "background-color: #2ecc71;"  // Зеленый
            "border: 2px solid #27ae60;"
            );
    } else {
        serverStatusCircle->setStyleSheet(
            "border-radius: 30px;"
            "background-color: #e74c3c;"  // Красный
            "border: 2px solid #c0392b;"
            );
    }

    // Обновляем кружок клиента
    if (clientConnected) {
        clientStatusCircle->setStyleSheet(
            "border-radius: 30px;"
            "background-color: #2ecc71;"  // Зеленый
            "border: 2px solid #27ae60;"
            );
    } else {
        clientStatusCircle->setStyleSheet(
            "border-radius: 30px;"
            "background-color: #e74c3c;"  // Красный
            "border: 2px solid #c0392b;"
            );
    }
}

void HomePage::updateButtonTexts()
{
    // Обновляем текст кнопок в зависимости от состояния
    if (serverRunning) {
        btnStartServer->setText("Остановить сервер");
        btnStartServer->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; }");
    } else {
        btnStartServer->setText("Старт сервера");
        btnStartServer->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; }");
    }

    if (clientRunning) {
        btnConnectClient->setText("Отключиться от клиента");
        btnConnectClient->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; }");
    } else {
        btnConnectClient->setText("Подключиться к клиенту");
        btnConnectClient->setStyleSheet("QPushButton { background-color: #3498db; color: white; }");
    }
}

void HomePage::setServerStatus(bool connected)
{
    serverConnected = connected;
    updateStatusDisplays();
}

void HomePage::setClientStatus(bool connected)
{
    clientConnected = connected;
    updateStatusDisplays();
}

void HomePage::getServerStatus()
{
    emit serverStatusChanged(serverConnected);
}
void HomePage::getClientStatus()
{
    emit clientStatusChanged(clientConnected);
}

void HomePage::resetStatus()
{
    serverConnected = false;
    clientConnected = false;
    updateStatusDisplays();
}

void HomePage::on_btn_start_server_clicked()
{
    qDebug() << "Кнопка старта сервера нажата";
    emit startServerRequested();
}

void HomePage::on_btn_connect_client_clicked()
{
    qDebug() << "Кнопка подключения клиента нажата";
    emit startClientRequested();
}

void HomePage::on_btn_disconnect_server_clicked()
{
    qDebug() << "Кнопка отключения сервера нажата";
    emit stopServerRequested();
}

void HomePage::on_btn_disconnect_client_clicked()
{
    qDebug() << "Кнопка отключения клиента нажата";
    emit stopClientRequested();
}
