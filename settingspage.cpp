#include "settingspage.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    QLabel *titleLabel = new QLabel("Страница настроек");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setLabelAlignment(Qt::AlignRight);

    QLineEdit *serverIpEdit = new QLineEdit(this);
    serverIpEdit->setInputMask("000.000.000.000;_");
    formLayout->addRow("IP адрес сервера", serverIpEdit);

    QSpinBox *serverPortEdit = new QSpinBox(this);
    serverPortEdit->setRange(1, 65535);
    formLayout->addRow("Порт сервера", serverPortEdit);



    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

}
