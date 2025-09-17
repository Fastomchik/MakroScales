#include "settingspage.h"
#include <QFormLayout>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
    loadSettings(); // Загружаем настройки при создании
}
SettingsPage::~SettingsPage()
{

}

void SettingsPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QLabel *titleLabel = new QLabel("Конфигурация");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setLabelAlignment(Qt::AlignRight);

    serverIpEdit = new QLineEdit(this);
    serverIpEdit->setFixedHeight(35);
    serverIpEdit->setInputMask("000.000.000.000;_");
    formLayout->addRow("IP адрес сервера", serverIpEdit);

    serverPortEdit = new QSpinBox(this);
    serverPortEdit->setFixedHeight(35);
    serverPortEdit->setRange(1, 65535);
    formLayout->addRow("Порт сервера", serverPortEdit);

    clientIpEdit = new QLineEdit(this);
    clientIpEdit->setFixedHeight(35);
    clientIpEdit->setInputMask("000.000.000.000;_");
    formLayout->addRow("IP принтера", clientIpEdit);

    clientPortEdit = new QSpinBox(this);
    clientPortEdit->setFixedHeight(35);
    clientPortEdit->setRange(1, 65535);
    formLayout->addRow("Порт принтера", clientPortEdit);

    setSettings = new QPushButton("Сохранить настройки");
    setSettings->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setSettings->setFixedHeight(50);
    setSettings->setStyleSheet("QPushButton { background-color: #3498db; color: white; font-weight: bold; }");

    connect(setSettings, &QPushButton::clicked, this, &SettingsPage::onSaveButtonClicked);

    formLayout->addRow(setSettings);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
}

void SettingsPage::onSaveButtonClicked()
{
    QString serverIp = serverIpEdit->text();
    int serverPort = serverPortEdit->value();
    QString clientIp = clientIpEdit->text();
    int clientPort = clientPortEdit->value();

    // Проверка валидности IP адресов
    if (serverIp.isEmpty() || serverIp.contains("_")) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, введите корректный IP адрес сервера");
        return;
    }

    if (clientIp.isEmpty() || clientIp.contains("_")) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, введите корректный IP адрес принтера");
        return;
    }

    QSettings settings("Makro", "MakroScales");
    settings.setValue("server/ip", serverIp);
    settings.setValue("server/port", serverPort);
    settings.setValue("client/ip", clientIp);
    settings.setValue("client/port", clientPort);

    settings.sync();

    emit settingsSaved();

    QMessageBox::information(this, "Успех", "Настройки успешно сохранены!");
}

void SettingsPage::loadSettings()
{
    QSettings settings("Makro", "MakroScales");

    QString serverIp = settings.value("server/ip", "192.168.1.100").toString();
    int serverPort = settings.value("server/port", 8080).toInt();
    QString clientIp = settings.value("client/ip", "192.168.1.200").toString();
    int clientPort = settings.value("client/port", 9100).toInt();

    serverIpEdit->setText(serverIp);
    serverPortEdit->setValue(serverPort);
    clientIpEdit->setText(clientIp);
    clientPortEdit->setValue(clientPort);
}

QString SettingsPage::getServerIp() const
{
    return serverIpEdit->text();
}

int SettingsPage::getServerPort() const
{
    return serverPortEdit->value();
}

QString SettingsPage::getClientIp() const
{
    return clientIpEdit->text();
}

int SettingsPage::getClientPort() const
{
    return clientPortEdit->value();
}
