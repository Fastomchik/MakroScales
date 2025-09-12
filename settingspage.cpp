#include "settingspage.h"

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Страница настроек");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
}
