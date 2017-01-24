#include <cmath>
#include <iostream>

using namespace std;

const double PI = acos(-1);
// const double RS_CONST = 1.484942765e-27;
const double RS_CONST = 1.0;
const int MAX_BISEC_ITER = 10;
const double BISEC_RANGE = 1.0e-6;

#define sqr(x) ((x) * (x))

class Sphere
{
   public:
    double x;
    double y;
    double m;
    double rg;
    double t;

    Sphere()
    {
    }

    Sphere(double _x, double _y, double _m, double _rg, double _t)
        : x(_x), y(_y), m(_m), rg(_rg), t(_t)
    {
    }
};

class Point
{
   public:
    double x;
    double y;
    double t;

    Point()
    {
    }

    Point(double _x, double _y, double _t) : x(_x), y(_y), t(_t)
    {
    }
};

class Pointr
{
   public:
    double r;
    double theta;
    double t;

    Pointr()
    {
    }

    Pointr(double _r, double _theta, double _t) : r(_r), theta(_theta), t(_t)
    {
    }
};

double f(double m, double rg, double rs, double r)
{
    return r * sqrt(1.0 - rs / r) +
           0.5 * rs * log(-rs + 2.0 * r * (1.0 + sqrt(1.0 - rs / r))) -
           0.5 * rs *
               log(-rs +
                   2.0 * rg * sqrt(rg / rs) *
                       (1.0 + sqrt(1.0 - (rs / (sin(sqrt(rs / rg)) *
                                                sqrt(sqr(rg) * rg / rs))))) *
                       sin(sqrt(rs / rg))) -
           rg * sqrt(rg / rs) *
               sqrt(1.0 -
                    (rs / (sin(sqrt(rs / rg)) * sqrt(sqr(rg) * rg / rs)))) *
               sin(sqrt(rs / rg)) +
           rg;
}

double solve(double m, double rg, double rs, double r0)
{
    double r1, r2, rMid, rNew;
    r1 = 0.0;
    r2 = r0;
    for (int i = 0; i < MAX_BISEC_ITER; ++i)
    {
        rMid = (r1 + r2) * 0.5;
        rNew = f(m, rg, rs, rMid);
        if (rNew - r0 > BISEC_RANGE)
            r2 = rMid;
        else if (rNew - r0 < -BISEC_RANGE)
            r1 = rMid;
        else
            break;
    }
    return rNew;
}

Pointr fromSphere(const Sphere& a, const Point& b)
{
    Pointr c;
    c.r = sqrt(sqr(a.x - b.x) + sqr(a.y - b.y));
    c.theta = atan2(b.y - a.y, b.x - a.x);
    c.t = b.t;
    return c;
}

Pointr curv(const Sphere& a, const Pointr& b)
{
    double rs = RS_CONST * a.m;
    Pointr c;
    c.r = sqrt(1.0 - rs / solve(a.m, a.rg, rs, b.r));
    c.theta = b.theta;
    c.t = 1.0 / sqrt(1.0 - rs / (b.r * c.r));
    return c;
}

Point totalCurv(int n, Sphere* a, Point b)
{
    Pointr c[n];
    for (int i = 0; i < n; i++) c[i] = curv(a[i], fromSphere(a[i], b));
    double x = 1.0, y = 1.0, t = b.t;
    for (int i = 0; i < n; i++)
    {
        x *= sqrt(sqr(sin(c[i].theta)) + sqr(c[i].r) * sqr(cos(c[i].theta)));
        y *= sqrt(sqr(cos(c[i].theta)) + sqr(c[i].r) * sqr(sin(c[i].theta)));
        t *= c[i].t;
    }
    return Point(x, y, t);
}

double d(const Point& b, const Point& c)
{
    return sqrt(sqr(b.x - c.x) + sqr(b.y - c.y));
}

double getDis(Point (*totalCurv)(int, Sphere*, Point), int n, Sphere* a,
              Point b, Point c, double dr)
{
    double s = 0.0;
    double theta = atan2(c.y - b.y, c.x - b.x);
    double initR = d(b, c);
    for (double rr = 0.0; rr < initR; rr += dr)
    {
        Point e(b.x + rr * cos(theta), b.y + rr * sin(theta), 1.0);
        Point d = totalCurv(n, a, e);
        s += sqrt(sqr(d.x) * sqr(cos(theta)) + sqr(d.y) * sqr(sin(theta))) * dr;
    }
    return s;
}

int main()
{
    int n, k;
    cout << "input n, k" << endl;
    cin >> n >> k;
    Sphere a[n];
    for (int i = 0; i < n; i++)
        a[i] = Sphere(100.0 * cos(2.0 * PI / n * i),
                      100.0 * sin(2.0 * PI / n * i), 1.0, 2.0, 1.0);
    Point b[k], c[k];
    for (int i = 0; i < k; i++)
        b[i] = Point(10.0 * cos(2.0 * PI / k * i), 10.0 * sin(2.0 * PI / k * i),
                     1.0);
    for (int i = 0; i < k; i++) c[i] = totalCurv(n, a, b[i]);
    for (int i = 0; i < k - 1; i++)
        for (int j = i + 1; j < k; j++)
            cout << "b[" << i << "] to b[" << j << "] is "
                 << getDis(totalCurv, n, a, b[i], b[j], 1.0e-3) << endl;
    for (int i = 0; i < k; i++) cout << c[i].t << endl;
    return 0;
}
