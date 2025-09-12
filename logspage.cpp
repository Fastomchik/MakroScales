#include "logspage.h"

LogsPage::LogsPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Страница логов");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}
