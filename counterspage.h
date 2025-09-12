#ifndef COUNTERSPAGE_H
#define COUNTERSPAGE_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>

class CounterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CounterWidget(const QString &title, QWidget *parent = nullptr);
    void setValue(int value, const QString &unit = "");

private:
    QLabel *titleLabel;
    QLabel *valueLabel;
};

class CountersPage : public QWidget
{
    Q_OBJECT
public:
    explicit CountersPage(QWidget *parent = nullptr);

    // Публичные методы для обновления значений извне
    void setBufferCodesCount(int count);
    void setLastWeight(int weight);

private:
    void setupUI();
    void updateDisplay();

    // Счётчики
    CounterWidget *bufferCodesCounter;
    CounterWidget *lastWeightCounter;

    // Данные
    int bufferCodesCount;
    int lastWeight;
};

#endif // COUNTERSPAGE_H
