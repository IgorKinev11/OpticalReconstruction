#include "Geometry.h"
#include <random>

#define _USE_MATH_DEFINES
#include <math.h>

Triangle::Triangle(Mesh* mesh, int v1, int v2, int v3)
    : mesh(mesh)
    , v1(v1)
    , v2(v2)
    , v3(v3)
{}

bool Triangle::hitTest(const Ray& ray, float& t, Vec3f& N) const
{
    Vec3f e1   = mesh->ownPoints_[v2] - mesh->ownPoints_[v1];
    Vec3f e2   = mesh->ownPoints_[v3] - mesh->ownPoints_[v1];
    Vec3f pvec = cross(ray.direction, e2);
    float det  = e1 * pvec;

    if (det < 1e-8 && det > -1e-8)
        return false;

    float inv_det = 1 / det;
    Vec3f tvec    = ray.origin - mesh->ownPoints_[v1];
    float u       = (tvec * pvec) * inv_det;
    if (u < 0 || u > 1)
        return false;

    Vec3f qvec = cross(tvec, e1);
    float v    = (ray.direction * qvec) * inv_det;
    if (v < 0 || v + u > 1)
        return false;

    float dist = (e2 * qvec) * inv_det;
    if (dist < t && dist > 1e-8)
    {
        t = dist;
        N = getNormal(-ray.direction);
        return true;
    }
    return false;
}

Vec3f Triangle::getNormal(const Vec3f& direction) const
{
    Vec3f N =
        cross(mesh->ownPoints_[v2] - mesh->ownPoints_[v1], mesh->ownPoints_[v3] - mesh->ownPoints_[v1]).normalize();
    return N * direction < 0 ? -N : N;
}

bool Triangle::isInside(const Vec3f& point) const
{
    Vec3f edge0 = mesh->ownPoints_[v2] - mesh->ownPoints_[v1];
    Vec3f edge1 = mesh->ownPoints_[v3] - mesh->ownPoints_[v1];

    Vec3f C0 = point - mesh->ownPoints_[v1];
    Vec3f C1 = point - mesh->ownPoints_[v2];
    Vec3f C2 = point - mesh->ownPoints_[v3];

    float S = cross(edge0, edge1).length() / 2;

    float S1 = cross(C0, C1).length() / 2;
    float S2 = cross(C1, C2).length() / 2;
    float S3 = cross(C2, C0).length() / 2;

    return fabs(S - (S1 + S2 + S3)) < 1e-6;
}

float Triangle::getArea() const
{
    Vec3f edge0 = mesh->ownPoints_[v2] - mesh->ownPoints_[v1];
    Vec3f edge1 = mesh->ownPoints_[v3] - mesh->ownPoints_[v1];

    return cross(edge0, edge1).length() / 2;
}

Vec3f Triangle::randomSurfPoint() const
{
    static std::mt19937                          gen;
    static std::uniform_real_distribution<float> dist(0, 1);

    Vec3f edge0 = mesh->ownPoints_[v2] - mesh->ownPoints_[v1];
    Vec3f edge1 = mesh->ownPoints_[v3] - mesh->ownPoints_[v1];

    float xi1 = dist(gen);
    float xi2 = dist(gen);
    if (xi1 + xi2 > 1)
    {
        xi1 = 1 - xi1;
        xi2 = 1 - xi2;
    }

    return edge0 * xi1 + edge1 * xi2 + mesh->ownPoints_[v1];
}

bool Mesh::hitTest(const Ray& ray, float& t, Vec3f& N) const
{
    bool result = false;

    for (const auto& triangle : triangles_)
    {
        if (triangle.hitTest(ray, t, N))
            result = true;
    }

    return result;
}

Vec3f Mesh::randomSurfPoint() const
{
    static std::mt19937                          gen;
    static std::uniform_real_distribution<float> dist(0, 1);

    std::vector<float> areas(triangles_.size());
    areas[0] = 0;
    for (int i = 1; i < triangles_.size(); ++i)
    {
        areas[i] = areas[i - 1] + triangles_[i - 1].getArea();
    }

    float xi = dist(gen) * (areas.back() + triangles_.back().getArea());
    int   l = 0, r = areas.size();
    while (r - l > 1)
    {
        int m = (l + r) / 2;
        if (areas[m] < xi)
        {
            l = m;
        }
        else
        {
            r = m;
        }
    }

    return triangles_[l].randomSurfPoint();
}

float Mesh::getSurfaceArea() const
{
    float area = 0.0;
    for (const auto& triangle : triangles_)
        area += triangle.getArea();

    return area;
}

Sphere::Sphere(const Vec3f& center, float radius)
    : center_(center)
    , radius_(radius)
{}

bool Sphere::hitTest(const Ray& ray, float& t, Vec3f& N) const
{
    const Vec3f oc   = ray.origin - center_;
    const float a    = ray.direction * ray.direction;
    const float b    = 2 * (oc * ray.direction);
    const float c    = oc * oc - radius_ * radius_;
    float       disc = b * b - 4 * a * c;

    if (disc < 1e-4)
        return false;

    disc           = sqrt(disc);
    const float t0 = (-b - disc) / (2 * a);
    const float t1 = (-b + disc) / (2 * a);
    t              = (t0 < t1) ? t0 : t1;

    Vec3f intersectionPoint = ray.origin + ray.direction * t;
    N                       = (intersectionPoint - center_).normalize();

    return true;
}

Vec3f Sphere::randomSurfPoint() const
{
    static std::mt19937                          gen;
    static std::uniform_real_distribution<float> dist(0, 1);

    float xi1 = dist(gen);
    float xi2 = dist(gen);

    float phi   = 2 * M_PI * xi1;
    float z     = -radius_ + 2 * radius_ * xi2;
    float theta = acos(z / radius_);
    float x     = radius_ * sin(theta) * cos(phi);
    float y     = radius_ * sin(theta) * sin(phi);

    return Vec3f(x, y, z);
}

float Sphere::getSurfaceArea() const
{
    return 4 * M_PI * radius_ * radius_;
}
