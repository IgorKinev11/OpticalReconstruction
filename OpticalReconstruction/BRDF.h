#pragma once

#include "SurfaceOpticProperties.h"

class BRDF : public SurfaceOpticProperty
{
public:
    BRDF(const Color& color, float coef, float e);

    Color CalculateLuminance(const Color& E, const Vec3f& U, const Vec3f& V, const Vec3f& N) const override;
    Ray   TransformRay(const Ray& ray, const Vec3f& N, const Vec3f& intersectionPoint) const override;

    const Color& getColor() const;
    float        getCoeff() const;

    void setColor(const Color& color);
    void setCoeff(float coeff);

private:
    Color color;
    float coeff; // kr
    float e;
    float c; // normalization constant
};
