// 网格

#ifndef MESH_CPP
#define MESH_CPP

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

const int MAX_MESH = 101;
const float CENTER_POS = float(MAX_MESH - 1) * 0.5;
const double MESH_SMALL_SCALE = 10.0;
const int MAX_MESH_SMALL = ceil(double(MAX_MESH) / MESH_SMALL_SCALE);

#define forMesh(i, j)                       \
    for (int(i) = 0; (i) < MAX_MESH; ++(i)) \
        for (int(j) = 0; (j) < MAX_MESH; ++(j))

template <class T>
class Mesh
{
   public:
    T mesh[MAX_MESH][MAX_MESH];

    // 不用初始化
    Mesh()
    {
    }

    // 重载()，方便读写mesh的值
    T& operator()(int i, int j)
    {
        return mesh[i][j];
    }

    // 取实数位置的值，用周围四个点线性插值
    T interpolar(double i, double j)
    {
        int iFloor = floor(i);
        int jFloor = floor(j);
        double iDecimal = i - iFloor;
        double jDecimal = j - jFloor;
        return (mesh[iFloor][jFloor] * (1 - iDecimal) +
                mesh[iFloor + 1][jFloor] * iDecimal) *
                   (1 - jDecimal) +
               (mesh[iFloor][jFloor + 1] * (1 - iDecimal) +
                mesh[iFloor + 1][jFloor + 1] * iDecimal) *
                   jDecimal;
    }

    // 取实数位置的值，只读
    T operator()(double i, double j)
    {
        return interpolar(i, j);
    }

    // 返回放大s倍的网格
    Mesh<T> scale(double s)
    {
        Mesh<T> res;
        double invS = 1.0 / s;
        forMesh(i, j) res(i, j) = interpolar(i * invS, j * invS);
        return res;
    }

    // 将mesh转换为json字符串
    string toJson()
    {
        stringstream ss;
        ss << "[";
        ss << "[";
        ss << setprecision(3) << mesh[0][0];
        for (int j = 1; j < MAX_MESH; ++j)
            ss << "," << setprecision(3) << mesh[0][j];
        ss << "]";
        for (int i = 0; i < MAX_MESH; ++i)
        {
            ss << ",[";
            ss << setprecision(3) << mesh[i][0];
            for (int j = 1; j < MAX_MESH; ++j)
                ss << "," << setprecision(3) << mesh[i][j];
            ss << "]";
        }
        ss << "]";

        string res;
        ss >> res;
        return res;
    }

    // 将mesh转换为多行字符串，用空格和换行符分隔，列宽固定
    string toTable()
    {
        stringstream ss;
        for (int i = 0; i < MAX_MESH; ++i)
        {
            for (int j = 0; j < MAX_MESH; ++j)
                ss << setprecision(3) << setw(6) << mesh[i][j];
            ss << endl;
        }
        ss << endl;

        string res;
        ss >> res;
        return res;
    }
};

#endif
