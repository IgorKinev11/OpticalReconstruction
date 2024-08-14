#pragma once

#include "Math.h"

inline std::vector<Vec3f> getRotationMatrix(Vec3f direct, Vec3f dirToMatrix)
{
    direct      = direct.normalize();
    dirToMatrix = dirToMatrix.normalize();

    std::vector<Vec3f> matrixOfRotation(3);

    if (direct == dirToMatrix)
    {
        matrixOfRotation[0] = {1, 0, 0};
        matrixOfRotation[1] = {0, 1, 0};
        matrixOfRotation[2] = {0, 0, 1};

        return matrixOfRotation;
    }

    if (direct == -dirToMatrix)
    {
        matrixOfRotation[0] = {-1, 0, 0};
        matrixOfRotation[1] = {0, -1, 0};
        matrixOfRotation[2] = {0, 0, -1};

        return matrixOfRotation;
    }

    Vec3f v = Vec3f(
        direct.y * dirToMatrix.z - direct.z * dirToMatrix.y, // cross multiplication
        direct.z * dirToMatrix.x - direct.x * dirToMatrix.z, direct.x * dirToMatrix.y - direct.y * dirToMatrix.x);
    float c              = direct.x * dirToMatrix.x + direct.y * dirToMatrix.y + direct.z * dirToMatrix.z; // dot mul
    float s              = sqrt(v.x * v.x + v.y * v.y + v.z * v.z); // norm or length of vector
    Vec3f kmat[3]        = {Vec3f{0, -v.z, v.y}, Vec3f{v.z, 0, -v.x}, Vec3f{-v.y, v.x, 0}};
    Vec3f kmatDotKmat[3] = {
        Vec3f{
            kmat[0].x * kmat[0].x + kmat[0].y * kmat[1].x + kmat[0].z * kmat[2].x, // kmatDotKmat = kmat * kmat
            kmat[0].x * kmat[0].y + kmat[0].y * kmat[1].y + kmat[0].z * kmat[2].y,
            kmat[0].x * kmat[0].z + kmat[0].y * kmat[1].z + kmat[0].z * kmat[2].z},
        Vec3f{
            kmat[1].x * kmat[0].x + kmat[1].y * kmat[1].x + kmat[1].z * kmat[2].x,
            kmat[1].x * kmat[0].y + kmat[1].y * kmat[1].y + kmat[1].z * kmat[2].y,
            kmat[1].x * kmat[0].z + kmat[1].y * kmat[1].z + kmat[1].z * kmat[2].z},
        Vec3f{
            kmat[2].x * kmat[0].x + kmat[2].y * kmat[1].x + kmat[2].z * kmat[2].x,
            kmat[2].x * kmat[0].y + kmat[2].y * kmat[1].y + kmat[2].z * kmat[2].y,
            kmat[2].x * kmat[0].z + kmat[2].y * kmat[1].z + kmat[2].z * kmat[2].z}};
    matrixOfRotation[0] = Vec3f{
        float(1.0 + kmat[0].x + kmatDotKmat[0].x * ((1.0 - c) / (s * s))),
        float(0.0 + kmat[0].y + kmatDotKmat[0].y * ((1.0 - c) / (s * s))),
        float(0.0 + kmat[0].z + kmatDotKmat[0].z * ((1.0 - c) / (s * s)))};
    matrixOfRotation[1] = Vec3f{
        float(0.0 + kmat[1].x + kmatDotKmat[1].x * ((1.0 - c) / (s * s))),
        float(1.0 + kmat[1].y + kmatDotKmat[1].y * ((1.0 - c) / (s * s))),
        float(0.0 + kmat[1].z + kmatDotKmat[1].z * ((1.0 - c) / (s * s)))};
    matrixOfRotation[2] = Vec3f{
        float(0.0 + kmat[2].x + kmatDotKmat[2].x * ((1.0 - c) / (s * s))),
        float(0.0 + kmat[2].y + kmatDotKmat[2].y * ((1.0 - c) / (s * s))),
        float(1.0 + kmat[2].z + kmatDotKmat[2].z * ((1.0 - c) / (s * s)))};

    return matrixOfRotation;
}
