#include <fstream>
#include "civil.h"

const int MAX_MAIN_LOOP = 100;

int main()
{
    // 从文件读取星球
    int inPlanetCount = 0;
    ifstream in("in.txt", fstream::in);
    if (in)
    {
        while (!in.eof())
        {
            double x, y, mass;
            in >> x >> y >> mass;
            planets[inPlanetCount] =
                Planet(inPlanetCount, inPlanetCount, Point(x, y), mass);
            ++inPlanetCount;
        }
        in.close();
    }

    // 随机初始化剩余星球
    for (int i = inPlanetCount; i < MAX_PLANET; ++i)
        planets[i] =
            Planet(i, i, newRandom.getPoint() * 100.0, newRandom.get() * 10.0);
    space.calcCurv();
    space.calcPlanetDis();

    // 初始化文明
    for (int i = 0; i < MAX_PLANET; ++i)
        civils.push_back(Civil(civils.size(), civils.size()));
    // civils[0].tech = 10.0;
    Civil::initFriendship();

    // 主循环
    for (int mainLoopCount = 0; mainLoopCount < MAX_MAIN_LOOP; ++mainLoopCount)
    {
        ++space.clock;
        cout << "clock " << space.clock << " civils " << civils.size()
             << " fleets " << fleets.size() << endl
             << endl;

        // 文明执行动作时不改变星球数量，会改变文明和舰队数量
        // 舰队执行动作时不改变舰队数量，需要删除的舰队标记为deleteLater
        // 因此可以放在for循环里
        cout << "civils action" << endl;
        for (int i = 1; i < MAX_PLANET; ++i)
            civils[planets[i].civilId].action();

        cout << "fleets action" << endl;
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

        cout << "civils print" << endl;
        for (int i = 1; i < MAX_PLANET; ++i)
            civils[planets[i].civilId].debugPrint();

        cout << "fleets print" << endl;
        for (auto i = fleets.begin(); i != fleets.end(); ++i) i->debugPrint();

        cout << endl;
    }

    return 0;
}
