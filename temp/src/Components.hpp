#pragma once

#include "Vec2.hpp"
#include <SFML/Graphics.hpp>

class Component
{
public:
    bool exists = false;
};

class CTransform : public Component
{
public:
    Vec2f pos        = { 0.0, 0.0 };
    Vec2f velocity   = { 0.0, 0.0 };
    float angle     = 0;

    CTransform() = default;
    CTransform(const Vec2f & p, const Vec2f & v, float a) 
        : pos(p), velocity(v), angle(a) {}
};

class CShape : public Component
{
public:
    sf::CircleShape circle;

    CShape() = default;
    CShape(float radius, size_t points, const sf::Color & fill, const sf::Color & outline, float thickness)
        : circle(radius, points)
    {
        circle.setFillColor(fill);
        circle.setOutlineColor(outline);
        circle.setOutlineThickness(thickness);
        circle.setOrigin({ radius, radius });
    }
};

class CCollision : public Component
{
public:
    float radius = 0;

    CCollision() = default;
    CCollision(float r) 
        : radius(r) {}
};

class CScore : public Component
{
public:
    int score = 0;

    CScore() = default;
    CScore(int s) 
        : score(s) {}
};

class CLifespan : public Component
{
public:
    int lifespan     = 0;
    int remaining    = 0;
    CLifespan() = default;
    CLifespan(int totalLifespan) 
        : lifespan(totalLifespan), remaining(totalLifespan) {}
};

class CInput : public Component
{
public:
    bool up     = false;
    bool left   = false;
    bool right  = false;
    bool down   = false;
    bool shoot  = false;

    CInput() = default;
};
