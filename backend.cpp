// 后端，在独立的线程中进行模拟文明的运算

#include "backend.h"

Backend *backend;

// 初始暂停运算，用户手动开始
Backend::Backend(QObject *parent)
    : QObject(parent),
      paused(true),
      locked(false),
      slow(false),
      lastClock(0),
      lastFpsTime(0),
      fps(0.0)
{
}

void Backend::lock()
{
    // 等待上个锁解除
    while (locked) QThread::msleep(1);
    locked = true;
}

void Backend::unlock()
{
    locked = false;
}

void Backend::init()
{
//    emit msg("正在读取星球数据...");
    emit msg("Backend Step 1 ...");
    int inPlanetCount = 0;
    QFile file("in.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            double x, y, mass;
            in >> x >> y >> mass;
            planets[inPlanetCount] =
                Planet(inPlanetCount, inPlanetCount, Point(x, y), mass);
            ++inPlanetCount;
        }
        file.close();
    }
//    emit msg("正在初始化剩余星球数据...");
    emit msg("Backend Step 2 ...");
    for (int i = inPlanetCount; i < MAX_PLANET; ++i)
        planets[i] =
            Planet(i, i, newRandom.getPoint() * 100.0, newRandom.get() * 10.0);
//    emit msg("正在计算时空曲率...");
    emit msg("Backend Step 3 ...");
    space.calcCurv();
//    emit msg("正在计算星球距离...");
    emit msg("Backend Step 4 ...");
    space.calcPlanetDis();
//    emit msg("正在初始化文明数据...");
    emit msg("Backend Step 5 ...");
    for (int i = 0; i < MAX_PLANET; ++i)
    {
        Civil c(i, i);
        civils.push_back(c);
    }
    Civil::initFriendship();
//    emit msg("初始化完成");
    emit msg("Backend Step 6 ...");

    emit initComplete();

    emit msg("Backend completed ...");
}

void Backend::work()
{
    while (true)
    {
        while (paused || locked) QThread::msleep(1);
        if (slow) QThread::msleep(100);

        ++space.clock;
        if (space.clock >= MAX_CLOCK) break;
        if (PRINT_ACTION)
            cout << "clock " << space.clock << " civils " << civils.size()
                 << " fleets " << fleets.size() << endl;

        // 文明执行动作时不改变星球数量，会改变文明和舰队数量
        // 舰队执行动作时不改变舰队数量，需要删除的舰队标记为deleteLater
        // 因此可以放在for循环里
        for (int i = 0; i < MAX_PLANET; ++i)
            civils[planets[i].civilId].action();
        for (auto i = fleets.begin(); i != fleets.end(); ++i) i->action();

        // 删除需要删除的舰队
        auto i = fleets.begin();
        while (i != fleets.end())
        {
            if (i->deleteLater)
                i = fleets.erase(i);
            else
                ++i;
        }

        int nowFpsTime = gTime.elapsed();
        if (nowFpsTime - lastFpsTime > 100)
        {
            fps = float(space.clock - lastClock) /
                  float(nowFpsTime - lastFpsTime) * 1000.0;
            lastClock = space.clock;
            lastFpsTime = nowFpsTime;
        }
    }
}
