#include "SurfaceOpticProperties.h"

#include <random>

#define _USE_MATH_DEFINES
#include <math.h>

Kd::Kd(const Color& color, float coeff)
    : color(color)
    , coeff(coeff)
{}

Color Kd::CalculateLuminance(const Color& E, const Vec3f& U, const Vec3f& V, const Vec3f& N) const
{
    return (E * color) * coeff * (1 / M_PI);
}

Ray Kd::TransformRay(const Ray& ray, const Vec3f& N, const Vec3f& intersectionPoint) const
{
    static std::mt19937                          gen;
    static std::uniform_real_distribution<float> dist(0, 1);

    float R     = 1;
    float phi   = 2 * M_PI * dist(gen);
    float z     = -R + 2 * R * dist(gen);
    float theta = acosf(z / R);

    float x = sinf(theta) * cosf(phi) * R;
    float y = sinf(theta) * sinf(phi) * R;

    Vec3f direction(x, y, z);
    direction = direction + N;
    direction = direction.normalize();

    Ray transformedRay;

    transformedRay.direction       = direction;
    transformedRay.origin          = intersectionPoint;
    transformedRay.color           = color * ray.color * coeff;
    transformedRay.trash.lastEvent = TransformRayEvent::e_KD;
    if (transformedRay.color.sum() > 0)
        transformedRay.color = transformedRay.color * ray.color.sum() / transformedRay.color.sum();

    return transformedRay;
}

const Color& Kd::getColor() const
{
    return color;
}

float Kd::getCoeff() const
{
    return coeff;
}

void Kd::setColor(const Color& color)
{
    this->color = color;
}

void Kd::setCoeff(float coeff)
{
    this->coeff = coeff;
}

Ks::Ks(const Color& color, float coeff)
    : color(color)
    , coeff(coeff)
{}

Color Ks::CalculateLuminance(const Color& E, const Vec3f& U, const Vec3f& V, const Vec3f& N) const
{
    return Color();
}

Ray Ks::TransformRay(const Ray& ray, const Vec3f& N, const Vec3f& intersectionPoint) const
{
    Vec3f direction         = ray.direction;
    Vec3f specularDirection = direction - N * 2 * (direction * N);
    specularDirection       = specularDirection.normalize();

    Ray transformedRay;

    transformedRay.direction       = specularDirection;
    transformedRay.origin          = intersectionPoint;
    transformedRay.color           = color * ray.color * coeff;
    transformedRay.trash.lastEvent = TransformRayEvent::e_KS;
    if (transformedRay.color.sum() > 0)
        transformedRay.color = transformedRay.color * ray.color.sum() / transformedRay.color.sum();

    return transformedRay;
}

const Color& Ks::getColor() const
{
    return color;
}

float Ks::getCoeff() const
{
    return coeff;
}

void Ks::setColor(const Color& color)
{
    this->color = color;
}

void Ks::setCoeff(float coeff)
{
    this->coeff = coeff;
}

Ktd::Ktd(const Color& color, float coeff)
    : color(color)
    , coeff(coeff)
{}

Color Ktd::CalculateLuminance(const Color& E, const Vec3f& U, const Vec3f& V, const Vec3f& N) const
{
    return Color();
}

Ray Ktd::TransformRay(const Ray& ray, const Vec3f& N, const Vec3f& intersectionPoint) const
{
    static std::mt19937                          gen;
    static std::uniform_real_distribution<float> dist(0, 1);

    float R     = 1;
    float phi   = 2 * M_PI * dist(gen);
    float z     = -R + 2 * R * dist(gen);
    float theta = acosf(z / R);

    float x = sinf(theta) * cosf(phi) * R;
    float y = sinf(theta) * sinf(phi) * R;

    Vec3f direction(x, y, z);
    direction = direction - N;
    direction = direction.normalize();

    Ray transformedRay;

    transformedRay.direction = direction;
    transformedRay.origin    = intersectionPoint;
    transformedRay.color     = color * ray.color * coeff;
    if (transformedRay.color.sum() > 0)
        transformedRay.color = transformedRay.color * ray.color.sum() / transformedRay.color.sum();
    transformedRay.trash.lastEvent = TransformRayEvent::e_KTD;

    return transformedRay;
}

const Color& Ktd::getColor() const
{
    return color;
}

float Ktd::getCoeff() const
{
    return coeff;
}

void Ktd::setColor(const Color& color)
{
    this->color = color;
}

void Ktd::setCoeff(float coeff)
{
    this->coeff = coeff;
}

Kts::Kts(const Color& color, float coeff)
    : color(color)
    , coeff(coeff)
{}

Color Kts::CalculateLuminance(const Color& E, const Vec3f& U, const Vec3f& V, const Vec3f& N) const
{
    return Color();
}

Ray Kts::TransformRay(const Ray& ray, const Vec3f& N, const Vec3f& intersectionPoint) const
{
    float nAir = 1.f;

    Vec3f I = ray.direction;

    float n1 = ray.envProp;
    float n2 = environmentProperty; // надо проверить

    float cosinus = -std::max(-1.f, std::min(1.f, I * N));

    if (cosinus < 0)
    { // луч выходит из объекта в воздух
        cosinus = -std::max(-1.f, std::min(1.f, I * (-N)));
        n2      = nAir;
    }

    float eta = n1 / n2;

    float maxRefractionAngle = std::asin(1 / eta); // максимальный угол преломления

    Ray transformedRay;

    if (maxRefractionAngle < std::acos(cosinus))
    {
        Vec3f direction         = ray.direction;
        Vec3f specularDirection = direction - N * 2 * (direction * N);
        specularDirection       = specularDirection.normalize();

        Ray transformedRay;

        transformedRay.direction       = specularDirection;
        transformedRay.origin          = intersectionPoint;
        transformedRay.color           = color * ray.color * coeff;
        transformedRay.trash.lastEvent = TransformRayEvent::e_KS;
        if (transformedRay.color.sum() > 0)
            transformedRay.color = transformedRay.color * ray.color.sum() / transformedRay.color.sum();

        return transformedRay;
    }

    float k = 1 - eta * eta * (1 - cosinus * cosinus);

    if (k < 0)
    {
        transformedRay.direction = Vec3f(1, 0, 0);
    }
    else
    {
        transformedRay.direction = I * eta + N * (eta * cosinus - sqrtf(k));
    }

    transformedRay.envProp = n2;
    transformedRay.origin  = intersectionPoint;
    transformedRay.color   = color * ray.color * coeff;
    if (transformedRay.color.sum() > 0)
        transformedRay.color = transformedRay.color * ray.color.sum() / transformedRay.color.sum();
    transformedRay.trash.lastEvent = TransformRayEvent::e_KTS;

    return transformedRay;
}

const Color& Kts::getColor() const
{
    return color;
}

float Kts::getCoeff() const
{
    return coeff;
}

void Kts::setColor(const Color& color)
{
    this->color = color;
}

void Kts::setCoeff(float coeff)
{
    this->coeff = coeff;
}
