#pragma once

#include <SFML/Graphics.hpp>
#include <math.h>

template <typename T>
class Vec2
{
public:

    T x = 0;
    T y = 0;

    Vec2() = default;

    Vec2(T xin, T yin)
        : x(xin), y(yin)
    { }

    // constructor to convert from sf::Vector2
    Vec2(const sf::Vector2<T>& vec)
        : x(vec.x), y(vec.y)
    { }
	
	Vec2(const sf::Vector2i& vec)
    : x(T(vec.x)), y(T(vec.y))
	{
	}

    // allow automatic conversion to sf::Vector2
    // this lets us pass Vec2 into sfml functions
    operator sf::Vector2<T>()
    {
        return sf::Vector2<T>(x, y);
    }

    Vec2 operator + (const Vec2& rhs) const
    {
        // TODO
        return Vec2();
    }

    Vec2 operator - (const Vec2& rhs) const
    {
        // TODO
        return Vec2();
    }

    Vec2 operator / (const T val) const
    {
        // TODO
        return Vec2();
    }

    Vec2 operator * (const T val) const
    {
        // TODO
        return Vec2();
    }

    bool operator == (const Vec2& rhs) const
    {
        // TODO
        return false;
    }

    bool operator != (const Vec2& rhs) const
    {
        // TODO
        return false;
    }

    void operator += (const Vec2& rhs)
    {
        // TODO
    }

    void operator -= (const Vec2& rhs)
    {
        // TODO
    }

    void operator *= (const T val)
    {
        // TODO
    }

    void operator /= (const T val)
    {
        // TODO
    }

    float dist(const Vec2& rhs) const
    {
        // TODO
    }

    float length() const
    {
        // TODO
    }

    void normalize()
    {
        // TODO
    }
};

using Vec2f = Vec2<float>;