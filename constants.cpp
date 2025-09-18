#include "constants.h"
#include <QDebug>
#include <QMutex>
#include <QQueue>

// Глобальные переменные (если они нужны в других местах)
//QQueue<QByteArray> printQueue;
//QQueue<QString> makrolineQueue;
//QMutex printQueueMutex;

Constants::Constants(QObject *parent)
    : QObject(parent)
{
    // Теперь класс только инициализирует константы
    qDebug() << "Constants initialized";
}

Constants::~Constants()
{
    // Больше не удаляем объекты, так как они управляются AppController
    qDebug() << "Constants destroyed";
}
