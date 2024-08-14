#pragma once

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

template<size_t DIM, class T>
struct vec
{
    vec()
    {
        for (size_t i = DIM; i--; data_[i] = T())
            ;
    }

    T& operator[](const size_t i)
    {
        assert(i < DIM);
        return data_[i];
    }

    const T& operator[](const size_t i) const
    {
        assert(i < DIM);
        return data_[i];
    }

private:
    T data_[DIM];
};

typedef vec<2, float>  Vec2f;
typedef vec<2, double> Vec2d;
typedef vec<3, float>  Vec3f;
typedef vec<3, double> Vec3d;

template<class T>
struct vec<2, T>
{
    vec()
        : x(T())
        , y(T())
    {}

    vec(T X, T Y)
        : x(X)
        , y(Y)
    {}

    const T& operator[](const size_t i) const
    {
        assert(i < 2 && i >= 0);
        if (i == 0)
            return x;
        else if (i == 1)
            return y;
    }

    T& operator[](const size_t i)
    {
        return const_cast<T&>(static_cast<const vec<2, T>&>(*this)[i]);
    }

    T x, y;
};

template<class T>
struct vec<3, T>
{
    vec()
        : x(T())
        , y(T())
        , z(T())
    {}

    vec(T X, T Y, T Z)
        : x(X)
        , y(Y)
        , z(Z)
    {}

    const T& operator[](const size_t i) const
    {
        assert(i < 3 && i >= 0);

        switch (i)
        {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        }
    }

    T& operator[](const size_t i)
    {
        return const_cast<T&>(static_cast<const vec<3, T>&>(*this)[i]);
    }

    T norm()
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    vec<3, T>& normalize(T l = 1)
    {
        *this = (*this) * (l / norm());
        return *this;
    }

    T length() const
    {
        return sqrt(x * x + y * y + z * z);
    }

    T x, y, z;
};

template<size_t DIM, class T>
bool operator==(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs)
{
    if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z)
        return true;
    return false;
}

template<size_t DIM, class T>
T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs)
{
    T ret = T();
    for (size_t i = DIM; i--; ret += lhs[i] * rhs[i])
        ;
    return ret;
}

template<size_t DIM, class T>
vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs)
{
    for (size_t i = DIM; i--; lhs[i] += rhs[i])
        ;
    return lhs;
}

template<size_t DIM, class T>
vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs)
{
    for (size_t i = DIM; i--; lhs[i] -= rhs[i])
        ;
    return lhs;
}

template<size_t DIM, class T, typename U>
vec<DIM, T> operator*(const vec<DIM, T>& lhs, const U& rhs)
{
    vec<DIM, T> ret;
    for (size_t i = DIM; i--; ret[i] = lhs[i] * rhs)
        ;
    return ret;
}

template<size_t DIM, class T>
vec<DIM, T> operator-(const vec<DIM, T>& lhs)
{
    return lhs * T(-1);
}

template<class T>
vec<3, T> cross(vec<3, T> v1, vec<3, T> v2)
{
    return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

template<size_t DIM, class T>
std::ostream& operator<<(std::ostream& out, const vec<DIM, T>& v)
{
    for (unsigned int i = 0; i < DIM; i++)
        out << v[i] << " ";

    return out;
}
