#ifndef LOGSPAGE_H
#define LOGSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QTextEdit>

class LogsPage : public QWidget
{
    Q_OBJECT
public:
    explicit LogsPage(QWidget *parent = nullptr);
private:
    void setupUI();


    QTextEdit *serverLogTextEdit;
    QTextEdit *clientLogTextEdit;
    QTextEdit *systemLogTextEdit;
};

#endif // LOGSPAGE_H
