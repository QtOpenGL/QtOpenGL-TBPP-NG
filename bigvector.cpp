// 存储大量元素
// 最多CACHE_SIZE * MAX_CACHE = 327680000个

#ifndef BIGVECTOR_CPP
#define BIGVECTOR_CPP

#include <iostream>

using namespace std;

const int CACHE_SIZE_BIT = 15;
const size_t CACHE_SIZE = 1 << CACHE_SIZE_BIT;
const size_t CACHE_MASK = CACHE_SIZE - 1;
const size_t MAX_CACHE = 10000;

// 要求T有默认构造函数
template <class T>
class BigVectorCache
{
   public:
    T data[CACHE_SIZE];

    BigVectorCache()
    {
    }

    T& operator[](size_t i)
    {
        return data[i];
    }
};

template <class T>
class BigVector
{
   public:
    BigVectorCache<T>* cache[MAX_CACHE];
    size_t _size;

    BigVector() : _size(0)
    {
    }

    ~BigVector()
    {
        for (size_t i = 0; i < (_size >> CACHE_SIZE_BIT) + 1; ++i)
            delete cache[i];
    }

    size_t size()
    {
        return _size;
    }

    T& operator[](size_t i)
    {
        return (*cache[i >> CACHE_SIZE_BIT])[i & CACHE_MASK];
    }

    void push_back(T t)
    {
        if (!(_size & CACHE_MASK))
        {
            cache[_size >> CACHE_SIZE_BIT] = new BigVectorCache<T>;
        }
        (*cache[_size >> CACHE_SIZE_BIT])[_size & CACHE_MASK] = t;
        ++_size;
    }
};

#endif
