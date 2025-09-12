#include "homepage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

HomeWidget::HomeWidget(QWidget *parent) : QWidget(parent)
{

}



HomePage::HomePage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void HomePage::setupUI()
{
    QVBoxLayout *mainLayot = new QVBoxLayout(this);
    mainLayot->setSpacing(20);
    mainLayot->setContentsMargins(30,30,30,30);

    QLabel *titleLabel = new QLabel("Статус подключения");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
    mainLayot->addWidget(titleLabel);

    QGridLayout *gridLayot = new QGridLayout;
    gridLayot->setSpacing(30);
    gridLayot->setContentsMargins(0,0,0,0);

    btnStartServer = new QPushButton("Старт сервера");
    btnConnectClient = new QPushButton("Подключиться к клиенту");
    gridLayot->addWidget(btnStartServer, 0, 0);
    gridLayot->addWidget(btnConnectClient, 0, 1);
    mainLayot->addLayout(gridLayot);
    mainLayot->addStretch();
}
