#include "Gradient.h"

#include <utility>
#include <fstream>
#include <sstream>

DiffRender::DiffRender(Render* render_)
    : render_(render_)
{}

float DiffRender::squaredDifference(const float& a, const float& b, bool isSignNeeded)
{
    float diff = 0.0f;
    float sign = 1.0f;

    float c = a - b;

    if (isSignNeeded)
    {
        std::cout << "a : " << a << "b :" << b << " c: " << c << std::endl;
        return c;
    }
    else
    {
        return c * c;
    }
}

// Функция для вычисления MSE между двумя изображениями
float DiffRender::calculateMSE(const std::vector<std::vector<Color>>& imageA, const std::vector<std::vector<Color>>& imageB)
{
    float mse        = 0.0;

    for (int i = 0; i < imageA.size(); ++i)
    {
        for (int j = 0; j < imageA[i].size(); ++j)
        {
            mse += squaredDifference(imageA[i][j].getPhotometric(), imageB[i][j].getPhotometric());
        }
    }

    return mse / (imageA.size() * imageA[0].size());
}

void DiffRender::CalculateAttenuationCoefs(
    std::map<int, std::map<coefs, std::map<int, std::vector<Color>>>>& objectsLuminance,
    std::map<int, std::map<int, float>>&                               attenuationCoefs)
{
    std::map<int, float> percentages;
    Color                totalSum;

    for (auto& obj : objectsLuminance)
    {

        for (auto& coef : obj.second)
        {
            for (auto& depth : coef.second)
            {
                for (auto& color : depth.second)
                {
                    totalSum = totalSum + color;
                }
            }
        }
    }

    float totalSumPhotometric = totalSum.getPhotometric();

    std::cout << "totalSUM: " << totalSumPhotometric << std::endl;

    for (auto& obj : objectsLuminance)
    {

        if (!totalSumPhotometric)
        {
            for (auto& coef : obj.second)
            {
                for (auto& depth : coef.second)
                {
                    percentages[depth.first] = 0.0;
                }
            }
        }
        else
        {
            for (auto& coef : obj.second)
            {
                for (auto& depth : coef.second)
                {
                    Color depthSum;
                    for (auto& color : depth.second)
                    {
                        depthSum = depthSum + color;
                    }

                    float depthSumPhotometric = depthSum.getPhotometric();

                    float percentage         = (depthSumPhotometric / totalSumPhotometric);
                    percentages[depth.first] = std::move(percentage);
                }
            }
        }

        std::cout << "Object " << obj.first << " percentages: ";
        for (auto& percent : percentages)
        {
            std::cout << percent.first << " : " << percent.second * 100.0 << "%, ";
        }
        std::cout << std::endl;

        attenuationCoefs[obj.first] = std::move(percentages);
    }
}

void DiffRender::PrintDiffs(std::map<int, std::map<coefs, std::map<int, std::vector<Color>>>>& objectsLuminance)
{

    std::cout << "\n";
    std::cout << "---------------------------------------------------" << std::endl;

    // Calc Luminances
    for (auto& obj : objectsLuminance)
    {
        std::cout << "obj : " << obj.first << std::endl;

        int countCoef = 0;
        for (auto& coef : obj.second)
        {
            std::cout << "coef : " << countCoef << std::endl;

            for (auto& depth : coef.second)
            {
                std::cout << "depth : " << depth.first << std::endl;

                for (auto lum : depth.second)
                {
                    std::cout << "lum: " << lum.getPhotometric() << std::endl;
                }
            }

            countCoef++;
        }
    }

    std::cout << "---------------------------------------------------" << std::endl;
}


void DiffRender::CalcLosses
(
    float        loss,
    const Color& originalLumForPixel,
    const float  epsilon,
    std::map<int, std::map<coefs, std::map<int, std::vector<Color>>>>&  objectsLuminance,
    std::map<int, std::map<coefs, float>>&                              objectCoefLosses)
{
    PrintDiffs(objectsLuminance);

    Color clr = originalLumForPixel;
    float orLum = clr.getPhotometric();

    std::map<int, std::map<coefs, float>> objectsSumLuminance;
    for (auto &obj : objectsLuminance)
    {
        for (int i = coefs::kd; i <= coefs::kd; ++i)                                                                               
        {
            coefs coef                            = static_cast<coefs>(i);
            objectsSumLuminance[obj.first][coef]  = 0.0;
        }
    }

    std::map<int, std::map<coefs, float>> originalLumByCoefs;
    for (auto &obj : objectsLuminance)
    {
        for (int i = coefs::kd; i <= coefs::kd; ++i)
        {
            coefs coef                                 = static_cast<coefs>(i);
            originalLumByCoefs[obj.first][coef]= 0;
        }
    }

    for (auto &obj : objectsLuminance)
    {
        for (int i = coefs::kd; i <= coefs::kd; ++i)
        {
            coefs coef                        = static_cast<coefs>(i);
            objectCoefLosses[obj.first][coef] = 0.0f;
        }
    }

    // Calc attenuation coefs
    std::map<int, std::map<int, float>> attenuationCoefs;
    CalculateAttenuationCoefs(objectsLuminance, attenuationCoefs);

    // Calc Luminances
    for (auto &obj : objectsLuminance)
    {
        for (auto &coef : obj.second)
        {
            for (auto& depth : coef.second)
            {
                for (auto lum : depth.second)
                {
                    // Diff Lum
                    objectsSumLuminance[obj.first][coef.first] =
                        objectsSumLuminance[obj.first][coef.first] + (lum.getPhotometric());

                    float originCoef = attenuationCoefs[obj.first][depth.first];
                    if (originCoef <= 0)
                    {
                        originCoef = 0.1e-60;
                    }

                    // Origin Lum
                    originalLumByCoefs[obj.first][coef.first] =
                        originalLumByCoefs[obj.first][coef.first] + (originalLumForPixel.getPhotometric() * originCoef);
                }
            }
        }
    }

    // Calc losses
    for (auto &obj : objectsSumLuminance)
    {
        for (auto &coef : obj.second)
        {
            bool isSignNeeded = true;

            objectCoefLosses[obj.first][coef.first] = 0.0f +
                (squaredDifference(originalLumByCoefs[obj.first][coef.first],
                    objectsSumLuminance[obj.first][coef.first], isSignNeeded));
        }
    }
}

void DiffRender::UpdateOpticalParameters(
    float learning_rate, std::map<int, std::map<coefs, float>>& objectCoefLosses,
    int iter)
{
    Scene* scene_ = render_->GetScene();

    for (auto &obj : objectCoefLosses)
    {
        for (auto& diffCoef : obj.second)
        {
            if (!scene_->geometry_[obj.first]->sourceLight_)
            {
                for (auto& origCoef : scene_->materials_[scene_->geometry_[obj.first]->materialId_]->optical_coef_map)
                {
                    std::cout << "learning_rate: " << learning_rate << " diffCoef: " << diffCoef.second << std::endl;

                    origCoef.second->setCoeff(origCoef.second->getCoeff() + (learning_rate * diffCoef.second));
                    scene_->materials_[scene_->geometry_[obj.first]->materialId_]->NormalizeCoefs(origCoef.second);

                    std::cout << "Step: " << iter << std::endl;

                    int materialCount = 0;
                    for (auto& material : scene_->materials_)
                    {
                        for (auto& coef : material->optical_coef_map)
                        {
                            std::cout << "material: " << materialCount << " coef: " << coef.second->getCoeff()
                                      << std::endl;
                        }
                        materialCount++;
                    }
                }
            }
        }
    }
}

void DiffRender::calculateGradient(
    float                                        loss,
    const std::vector < std::vector < Color >>&  originalImage,
    const std::map<int, Ray>&                          sparseRays,
    float                                        learningRate,
    float                                        epsilon,
    float                                        stepSize,
    int                                          iter
)
{

    int count = 0;

    for (auto ray : sparseRays)
    {
        int col = (ray.first % originalImage[0].size());
        int row = int(ray.first / float(originalImage[0].size()));
   
        render_->initFluxes();

        std::map<int, std::map<coefs,std::map<int, std::vector<Color>>>> objectsLuminance;
        render_->CalculateOneRayDiffs(ray.second, stepSize, objectsLuminance);

        // Расчет яркостей и loss
        std::map<int, std::map<coefs, float>> objectCoefLosses;
        CalcLosses(loss, originalImage[col][row], epsilon, objectsLuminance, objectCoefLosses);

        // Обновление градиентов
        UpdateOpticalParameters(learningRate, objectCoefLosses, iter);

        count++;
    }
} 

void saveCoefficientsToFile(const std::vector<Material*>& materials, const std::string& filename)
{
    std::ofstream outFile(filename, std::ios_base::app);
    if (outFile.is_open())
    {
        for (const auto& material : materials)
        {
            for (const auto& coef : material->optical_coef_map)
            {
                outFile << coef.second->getCoeff() << std::endl;
            }
        }
        outFile.close();
    }
    else
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

// Функция градиентного спуска
void DiffRender::gradientDescent(
    const std::vector<std::vector<Color>>&  originalImage,
    float                                   stepSize,
    float                                   learningRate,
    float                                   epsilon,
    int                                     numOfEpochs)
{
    int raysLimit = 1000;
    int height    = render_->GetHeight();
    int width     = render_->GetWidth();

    std::vector<std::vector<Ray>> rays       = render_->getRays();
    const std::map<int, Ray>            sparseRays = render_->getRaysWithIntersection(height, width, rays, raysLimit, originalImage);

    for (int i = 0; i < numOfEpochs; i++)
    {
        Scene* scene_ = render_->GetScene();
        saveCoefficientsToFile(scene_->materials_, std::string(SOURCE_DIR) + "/result/graph.txt");
        calculateGradient(0.0, originalImage, sparseRays, learningRate, epsilon, stepSize, i);
    }
    std::vector<std::vector<Color>> renderedImage(height, std::vector<Color>(width));

    render_->renderZBuffer(renderedImage);

    render_->saveResultInFile(
        std::string(SOURCE_DIR) + "/result/" + "render_epoch_" + std::to_string(50), 900, 900, renderedImage);

    float loss = calculateMSE(originalImage, renderedImage);

    std::cout << "Epochs: " + std::to_string(50) + " , Loss: " << loss << '\n';

}

void DiffRender::ReadOriginalImage
(
    const std::string&                             filePath,
    int&                                           height,
    int&                                           width,
    std::vector<std::vector<Color>>&               originalImage
)
{
    std::ifstream in(filePath);
    if (!in.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    std::string line;
    int         waveLengthIter = 0;

    while (std::getline(in, line))
    {
        if (line.find("wave_length") != std::string::npos)
        {
            std::istringstream iss(line);
            std::string        dummy;
            int                waveLength;
            iss >> dummy >> waveLength;

            if (waveLengthIter >= Color::waveLengths.size())
            {
                std::cerr << "Unexpected wave length found in file." << std::endl;
                return;
            }

            for (int i = 0; i < width; ++i)
            {
                std::getline(in, line);
                std::istringstream iss(line);
                for (int j = 0; j < height; ++j)
                {
                    float luminance;
                    iss >> luminance;
                    originalImage[i][j][waveLengthIter] = luminance;
                }
            }

            ++waveLengthIter;
            std::getline(in, line);
        }
    }

    in.close();
}