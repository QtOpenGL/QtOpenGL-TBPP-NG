// 文明模块

#include "civil.h"

list<Fleet> fleets;

#ifdef BIGVECTOR_CPP
BigVector<Civil> civils;
#else
vector<Civil> civils;
#endif

double Civil::friendship[MAX_PLANET][MAX_PLANET];

map<int, AiParams> Civil::aiMap[MAX_PLANET];

Fleet::Fleet()
{
}

Fleet::Fleet(int _fromCivilId, int _targetPlanetId, ActType _actType,
             double _initDis, double _initTech)
    : fromCivilId(_fromCivilId),
      targetPlanetId(_targetPlanetId),
      initTime(space.clock),
      actType(_actType),
      initDis(_initDis),
      remainDis(_initDis),
      initTech(_initTech),
      deleteLater(false)
{
}

void Fleet::debugPrint()
{
    cout << fromCivilId << " " << targetPlanetId << " " << initTime << " "
         << actType << " " << initDis << " " << remainDis << " " << initTech
         << endl;
}

void Fleet::attack()
{
    Planet& targetPlanet = planets[targetPlanetId];
    Civil& target = civils[targetPlanet.civilId];

// 对方的第一层策略参数和外交参数变化
#define tempMacro(a)                                                      \
    a += civils[fromCivilId].aiMix(                                       \
        __LINE__,                                                         \
        {target.rateDev, target.rateAtk, target.rateCoop,                 \
         Civil::friendship[targetPlanetId][civils[fromCivilId].planetId], \
         target.tech / civils[fromCivilId].tech,                          \
         target.timeScale / civils[fromCivilId].timeScale,                \
         space.getDis(targetPlanetId, civils[fromCivilId].planetId)})
    tempMacro(target.rateDev);
    tempMacro(target.rateAtk);
    tempMacro(target.rateCoop);
    tempMacro(Civil::friendship[targetPlanetId][civils[fromCivilId].planetId]);
#undef tempMacro

    // 判定是否攻击成功
    double atkChance = newRandom.get();
    if (1.0 - exp(-initTech / target.tech) > atkChance)
    {
        target.deathTime = space.clock;
        target.normalizeRate();

        // 判定殖民或成为废墟
        double desChance = newRandom.get();
        if (0.1 * exp(-initTech / target.tech) < desChance)
        {
            // 殖民
            if (PRINT_ACTION)
                cout << fromCivilId << " colonize " << target.civilId << endl;
            civils.push_back(Civil(targetPlanetId, civils.size()));
            // push_back之后原来的文明在内存中的位置可能发生变化，需要重新引用
            Civil& oldCivil = civils[fromCivilId];
            Civil& newCivil = civils[civils.size() - 1];
            targetPlanet.civilId = newCivil.civilId;
            // 复制参数
            Civil::colonize(oldCivil, newCivil);
            // 变异
            newCivil.mutate();
        }
        else
        {
            // 成为废墟
            if (PRINT_ACTION)
                cout << fromCivilId << " ruin " << target.civilId << endl;
            targetPlanet.ruinMark = true;
            targetPlanet.lastTech = target.tech;

            // 产生随机初始化的新文明
            civils.push_back(Civil(targetPlanetId, civils.size()));
            Civil& newCivil = civils[civils.size() - 1];
            targetPlanet.civilId = newCivil.civilId;

            // 重置外交参数
            for (int i = 0; i < MAX_PLANET; ++i)
            {
                Civil::friendship[targetPlanetId][i] = 0.0;
                Civil::friendship[i][targetPlanetId] = 0.0;
            }
        }
    }
}

void Fleet::cooperate()
{
    Civil& target = civils[planets[targetPlanetId].civilId];

// 对方的第一层策略参数和外交参数变化
#define tempMacro(a)                                                      \
    a += civils[fromCivilId].aiMix(                                       \
        __LINE__,                                                         \
        {target.rateDev, target.rateAtk, target.rateCoop,                 \
         Civil::friendship[targetPlanetId][civils[fromCivilId].planetId], \
         target.tech / civils[fromCivilId].tech,                          \
         target.timeScale / civils[fromCivilId].timeScale,                \
         space.getDis(targetPlanetId, civils[fromCivilId].planetId)})
    tempMacro(target.rateDev);
    tempMacro(target.rateAtk);
    tempMacro(target.rateCoop);
    tempMacro(Civil::friendship[targetPlanetId][civils[fromCivilId].planetId]);
#undef tempMacro

    // 双方科技增加
    double t = initTech / target.tech;
    //    civils[fromCivilId].tech += pow((2.0 * t / (1.0 + pow(t, 4))), 4);
    civils[fromCivilId].tech += 2.67 * exp(-0.333 * t) / (t + 1.0 / t);
    t = 1.0 / t;
    //    target.tech += pow((2.0 * t / (1.0 + pow(t, 4))), 4);
    target.tech += 2.67 * exp(-0.333 * t) / (t + 1.0 / t);
}

void Fleet::action()
{
    remainDis -= 1.0;
    if (remainDis <= 0.0)
    {
        switch (actType)
        {
            case ACT_ATK:
                if (PRINT_ACTION)
                    cout << fromCivilId << " attack " << targetPlanetId << endl;
                attack();
                break;
            case ACT_COOP:
                if (PRINT_ACTION)
                    cout << fromCivilId << " cooperate " << targetPlanetId
                         << endl;
                cooperate();
                break;
        }
        civils[fromCivilId].exiFleet[targetPlanetId] = false;
        deleteLater = true;
    }
}

void Civil::initFriendship()
{
    for (int i = 0; i < MAX_PLANET; ++i)
        for (int j = 0; j < MAX_PLANET; ++j) friendship[i][j] = 0.0;
}

void Civil::colonize(Civil& oldCivil, Civil& newCivil)
{
    newCivil.parentCivilId = oldCivil.civilId;
    ++oldCivil.childCivilCount;

// 复制自己的科技与策略参数
#define copyData(a) newCivil.a = oldCivil.a
    copyData(tech);
    copyData(rateAtk);
    copyData(rateCoop);
    copyData(rateDev);
#undef copyData
    aiMap[newCivil.planetId] = aiMap[oldCivil.planetId];

    // 复制自己对其他星球的外交参数，其他星球对自己的外交参数
    for (int i = 0; i < MAX_PLANET; ++i)
    {
        friendship[newCivil.planetId][i] = friendship[oldCivil.planetId][i];
        friendship[i][newCivil.planetId] = friendship[i][oldCivil.planetId];
    }
    // 标记自己与子文明的外交参数
    friendship[oldCivil.planetId][newCivil.planetId] = CHILD_CIVIL_FRIENDSHIP;
    friendship[newCivil.planetId][oldCivil.planetId] = CHILD_CIVIL_FRIENDSHIP;
}

Civil::Civil()
{
}

// 随机初始化
Civil::Civil(int _planetId, int _civilId)
    : planetId(_planetId),
      civilId(_civilId),
      parentCivilId(-1),
      birthTime(space.clock),
      deathTime(-1),
      tech(newRandom.get() * MAX_INIT_TECH),
      timeScale(space.curvtt(planets[_planetId].x, planets[_planetId].y)),
      remainTime(timeScale),
      rateDev(newRandom.get()),
      rateAtk(newRandom.get()),
      rateCoop(newRandom.get()),
      childCivilCount(0),
      devCount(0),
      atkCount(0),
      coopCount(0)
{
    for (int i = 0; i < MAX_PLANET; ++i) exiFleet[i] = false;
}

void Civil::debugPrint()
{
    if (PRINT_FULL)
    {
        cout << planetId << " " << planets[planetId].x << " "
             << planets[planetId].y << " " << planets[planetId].mass << endl;
        cout << civilId << " " << parentCivilId << " " << birthTime << " "
             << deathTime << " " << space.clock - birthTime << endl;
        cout << tech << " " << timeScale << " " << remainTime << endl;
        cout << rateDev << " " << rateAtk << " " << rateCoop << endl;
        cout << childCivilCount << " " << devCount << " " << atkCount << " "
             << coopCount << endl;
        cout << endl;
    }
    else
    {
        cout << planetId << " " << civilId << " " << tech << " "
             << space.clock - birthTime << endl;
    }
}

// 参数不能超过MAX_AI_PARAM个
double Civil::aiMix(int key, initializer_list<double> list)
{
    auto& p = aiMap[planetId][key];
    double res = 0.0;
    for (size_t i = 0; i < list.size(); ++i)
        res += p[i] * (*(list.begin() + i));
    res = p[MAX_AI_PARAM];
    // 防止过大
    if (abs(res) > MAX_AI_MIX) res = MAX_AI_MIX / res;
    return res;
}

void Civil::normalizeRate()
{
    double sum = abs(rateDev) + abs(rateAtk) + abs(rateCoop);
    rateDev /= sum;
    rateAtk /= sum;
    rateCoop /= sum;
}

void Civil::develop()
{
    if (PRINT_ACTION) cout << civilId << " develop" << endl;
    /* DEPRECATED
    // 科技累积到一定程度时触发技术爆炸
    for (int i = 1; i < TECH_LIMIT / TECH_STEP; ++i)
    {
        if (abs(tech - TECH_STEP * i) < TECH_BOOM_RANGE)
        {
            if (PRINT_ACTION) cout << civilId << " tech boom" << endl;
            tech += 2.0;
            return;
        }
    }
    */
    tech += 1.0;
}

// 探测的功能还要再考虑，先不要写到action里
// 探测到废墟则科技增加，增加量与上一个文明的科技成正比，随时间指数衰减
// TODO：探测到废墟时，若对方科技比自己高，或者科技走向不同（发展策略不同）则技术爆炸
void Civil::detect(Planet& targetPlanet)
{
    if (PRINT_ACTION)
        cout << civilId << " detect " << targetPlanet.planetId << endl;
    if (targetPlanet.ruinMark)
    {
        tech += targetPlanet.lastTech *
                exp(-RUIN_TECH_REDUCE *
                    (space.clock - civils[targetPlanet.civilId].deathTime));
        targetPlanet.ruinMark = false;
    }
}

void Civil::action()
{
    remainTime -= 1.0;
    if (remainTime > 0.0) return;
    remainTime = timeScale;

    // 回合开始前将第一层策略参数归一化，防止过大
    normalizeRate();

    // 0：发展
    // 1 ~ MAX_PLANET：攻击
    // MAX_PLANET * + 1 ~ MAX_PLANET * 2 + 1：合作
    const int actCount = MAX_PLANET * 2 + 1;
    double actProb[actCount];

    // 发展的概率
    actProb[0] = exp(aiMix(__LINE__, {rateDev}));
    double sum = actProb[0];
    // 攻击、合作的概率
    for (int i = 0; i < MAX_PLANET; ++i)
    {
        if (!space.isNear[planetId][i] || exiFleet[i])
        {
            actProb[i + 1] = actProb[i + MAX_PLANET + 1] = 0.0;
            continue;
        }
        actProb[i + 1] = exp(
            aiMix(__LINE__, {rateAtk, friendship[planetId][i],
                             tech / civils[planets[i].civilId].tech,
                             timeScale / civils[planets[i].civilId].timeScale,
                             space.getDis(planetId, i)}));
        actProb[i + MAX_PLANET + 1] = exp(
            aiMix(__LINE__, {rateCoop, friendship[planetId][i],
                             tech / civils[planets[i].civilId].tech,
                             timeScale / civils[planets[i].civilId].timeScale,
                             space.getDis(planetId, i)}));
        sum += actProb[i + 1] + actProb[i + MAX_PLANET + 1];
    }

    // 将概率归一化
    double invSum = 1.0 / sum;
    for (int i = 0; i < actCount; ++i) actProb[i] *= invSum;
    double choice = newRandom.get();
    int choiceIndex = 0;
    double sumProb = 0.0;
    while (choiceIndex < actCount)
    {
        sumProb += actProb[choiceIndex];
        if (sumProb > choice) break;
        ++choiceIndex;
    }
    if (choiceIndex >= actCount)
    {
        // ERROR
    }

    if (choiceIndex == 0)
    {
// 发展
// 第一层策略参数变化
// 用行号确定需要的第二层策略参数
#define tempMacro(a) a += aiMix(__LINE__, {rateDev, rateAtk, rateCoop})
        tempMacro(rateDev);
        tempMacro(rateAtk);
        tempMacro(rateCoop);
#undef tempMacro
        develop();
        ++devCount;
    }
    else if (choiceIndex <= MAX_PLANET)
    {
        // 攻击
        --choiceIndex;
        Planet& p = planets[choiceIndex];
// 第一层策略参数变化
// 用行号确定需要的第二层策略参数
#define tempMacro(a)                                                          \
    a += aiMix(__LINE__,                                                      \
               {rateDev, rateAtk, rateCoop, friendship[planetId][p.planetId], \
                tech / civils[p.civilId].tech,                                \
                timeScale / civils[p.civilId].timeScale,                      \
                space.getDis(planetId, p.planetId)})
        tempMacro(rateDev);
        tempMacro(rateAtk);
        tempMacro(rateCoop);
#undef tempMacro
        fleets.push_back(Fleet(civilId, p.planetId, ACT_ATK,
                               space.getDis(planetId, p.planetId), tech));

        if (PRINT_ACTION)
            cout << civilId << " begin attack " << p.planetId << endl;
        exiFleet[p.planetId] = true;
        ++atkCount;
    }
    else
    {
        // 合作
        choiceIndex -= MAX_PLANET + 1;
        Planet& p = planets[choiceIndex];
// 第一层策略参数变化
// 用行号确定需要的第二层策略参数
#define tempMacro(a)                                                          \
    a += aiMix(__LINE__,                                                      \
               {rateDev, rateAtk, rateCoop, friendship[planetId][p.planetId], \
                tech / civils[p.civilId].tech,                                \
                timeScale / civils[p.civilId].timeScale,                      \
                space.getDis(planetId, p.planetId)})
        tempMacro(rateDev);
        tempMacro(rateAtk);
        tempMacro(rateCoop);
#undef tempMacro
        fleets.push_back(Fleet(civilId, p.planetId, ACT_COOP,
                               space.getDis(planetId, p.planetId), tech));

        if (PRINT_ACTION)
            cout << civilId << " begin cooperate " << p.planetId << endl;
        exiFleet[p.planetId] = true;
        ++coopCount;
    }
}

void Civil::mutate()
{
    const int MAX_PARENT = 5;
    const double STEP_WEIGHT_REDUCE = 0.5;

    // 将各代母文明存入parentIdList，不包括自己
    // 达到MAX_PARENT或没有母文明时结束
    int parentIdList[MAX_PARENT];
    int parentCount = 0;
    int nowCivilId = civilId;
    for (int i = 0; i < MAX_PARENT; ++i)
    {
        if (civils[nowCivilId].parentCivilId == -1) break;
        nowCivilId = civils[nowCivilId].parentCivilId;
        parentIdList[parentCount] = nowCivilId;
        ++parentCount;
    }

    auto& p = aiMap[planetId];
    for (auto i : p)
    {
        for (int j = 0; j < MAX_AI_PARAM + 1; ++j)
        {
            // stepCumu是各代母文明的策略参数变化量，按指数衰减加权平均的结果
            // 之前的变异往策略参数较好的方向进行，所以之后的变异也要往这个方向进行
            // 同时要有一定随机性
            double stepCumu = 1.0;
            for (int k = 0; k < parentCount - 1; ++k)
            {
                stepCumu =
                    stepCumu * STEP_WEIGHT_REDUCE +
                    aiMap[civils[parentIdList[k]].planetId][i.first][j] -
                    aiMap[civils[parentIdList[k + 1]].planetId][i.first][j];
            }
            // 调节策略参数，变化量的数量级与stepCumu相同，方向也偏向stepCumu
            p[i.first][j] += (newRandom.getNormal() + 0.33) * stepCumu;
            // 防止过大
            if (abs(p[i.first][j]) > MAX_AI_MIX)
                p[i.first][j] = MAX_AI_MIX / p[i.first][j];
        }
    }
}

void Civil::mutateNaive()
{
    // 随机变异
    auto& p = aiMap[planetId];
    for (auto i : p)
    {
        for (int j = 0; j < MAX_AI_PARAM + 1; ++j)
        {
            p[i.first][j] += newRandom.getNormal() * p[i.first][j];
            // 防止过大
            if (abs(p[i.first][j]) > MAX_AI_MIX)
                p[i.first][j] = MAX_AI_MIX / p[i.first][j];
        }
    }
}
