#include "logspage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

LogsPage::LogsPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void LogsPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *titleLabel = new QLabel("Логи");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel);

    QHBoxLayout *logsLayout = new QHBoxLayout();
    logsLayout->setSpacing(30);

    serverLogTextEdit = new QTextEdit();
    clientLogTextEdit = new QTextEdit();
    systemLogTextEdit = new QTextEdit();

    logsLayout->addWidget(serverLogTextEdit);
    logsLayout->addWidget(clientLogTextEdit);
    logsLayout->addWidget(systemLogTextEdit);

    mainLayout->addLayout(logsLayout);

}
