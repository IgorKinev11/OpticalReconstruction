#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "Render.h"

class DiffRender
{
public:
    DiffRender(Render* scene_);

    float squaredDifference(const float& a, const float& b, bool isSignNeeded = false);
    float calculateMSE(const std::vector<std::vector<Color>>& imageA, const std::vector<std::vector<Color>>& imageB);

    void CalculateAttenuationCoefs(
        std::map<int, std::map<coefs, std::map<int, std::vector<Color>>>>& objectsLuminance,
        std::map<int, std::map<int, float>>&                               attenuationCoefs);

    void PrintDiffs(std::map<int, std::map<coefs, std::map<int, std::vector<Color>>>>& objectsLuminance);

    void CalcLosses(
        float                                                              loss,
        const Color&                                                       originalLumForPixel,
        const float                                                        epsilon,
        std::map<int, std::map<coefs, std::map<int, std::vector<Color>>>>& objectsLuminance,
        std::map<int, std::map<coefs, float>>&                             objectCoefLosses);

    void UpdateOpticalParameters(float learning_rate, std::map<int, std::map<coefs, float>>& objectCoefLosses, int iter);

    void calculateGradient(
        float                                  loss,
        const std::vector<std::vector<Color>>& originalImage,
        const std::map<int, Ray>&              sparseRays,
        float                                  learningRate,
        float                                  epsilon,
        float                                  stepSize,
        int                                    iter
    );

    void gradientDescent(
        const std::vector<std::vector<Color>>&     originalImage, 
        float                                      stepSize,
        float                                      learningRate,
        float                                      epsilon,
        int                                        numOfEpochs
    );

    void DiffRender::ReadOriginalImage(
        const std::string& filePath,
        int& height,
        int& width,
        std::vector<std::vector<Color>>& originalImage
    );

private:
    Render* render_ = nullptr;
};