// 杂项

#ifndef UTILS_H
#define UTILS_H

#include <ctime>
#include <random>

using namespace std;

#define sqr(x) ((x) * (x))

// 包装一对double
class Point
{
   public:
    double x, y;

    // 默认构造函数
    Point()
    {
    }

    Point(double _x, double _y) : x(_x), y(_y)
    {
    }

    Point operator*(double s)
    {
        return Point(x * s, y * s);
    }
};

// 包装与随机数有关的函数
class NewRandom
{
   private:
    // 使用Mersenne Twister算法生成随机数，统计性质比rand()更好
    // 将时间hash后作为Mersenne Twister算法的初始种子
    mt19937 gen;
    uniform_real_distribution<double> dist;
    normal_distribution<double> normalDist;

    unsigned int hash(unsigned int x)
    {
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return x;
    }

   public:
    NewRandom() : gen(hash(time(nullptr))), dist(0.0, 1.0), normalDist(0.0, 1.0)
    {
    }

    // 返回(0, 1)之间均匀分布的随机实数
    double get()
    {
        return dist(gen);
    }

    // 返回平均数为0，标准差为1的正态分布的随机实数
    double getNormal()
    {
        return normalDist(gen);
    }

    // 返回圆心为(0.5, 0.5)，半径为0.5的圆内的两个随机实数
    // TODO：用高斯取样法提高效率
    Point getPoint()
    {
        while (true)
        {
            double x = get(), y = get();
            if (sqr(x - 0.5) + sqr(y - 0.5) < 0.25) return Point(x, y);
        }
    }
};

extern NewRandom newRandom;

// 快速但不精确的倒数
__inline__ float __attribute__((const)) reciprocal(float x)
{
    union {
        float dbl;
        unsigned uint;
    } u;
    u.dbl = x;
    u.uint = (0xbe6eb3beU - u.uint) >> (unsigned char)1;
    u.dbl *= u.dbl;
    return u.dbl;
}

__inline__ double __attribute__((const)) reciprocal(double x)
{
    union {
        double dbl;
        unsigned long long ull;
    } u;
    u.dbl = x;
    u.ull = (0xbfcdd6a18f6a6f52ULL - u.ull) >> (unsigned char)1;
    u.dbl *= u.dbl;
    return u.dbl;
}

#endif
