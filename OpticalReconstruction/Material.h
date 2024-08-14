#pragma once

#include "BRDF.h"
#include "SurfaceOpticProperties.h"

#include <map>

enum coefs
{
    kd,
    //ks,
    //ktd,
    //kts
    //brdf
};

class Material
{
public:
    Material() = default;
    Material(Color& color, std::map<coefs, float> optical_coef_float_map);
    Material(const Material& other);
    ~Material();

    Material& operator=(const Material& other);

public:
    Color                 CalculateLuminance(const Color& E, const Vec3f& U, const Vec3f& V, const Vec3f& N) const;
    Ray                   TransformRay(const Ray& ray, const Vec3f& N, const Vec3f& intersectionPoint) const;
    SurfaceOpticProperty* chooseEvent(const Ray& ray) const;
    void                  NormalizeCoefs(SurfaceOpticProperty* prior_coef = nullptr);

private:
    // Вспомогательная функция для глубокого копирования
    void deepCopy(const std::map<coefs, SurfaceOpticProperty*>& sourceMap);

public:
    std::map<coefs, SurfaceOpticProperty*> optical_coef_map;
};
