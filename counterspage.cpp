#include "counterspage.h"

CountersPage::CountersPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Страница счетчиков");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}
