#pragma once

#include <iostream>

#include "Color.h"
#include "Math.h"

enum TransformRayEvent
{
    e_KD,
    e_KS,
    e_KTD,
    e_KTS,
    e_BRDF,
    e_KILL,
    e_START
};

struct Container
{
    TransformRayEvent lastEvent = e_START;
};

struct Ray
{
    Vec3f     origin;
    Vec3f     direction;
    float     ks;
    Color     color;
    Container trash;
    float     envProp = 1.0f;

    Ray(const Vec3f& origin, const Vec3f& direction, float ks = 1)
        : origin(origin)
        , direction(direction)
        , ks(ks)
        , color(Color())
    {}
    Ray()
    {}

    Ray(const Ray& other)
        : origin(other.origin)
        , direction(other.direction)
        , ks(other.ks)
        , color(other.color)
        , trash(other.trash)
        , envProp(other.envProp)
    {}

    Ray& operator=(const Ray& other)
    {
        if (this != &other)
        {
            origin    = other.origin;
            direction = other.direction;
            ks        = other.ks;
            color     = other.color;
            trash     = other.trash;
            envProp   = other.envProp;
        }
        return *this;
    }
};

struct RayPath
{
    std::vector<Vec3f> origins;
    std::vector<Vec3f> directions;
};
