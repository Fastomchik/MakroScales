#include "settingspage.h"
#include <QFormLayout>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>
#include <QSpinBox>

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
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
    serverIpEdit->setFixedHeight(25);
    serverIpEdit->setInputMask("000.000.000.000;_");
    formLayout->addRow("IP адрес сервера", serverIpEdit);

    serverPortEdit = new QSpinBox(this);
    serverPortEdit->setFixedHeight(25);
    serverPortEdit->setRange(1, 65535);
    formLayout->addRow("Порт сервера", serverPortEdit);


    clientIpEdit = new QLineEdit();
    clientIpEdit->setFixedHeight(25);
    clientIpEdit->setInputMask("000.000.000.000;_");
    formLayout->addRow("IP клиента", clientIpEdit);

    clientPortEdit = new QSpinBox(this);
    clientPortEdit->setFixedHeight(25);
    clientPortEdit->setRange(1, 65535);
    formLayout->addRow("Порт клиента", clientPortEdit);

    setSettings = new QPushButton("Cохранить");
    setSettings->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setSettings->setFixedHeight(50);
    formLayout->addRow(setSettings);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
}

void SettingsPage::onSaveButtonClicked()
{
    QString serverIp = serverIpEdit->text();
    int serverPort = serverPortEdit->value();
    QString client_Ip = clientIpEdit->text();
    int clientPort = clientPortEdit->value();

    if (serverIp.isEmpty() || client_Ip.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните");
        return;
        }

    QSettings settings("Makro", "MakroScales");
    settings.setValue("server/ip", serverIp);
    settings.setValue("server/port", serverPort);
    settings.setValue("client/ip", client_Ip);
    settings.setValue("client/port", clientPort);

    settings.sync();
}
