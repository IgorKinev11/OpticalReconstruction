#include "BRDF.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <random>

BRDF::BRDF(const Color& color, float coef, float e)
    : color(color)
    , coeff(coeff)
    , e(e)
{
    float integral = 2 * M_PI / (e + 1); // cos(theta) ^ e * sin(theta) * dtheta *
                                         // dphi (phi - (0, 360), theta - (0, 90))
    c = coeff / integral;                // коэффициент нормализации
}

Color BRDF::CalculateLuminance(const Color& E, const Vec3f& U, const Vec3f& V, const Vec3f& N) const
{
    // FIXME угол между зеркально отраженным лучом наблюдения и направлением
    // освещения
    Vec3f reflectedU = U - N * 2 * (U * N);
    reflectedU       = reflectedU.normalize();
    float cosTheta   = reflectedU * V;
    if (cosTheta < 0)
        cosTheta = 0;
    return (color * E) * coeff * powf(cosTheta, e) / M_PI; // (25.2) Ray Tracing from Ground Up
}

Ray BRDF::TransformRay(const Ray& ray, const Vec3f& N, const Vec3f& intersectionPoint) const
{
    static std::mt19937                          gen;
    static std::uniform_real_distribution<float> dist(0, 1);

    // Направление зеркального отражения
    Vec3f specularDirection = ray.direction - N * 2 * (ray.direction * N);
    specularDirection       = specularDirection.normalize();

    // Оси новой системы координат
    Vec3f Z = specularDirection;
    Vec3f X = cross(Z, N).normalize();
    Vec3f Y = cross(Z, X).normalize();

    // генерируем луч
    float phi      = 2 * M_PI * dist(gen);
    float cosTheta = powf(1 - dist(gen), 1 + e);
    float sinTheta = sqrtf(1 - cosTheta * cosTheta);

    Vec3f direction(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
    direction = direction.normalize();

    // Переходим в глобальную систему координат
    float x = direction.x * X.x + direction.y * Y.x + direction.z * Z.x;
    float y = direction.y * X.y + direction.y * Y.y + direction.y * Z.y;
    float z = direction.z * X.z + direction.z * Y.z + direction.z * Z.z;

    direction = Vec3f(x, y, z).normalize();

    Ray transformedRay;
    transformedRay.direction       = direction;
    transformedRay.origin          = intersectionPoint;
    transformedRay.trash.lastEvent = TransformRayEvent::e_BRDF;
    transformedRay.color           = ray.color * color;
    if (transformedRay.color.sum() > 0)
        transformedRay.color = transformedRay.color * ray.color.sum() / transformedRay.color.sum();

    if (direction * N < 0)
        transformedRay.trash.lastEvent = TransformRayEvent::e_KILL;

    return transformedRay;
}

const Color& BRDF::getColor() const
{
    return color;
}

float BRDF::getCoeff() const
{
    return coeff;
}

void BRDF::setColor(const Color& color)
{
    this->color = color;
}

void BRDF::setCoeff(float coeff)
{
    this->coeff = coeff;

    // Пересчитываем c // TODO: fix it
    float integral = 2 * M_PI / (e + 1); // cos(theta) ^ e * sin(theta) * dtheta *
                                         // dphi (phi - (0, 360), theta - (0, 90))
    c = coeff / integral;                // коэффициент нормализации
}