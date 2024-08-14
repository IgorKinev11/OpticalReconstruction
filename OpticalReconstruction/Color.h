#pragma once

#include <cstdlib>
#include <vector>

#include "Math.h"
#include "Curve.h"

class Color
{
private:
    std::vector<float> colors;

public:
    static std::vector<int> waveLengths;

    static void setWaveLengths(int start, int end, int step);
    static void setWaveLengths(const std::vector<int>& waves);

    static Curve curve;
    static void  readCurve(const std::string& fileName);

    Color();
    explicit Color(float value);
    Color(const std::vector<float>& colors);
    void               setColors(const std::vector<float>& x);
    std::vector<float> getColors() const;
    Color              operator+(const Color& color) const;
    //    Color operator+(float value) const;
    Color operator-(const Color& color) const;
    //    Color operator-(float value) const;
    Color operator*(const Color& color) const;
    Color operator*(float value) const;
    Color operator/(const Color& color) const;
    Color operator/(float value) const;
    float dot(const Color& color) const;

    float sum() const;

    float&       operator[](const size_t i);
    const float& operator[](const size_t i) const;

    float getPhotometric() const;
    Vec3f getXYZ();
    Vec3f getLab();
};
