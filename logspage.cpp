#include "logspage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>

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
    serverLogTextEdit->setReadOnly(true);
    serverLogTextEdit->setPlaceholderText("Логи сервера...");

    clientLogTextEdit = new QTextEdit();
    clientLogTextEdit->setReadOnly(true);
    clientLogTextEdit->setPlaceholderText("Логи клиента...");

    systemLogTextEdit = new QTextEdit();
    systemLogTextEdit->setReadOnly(true);
    systemLogTextEdit->setPlaceholderText("Системные логи...");

    logsLayout->addWidget(serverLogTextEdit);
    logsLayout->addWidget(clientLogTextEdit);
    logsLayout->addWidget(systemLogTextEdit);

    mainLayout->addLayout(logsLayout);
}

void LogsPage::addLogMessage(const QString &message)
{
    QString timeStamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    QString formattedMessage = timeStamp + message;

    if (message.contains("[Server]", Qt::CaseInsensitive)) {
        serverLogTextEdit->append(formattedMessage);
    } else if (message.contains("[Client]", Qt::CaseInsensitive)) {
        clientLogTextEdit->append(formattedMessage);
    } else {
        systemLogTextEdit->append(formattedMessage);
    }
}
