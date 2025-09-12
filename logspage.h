#ifndef LOGSPAGE_H
#define LOGSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class LogsPage : public QWidget
{
    Q_OBJECT
public:
    explicit LogsPage(QWidget *parent = nullptr);
};

#endif // LOGSPAGE_H
