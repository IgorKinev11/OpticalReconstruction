#include "Curve.h"

#include <fstream>
#include <sstream>

void Curve::readCurve(const std::string& fileName)
{
    waveLengthMinBound = 380;
    waveLengthMaxBound = 780;
    step               = 5;

    std::ifstream   in(fileName);
    bool            chek = in.is_open();
    std::string     line;
    CurveParseState state = CurveParseState::INIT;

    float x, y, z;
    while (getline(in, line))
    {
        switch (state)
        {
        case CurveParseState::INIT: {
            if (line.find("X-curve") != std::string::npos)
            {
                state = CurveParseState::X_curve;
            }
            break;
        }
        case CurveParseState::X_curve: {
            if (line.find("Y-curve") != std::string::npos)
            {
                state = CurveParseState::Y_curve;
                break;
            }
            else
            {
                std::istringstream iss(line);
                iss >> x;
                curve_x.push_back(x);
            }
            break;
        }
        case CurveParseState::Y_curve: {
            if (line.find("Z-curve") != std::string::npos)
            {
                state = CurveParseState::Z_curve;

                break;
            }
            else
            {
                std::istringstream iss(line);
                iss >> y;
                curve_y.push_back(y);
            }
            break;
        }
        case CurveParseState::Z_curve: {
            std::istringstream iss(line);
            iss >> z;
            curve_z.push_back(z);

            break;
        }
        }
    }
    in.close();
}

std::vector<CubicSpline> computeCubicSplines(const std::vector<int>& waveLengths, const std::vector<float>& colors)
{
    int n = waveLengths.size() - 1;

    std::vector<float> h(n);
    std::vector<float> alpha(n);
    for (int i = 0; i < n; ++i)
    {
        h[i]     = waveLengths[i + 1] - waveLengths[i];
        alpha[i] = (colors[i + 1] - colors[i]) / h[i];
    }

    std::vector<float> l(n + 1, 1.0);
    std::vector<float> mu(n, 0.0);
    std::vector<float> z(n + 1, 0.0);

    for (int i = 1; i < n; ++i)
    {
        l[i]  = 2 * (waveLengths[i + 1] - waveLengths[i - 1]) - h[i - 1] * mu[i - 1];
        mu[i] = h[i] / l[i];
        z[i]  = (alpha[i] - alpha[i - 1]) / l[i];
    }

    std::vector<CubicSpline> splines(n);
    std::vector<float>       c(n + 1, 0.0);
    std::vector<float>       b(n, 0.0);
    std::vector<float>       d(n, 0.0);

    for (int j = n - 1; j >= 0; --j)
    {
        c[j]       = z[j] - mu[j] * c[j + 1];
        b[j]       = (colors[j + 1] - colors[j]) / h[j] - h[j] * (c[j + 1] + 2 * c[j]) / 3;
        d[j]       = (c[j + 1] - c[j]) / (3 * h[j]);
        splines[j] = {colors[j], b[j], c[j], d[j], static_cast<float>(waveLengths[j])};
    }

    return splines;
}

float evaluateCubicSpline(const std::vector<CubicSpline>& splines, int x)
{
    CubicSpline s;
    for (const auto& spline : splines)
    {
        if (x >= spline.x)
        {
            s = spline;
        }
        else
        {
            break;
        }
    }
    float dx = x - s.x;
    return s.a + s.b * dx + s.c * dx * dx + s.d * dx * dx * dx;
}

std::vector<float> interpolateColors(
    const std::vector<int>& waveLengths, const std::vector<float>& colors, const std::vector<int>& curveWaveLengths)
{
    std::vector<CubicSpline> splines = computeCubicSplines(waveLengths, colors);
    std::vector<float>       interpolatedColors;

    for (int curveWaveLength : curveWaveLengths)
    {
        if (std::find(waveLengths.begin(), waveLengths.end(), curveWaveLength) != waveLengths.end())
        {
            auto it = std::find(waveLengths.begin(), waveLengths.end(), curveWaveLength);
            interpolatedColors.push_back(colors[it - waveLengths.begin()]);
        }
        else
        {
            if (curveWaveLength < waveLengths.front())
            {
                interpolatedColors.push_back(colors.front());
            }
            else if (curveWaveLength > waveLengths.back())
            {
                interpolatedColors.push_back(colors.back());
            }
            else
            {
                interpolatedColors.push_back(evaluateCubicSpline(splines, curveWaveLength));
            }
        }
    }

    return interpolatedColors;
}

float integrateCubicSpline(const std::vector<int>& waves, const std::vector<float>& lum, int step)
{
    std::vector<CubicSpline> splines = computeCubicSplines(waves, lum);
    float                    area    = 0.0;
    int                      n       = waves.size();

    for (int i = 1; i < n; ++i)
    {
        int   x0 = waves[i - 1];
        int   x1 = waves[i];
        float h  = (x1 - x0) / static_cast<float>(step);

        for (int j = 0; j < step; ++j)
        {
            float x_left  = x0 + j * h;
            float x_right = x0 + (j + 1) * h;
            float f_left  = evaluateCubicSpline(splines, x_left);
            float f_right = evaluateCubicSpline(splines, x_right);
            area += 0.5 * h * (f_left + f_right);
        }
    }

    return area;
}