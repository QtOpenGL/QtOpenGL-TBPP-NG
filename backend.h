// 后端，在独立的线程中进行模拟文明的运算

#ifndef BACKEND_H
#define BACKEND_H

#include <QFile>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QThread>
#include "civil.h"
#include "globaltime.h"

const int MAX_CLOCK = 10000;

class Backend : public QObject
{
    Q_OBJECT

   public:
    // paused用于用户暂停
    // locked用于线程保护
    // slow为慢速模拟
    bool paused, locked, slow;
    int lastClock, lastFpsTime;
    float fps;

    Backend(QObject *parent = 0);

    void lock();
    void unlock();
    void init();

   public slots:
    void work();

   signals:
    msg(QString s);

signals:
    void initComplete();
};

extern Backend *backend;

#endif
