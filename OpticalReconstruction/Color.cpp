#include "Color.h"

#include <cassert>
#include <vector>

std::vector<int> Color::waveLengths;
Curve            Color::curve;

void Color::setWaveLengths(int start, int end, int step)
{
    // Создаем массив длин волн с определенным шагом
    waveLengths.clear();
    for (int i = start; i <= end; i += step)
        waveLengths.push_back(i);
}

void Color::setWaveLengths(const std::vector<int>& waves)
{
    waveLengths = waves;
}

void Color::readCurve(const std::string& fileName)
{
    curve.readCurve(fileName);
}

Color::Color()
    : colors(waveLengths.size(), 0)
{}

Color::Color(float value)
    : colors(waveLengths.size(), value)
{}

Color::Color(const std::vector<float>& colors)
    : colors(colors)
{}

void Color::setColors(const std::vector<float>& colors)
{
    this->colors = colors;
}

std::vector<float> Color::getColors() const
{
    return colors;
}

Color Color::operator+(const Color& color) const
{
    assert(this->colors.size() == color.colors.size());

    Color result;

    for (int i = 0; i < colors.size(); i++)
        result.colors[i] = colors[i] + color.colors[i];

    return result;
}

// Color Color::operator+(float value) const {
//     Color result;
//
//     for (int i = 0; i < colors.size(); i++) {
//         result.colors[i] = colors[i] + value;
//     }
//
//     return result;
// }

Color Color::operator-(const Color& color) const
{
    assert(this->colors.size() == color.colors.size());

    Color result;

    for (int i = 0; i < colors.size(); i++)
        result.colors[i] = colors[i] - color.colors[i];

    return result;
}

// Color Color::operator-(float value) const {
//     Color result;
//
//     for (int i = 0; i < colors.size(); i++) {
//       result.colors[i] = colors[i] - value;
//     }
//
//     return result;
// }

Color Color::operator*(const Color& color) const
{
    assert(this->colors.size() == color.colors.size());

    Color result;

    for (int i = 0; i < colors.size(); i++)
        result.colors[i] = colors[i] * color.colors[i];

    return result;
}

Color Color::operator*(float value) const
{
    Color result;

    for (int i = 0; i < colors.size(); i++)
        result.colors[i] = colors[i] * value;

    return result;
}

Color Color::operator/(const Color& color) const
{
    assert(this->colors.size() == color.colors.size());

    Color result;

    for (int i = 0; i < colors.size(); i++)
        result.colors[i] = colors[i] / color.colors[i];

    return result;
}

Color Color::operator/(float value) const
{
    Color result;

    for (int i = 0; i < colors.size(); i++)
        result.colors[i] = colors[i] / value;

    return result;
}

float Color::dot(const Color& color) const
{
    assert(this->colors.size() == color.colors.size());

    float result = 0;

    for (int i = 0; i < colors.size(); i++)
        result += colors[i] * color.colors[i];

    return result;
}

float& Color::operator[](const size_t i)
{
    assert(i < colors.size());
    return colors[i];
}

const float& Color::operator[](const size_t i) const
{
    assert(i < colors.size());
    return colors[i];
}

float Color::sum() const
{
    float res = 0;
    for (float x : colors)
        res += x;

    return res;
}

 float Color::getPhotometric() const
{
    std::vector<int>   curveWaveLenghts;
    std::vector<int>   waveLenghtsTemp = waveLengths;
    std::vector<float> curve_Y;

    for (int i = curve.waveLengthMinBound; i <= curve.waveLengthMaxBound; i += curve.step)
        curveWaveLenghts.push_back(i);

    // Вычисляем сплайны
    std::vector<float> curve_Temp = interpolateColors(waveLenghtsTemp, colors, curveWaveLenghts);

    // Интерполяция значений

    for (auto i = 0; i < curveWaveLenghts.size(); i++)
        curve_Y.push_back(curve_Temp[i] * curve.curve_y[i]);

    int step = 1;
    return integrateCubicSpline(curveWaveLenghts, curve_Y, step);
}

Vec3f Color::getXYZ()
{
    std::vector<int>   curveWaveLenghts;
    std::vector<int>   waveLenghtsTemp = waveLengths;
    std::vector<float> curve_X;
    std::vector<float> curve_Y;
    std::vector<float> curve_Z;

    for (int i = curve.waveLengthMinBound; i <= curve.waveLengthMaxBound; i += curve.step)
        curveWaveLenghts.push_back(i);

    std::vector<float> curve_Temp = interpolateColors(waveLenghtsTemp, colors, curveWaveLenghts);

    for (auto i = 0; i < curveWaveLenghts.size(); i++)
        curve_X.push_back(curve_Temp[i] * curve.curve_x[i]);

    for (auto i = 0; i < curveWaveLenghts.size(); i++)
        curve_Y.push_back(curve_Temp[i] * curve.curve_y[i]);

    for (auto i = 0; i < curveWaveLenghts.size(); i++)
        curve_Z.push_back(curve_Temp[i] * curve.curve_z[i]);

    int step = 1;

    float X = integrateCubicSpline(curveWaveLenghts, curve_X, step);
    float Y = integrateCubicSpline(curveWaveLenghts, curve_Y, step);
    float Z = integrateCubicSpline(curveWaveLenghts, curve_Z, step);

    return Vec3f(X, Y, Z);
}

float fLab(float t)
{
    return (t > 0.008856) ? std::cbrt(t) : (7.787 * t + 16.0 / 116.0);
}

Vec3f Color::getLab()
{
    std::vector<int>   curveWaveLenghts;
    std::vector<int>   waveLenghtsTemp = waveLengths;
    std::vector<float> curve_X;
    std::vector<float> curve_Y;
    std::vector<float> curve_Z;

    for (int i = curve.waveLengthMinBound; i <= curve.waveLengthMaxBound; i += curve.step)
        curveWaveLenghts.push_back(i);

    std::vector<float> curve_Temp = interpolateColors(waveLenghtsTemp, colors, curveWaveLenghts);

    for (auto i = 0; i < curveWaveLenghts.size(); i++)
        curve_X.push_back(curve_Temp[i] * curve.curve_x[i]);

    for (auto i = 0; i < curveWaveLenghts.size(); i++)
        curve_Y.push_back(curve_Temp[i] * curve.curve_y[i]);

    for (auto i = 0; i < curveWaveLenghts.size(); i++)
        curve_Z.push_back(curve_Temp[i] * curve.curve_z[i]);

    int step = 1;

    float X = integrateCubicSpline(curveWaveLenghts, curve_X, step);
    float Y = integrateCubicSpline(curveWaveLenghts, curve_Y, step);
    float Z = integrateCubicSpline(curveWaveLenghts, curve_Z, step);

    const float Xn = 95.047;
    const float Yn = 100.000;
    const float Zn = 108.883;

    X /= Xn;
    Y /= Yn;
    Z /= Zn;

    float L = 116.0 * fLab(Y) - 16.0; 
    float a = 500.0 * (fLab(X) - fLab(Y));
    float b = 200.0 * (fLab(Y) - fLab(Z));

    return Vec3f(L, a, b);
}