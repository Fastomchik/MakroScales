#include "homepage.h"

HomePage::HomePage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Главная страница\nДобро пожаловать в систему мониторинга");
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 16px;");
    layout->addWidget(label);
}
