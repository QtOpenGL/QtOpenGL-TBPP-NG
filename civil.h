// 文明模块

#ifndef CIVIL_H
#define CIVIL_H

#include <initializer_list>
#include <iostream>
#include <list>
#include <map>
#include "bigvector.cpp"
#include "space.h"

using namespace std;

const bool PRINT_ACTION = false;  // 是否在文明执行动作时输出内容
const bool PRINT_FULL = false;    // 是否输出详细内容

const double MAX_INIT_TECH = 3.0;
const double TECH_LIMIT = 100.0;
const double TECH_STEP = 10.0;       // 两次技术爆炸之间的科技
const double TECH_BOOM_RANGE = 1.0;  // 判定技术爆炸时的误差范围
const double CHILD_CIVIL_FRIENDSHIP = 10.0;
const double RUIN_TECH_REDUCE = 1.0;  // 废墟造成的技术爆炸随时间减少的速度

const double MAX_AI_MIX = 10.0;
const int MAX_AI_PARAM = 8;
const double MAX_INIT_AI_PARAM = 0.1;

// aiMap一次取出的参数
class AiParams
{
   public:
    double n[MAX_AI_PARAM + 1];

    AiParams()
    {
        for (int i = 0; i < MAX_AI_PARAM + 1; ++i)
            n[i] = newRandom.getNormal() * MAX_INIT_AI_PARAM;
    }

    double& operator[](int i)
    {
        return n[i];
    }
};

enum ActType
{
    ACT_ATK,
    ACT_COOP
};

class Fleet
{
   public:
    int fromCivilId, targetPlanetId, initTime;
    ActType actType;
    double initDis, remainDis, initTech;
    bool deleteLater;

    // 默认构造函数
    Fleet();
    Fleet(int _fromCivilId, int _targetPlanetId, ActType _actType,
          double _initDis, double _initTech);
    void debugPrint();
    void attack();
    void cooperate();
    void action();
};

// 舰队需要经常从中间删除，因此使用list，而不是vector
extern list<Fleet> fleets;

class Civil
{
   public:
    // friendship[a][b]描述星球a对星球b的外交情况
    // 星球上的文明改变时外交参数需要重置
    // 已死文明的外交参数不会储存
    static double friendship[MAX_PLANET][MAX_PLANET];

    // 第二层策略参数
    // 已死文明的第二层策略参数不会储存
    static map<int, AiParams> aiMap[MAX_PLANET];

    static void initFriendship();

    // 殖民时复制数据
    static void colonize(Civil& oldCivil, Civil& newCivil);

    // planetId为星球编号，即Planet对象在planets中的位置
    // civilId为文明编号，即Civil对象在civils中的位置
    // 一个星球可能先后有多个文明，一个文明只对应一个星球
    // parentCivilId == -1表示没有母文明
    int planetId, civilId, parentCivilId;

    // 以clock（平直时空中的时间）为单位
    // deathTime == -1表示没有死亡
    int birthTime, deathTime;

    // 科技，时间曲率，距离下次执行动作的时间
    double tech, timeScale, remainTime;

    // 第一层策略参数
    double rateDev, rateAtk, rateCoop;

    // exiFleet[i] == true表示对星球i已经发出舰队
    bool exiFleet[MAX_PLANET];

    // 统计指标
    int childCivilCount, devCount, atkCount, coopCount;

    // 默认构造函数
    Civil();

    Civil(int _planetId, int _civilId);

    void debugPrint();

    // 根据第二层策略参数调节第一层策略参数
    double aiMix(int key, initializer_list<double> list);

    // 将第一层策略参数归一化，防止过大
    void normalizeRate();

    void develop();
    void detect(Planet& targetPlanet);
    void action();

    void mutate();
    // 平凡的进化策略，用于与厉害的进化策略对比
    void mutateNaive();
};

#ifdef BIGVECTOR_CPP
extern BigVector<Civil> civils;
#else
extern vector<Civil> civils;
#endif

#endif
