#pragma once

#include "Math.h"
#include "Ray.h"

#include <vector>

class Mesh;
class Light;

struct Triangle
{
    Triangle(Mesh* mesh, int v1, int v2, int v3);

    bool  hitTest(const Ray& ray, float& t, Vec3f& N) const;
    Vec3f getNormal(const Vec3f& direction) const;
    bool  isInside(const Vec3f& point) const;
    float getArea() const;
    Vec3f randomSurfPoint() const;

    Mesh* mesh;
    int   v1, v2, v3;
};

class Geometry
{
public:
    virtual Vec3f randomSurfPoint() const                           = 0;
    virtual bool  hitTest(const Ray& ray, float& t, Vec3f& N) const = 0;
    virtual float getSurfaceArea() const                            = 0;
    virtual ~Geometry()                                             = default;

public:
    int    materialId_;
    Light* sourceLight_ = nullptr;
};

class Mesh : public Geometry
{
public:
    Vec3f randomSurfPoint() const override;
    bool  hitTest(const Ray& ray, float& t, Vec3f& N) const override;
    float getSurfaceArea() const override;

public:
    std::vector<Triangle> triangles_;
    std::vector<Vec3f>    ownPoints_;
};

class Sphere : public Geometry
{
public:
    Sphere()
    {}
    Sphere(const Vec3f& center, float radius);

    Vec3f randomSurfPoint() const override;
    bool  hitTest(const Ray& ray, float& t, Vec3f& N) const override;
    float getSurfaceArea() const override;

public:
    Vec3f center_;
    float radius_ = 0;
};
