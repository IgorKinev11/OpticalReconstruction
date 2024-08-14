#pragma once
#include <string>
#include <vector>

enum CurveParseState
{
    INIT,
    X_curve,
    Y_curve,
    Z_curve
};

class Curve
{
public:
    Curve() = default;

    ~Curve() = default;

public:
    int                waveLengthMinBound;
    int                waveLengthMaxBound;
    int                step;
    std::vector<float> curve_x;
    std::vector<float> curve_y;
    std::vector<float> curve_z;

    void readCurve(const std::string& fileName);
};

struct LinearSpline
{
    float a, b;
    int   x;
};

struct CubicSpline
{
    float a, b, c, d, x;
};

std::vector<CubicSpline> computeCubicSplines(const std::vector<int>& waveLengths, const std::vector<float>& colors);
float                    evaluateCubicSpline(const std::vector<CubicSpline>& splines, int x);

std::vector<float> interpolateColors(
    const std::vector<int>& waveLengths, const std::vector<float>& colors, const std::vector<int>& curveWaveLengths);

float integrateCubicSpline(const std::vector<int>& waves, const std::vector<float>& lum, int step);