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
    void setTotalCountСounter(int count);
    void setCountPrintedCounter(int count);

private:
    void setupUI();
    void updateDisplay();

    // Счётчики
    CounterWidget *countBufferInPrinterCounter;
    CounterWidget *lastWeightCounter;
    CounterWidget *totalCountСounter;
    CounterWidget *countPrintedCounter;

    // Данные
    int bufferCodesCount = 0;
    int lastWeight = 0;
    int totalCount = 0;
    int countPrinted = 0;
};

#endif // COUNTERSPAGE_H
