#pragma once

#include "Ray.h"

class Camera
{
public:
    Camera(
        int   id,
        Vec3f eye,
        Vec3f center,
        Vec3f leftUpCorner,
        Vec3f rightUpCorner,
        Vec3f leftDownCorner,
        int   width,
        int   height,
        float fov);
    ~Camera() = default;

    Ray castRay(int X = -1, int Y = -1);
    // Ray castRayOnDir(Vec3f& dirN);

    std::vector<std::vector<Ray>> render();

    int   getId() const;
    Vec3f getEye() const;
    Vec3f getCenter() const;
    Vec3f getLeftUpCorner() const;
    Vec3f getRightUpCorner() const;
    Vec3f getLeftDownCorner() const;
    int   getWidth() const;
    int   getHeight() const;
    float getFov() const;

private:
    int         id;
    const Vec3f eye;
    const Vec3f center;
    const Vec3f leftUpCorner;
    const Vec3f rightUpCorner;
    const Vec3f leftDownCorner;
    const int   WIDTH;
    const int   HEIGHT;
    const float fov;
};