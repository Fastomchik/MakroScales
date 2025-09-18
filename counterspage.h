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
    ~CounterWidget();
    template<typename T>
    void setValue(T value, const QString &unit = "");

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
    void setTotalCountСounter(int count);
    void setCountPrintedCounter(int count);
    void setLastWeight(float weight);
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
    float lastWeight = 0;
    int totalCount = 0;
    int countPrinted = 0;
};

#endif // COUNTERSPAGE_H
