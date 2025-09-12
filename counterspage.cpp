#include "counterspage.h"
#include <QVBoxLayout>
#include <QFont>

CounterWidget::CounterWidget(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->setContentsMargins(15, 15, 15, 15);

    // Название счётчика (сверху)
    titleLabel = new QLabel(title);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 14px; color: #555; font-weight: bold;");

    // Значение счётчика (снизу)
    valueLabel = new QLabel("0");
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #2c3e50;");

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);

    // Стиль рамки
    setStyleSheet("border: 2px solid #bdc3c7; border-radius: 10px; background-color: #ecf0f1;");
    setFixedSize(200, 120);
}

void CounterWidget::setValue(int value, const QString &unit) {
    if (unit.isEmpty()) {
        valueLabel->setText(QString::number(value));
    } else {
        valueLabel->setText(QString("%1 %2").arg(value).arg(unit));
    }
}

CountersPage::CountersPage(QWidget *parent) : QWidget(parent),
    bufferCodesCount(0),
    lastWeight(0)
{
    setupUI();
}

void CountersPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // Заголовок страницы
    QLabel *titleLabel = new QLabel("Мониторинг весов");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin-bottom: 20px;");
    mainLayout->addWidget(titleLabel);

    // Сетка для счётчиков
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setSpacing(30);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    // Создаём счётчики
    bufferCodesCounter = new CounterWidget("Кол-во кодов в буфере");
    lastWeightCounter = new CounterWidget("Последний вес");

    // Размещаем счётчики в ряд
    gridLayout->addWidget(bufferCodesCounter, 0, 0);
    gridLayout->addWidget(lastWeightCounter, 0, 1);

    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch();

    // Инициализируем отображение
    updateDisplay();
}

void CountersPage::setBufferCodesCount(int count)
{
    bufferCodesCount = count;
    updateDisplay();
}

void CountersPage::setLastWeight(int weight)
{
    lastWeight = weight;
    updateDisplay();
}

void CountersPage::updateDisplay()
{
    bufferCodesCounter->setValue(bufferCodesCount, "шт");
    lastWeightCounter->setValue(lastWeight, "г");
}
