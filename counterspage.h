#ifndef COUNTERSPAGE_H
#define COUNTERSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class CountersPage : public QWidget
{
    Q_OBJECT
public:
    explicit CountersPage(QWidget *parent = nullptr);
};

#endif // COUNTERSPAGE_H
