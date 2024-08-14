#pragma once

#include "Geometry.h"
#include "Light.h"
#include "Math.h"
#include "Ray.h"

class Light
{
public:
    Light(const Color& ks, std::vector<float> intensityTable, Vec3f normal, float flux = -1);
    Color getColor() const;
    float getFlux() const;
    virtual ~Light()                     = default;
    virtual Ray   fireRay()              = 0;
    virtual Vec3f getRandomPointOfSurf() = 0;
    virtual Color calculateIlluminance(const Vec3f& surfPoint, const Vec3f& surfNormal, const Vec3f& lightPoint) = 0;

protected:
    std::vector<float> calculateIntegral();
    Vec3f              getDir();
    std::vector<float> intensityTableExtension(std::vector<float> intensityTable);
    int                binarySearch(const std::vector<float>& array, double value);
    double             randomFromRange(double maximum = 1., double minimum = 0.);
    bool               rejectionSampling(
                      double leftProbability, double rightProbability, double chosenAngle, double leftAngle, double rightAngle);

protected:
    float              flux_;
    Vec3f              normal_;
    std::vector<float> intensityTable_;
    Color              color_;
    std::vector<float> tabularProbabilities_;
    std::vector<Vec3f> rotationMatrix_;
};

class PointLight : public Light
{
public:
    PointLight(
        const Color&              ks,
        const std::vector<float>& intensityTable,
        const Vec3f&              origin,
        const Vec3f&              normal,
        float                     flux = -1);
    Ray   fireRay() override;
    Vec3f getRandomPointOfSurf() override;
    Color calculateIlluminance(const Vec3f& surfPoint, const Vec3f& surfNormal, const Vec3f& lightPoint) override;

private:
    Vec3f origin_;
};

class RectangleLight : public Light
{
public:
    RectangleLight(
        const Color&              ks,
        const std::vector<float>& intensityTable,
        const Vec3f&              origin,
        const Vec3f&              normal,
        const Mesh&               mesh,
        float                     flux = -1);
    Color calculateLuminance(const Vec3f& rayDir);
    Vec3f getRandomPointOfSurf() override;
    Ray   fireRay() override;
    Color calculateIlluminance(const Vec3f& surfPoint, const Vec3f& surfNormal, const Vec3f& lightPoint) override;

private:
    std::vector<float> luminanceTable_;
    Mesh               geometryMesh_;
};
