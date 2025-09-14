#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QObject>
#include <QQueue>

extern QQueue<QByteArray> printQueue;
extern QQueue<QString> makrolineQueue;

class Constants : public QObject
{
    Q_OBJECT
public:
    explicit Constants(QObject *parent = nullptr);
private:

};

#endif // CONSTANTS_H
