#include "Render.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <functional>
#include <map>
#include <utility>

const float Render::EPS = 1e-2;


Render::Render(Scene* scene_)
    : scene(scene_)
{}

Scene* Render::GetScene()
{
    return scene;
}

void Render::changeSceneMaterials(const std::string& materialsFile)
{
    scene->clearMaterials();
    scene->readMaterials(materialsFile);
}

void Render::initFluxes()
{
    fluxes.resize(scene->lights_.size());
    fluxes[0] = 0;

    for (int i = 1; i < scene->lights_.size(); ++i)
        fluxes[i] = fluxes[i - 1] + scene->lights_[i - 1]->getFlux();

    totalFlux = fluxes.back() + scene->lights_.back()->getFlux();
}

std::vector<std::vector<Ray>> Render::getRays()
{
    return scene->camera_->render();
}

std::map<int, Ray> Render::getRaysWithIntersection(
    int height,
    int width,
    const std::vector<std::vector<Ray>>& rays,
    int raysLimit,
    const std::vector<std::vector<Color>>& originalImage
)
{
    std::map<int, Ray> result;

    //int k = (462 * width) + 582;

    //result[k] = rays[582][462];

    int              i, j, k = 0;
    std::vector<std::pair<int, Ray>> oneDimRayArray(height * width);
    for (i = 0; i < height; ++i)
    {
        for (j = 0; j < width; ++j)
        {
            k                 = (i * width) + j;
            oneDimRayArray[k] = {k, rays[i][j]};
            k++;
        }
    }

    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::shuffle(oneDimRayArray.begin(), oneDimRayArray.end(), rng);

    int index    = 0;
    int rayCount = 0;

    while (index < oneDimRayArray.size() && rayCount <= raysLimit)
    {
        float t = std::numeric_limits<float>().max();
        Vec3f N;

        Geometry* hittedGeometry = getIntersection(oneDimRayArray[index].second, t, N);

        if (hittedGeometry != nullptr && !hittedGeometry->sourceLight_)
        {
            int col = oneDimRayArray[index].first % 900;
            int row = int(oneDimRayArray[index].first / 900.0f);

            if (originalImage[col][row].getPhotometric() > 0.02f)
            {
                result[oneDimRayArray[index].first] = oneDimRayArray[index].second;
                rayCount++;
            }
        }

        index++;
    }

    return result;
}

void Render::renderImage(
    int height, int width, const std::vector<std::vector<Ray>>& rays, std::vector<std::vector<Color>>& outLuminance)
{
    for (int phase = 0; phase < PHASES; ++phase)
    {
        std::cout << "Phase #" << phase << std::endl;

#pragma omp parallel for shared(height, width, outLuminance, rays) default(none)
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                Ray ray = rays[i][j];

                int   depth;
                Color currentPhaseLuminance;
                for (depth = 1; depth <= MAX_RENDER_DEPTH; ++depth)
                {
                    float t = std::numeric_limits<float>().max();
                    Vec3f N;
                    // передаем предыдущую hittedGeometry, чтобы исключить повторные
                    // пересечения с ней
                    Geometry* hittedGeometry = getIntersection(ray, t, N);

                    if (hittedGeometry == nullptr) // Нет пересечений
                    {
                        break;
                    }

                    if (hittedGeometry->sourceLight_)
                    { // Попали в источник света
                        // Попали после отражения или преломления
                        if (ray.trash.lastEvent == TransformRayEvent::e_START ||
                            ray.trash.lastEvent == TransformRayEvent::e_KS ||
                            ray.trash.lastEvent == TransformRayEvent::e_KTS)
                        {
                            currentPhaseLuminance =
                                currentPhaseLuminance +
                                ((RectangleLight*)hittedGeometry->sourceLight_)->calculateLuminance(ray.direction);
                        }
                    }

                    Vec3f     intersectionPoint = ray.origin + ray.direction * t;
                    Material* material          = scene->materials_[hittedGeometry->materialId_];

                    // Считаем освещенность
                    Light* light      = chooseLight();
                    Vec3f  lightPoint = light->getRandomPointOfSurf();
                    // Проверяем есть ли препятствия
                    Ray lightRay(intersectionPoint, (lightPoint - intersectionPoint).normalize());
                    t = (lightPoint - intersectionPoint).length() - 2 * EPS;
                    Ray       lightRayFake(lightRay.origin + lightRay.direction * EPS, lightRay.direction);
                    Vec3f     dummy;
                    Geometry* shadowGeometry = getIntersection(lightRayFake, t, dummy);

                    Color illuminance = shadowGeometry == nullptr
                                            ? light->calculateIlluminance(intersectionPoint, N, lightPoint)
                                            : Color();
                    illuminance       = illuminance * totalFlux / light->getFlux();

                    // Считаем яркость
                    currentPhaseLuminance =
                        currentPhaseLuminance +
                        material->CalculateLuminance(illuminance, ray.direction, lightRay.direction, N);

                    SurfaceOpticProperty* surfaceOpticProperty = material->chooseEvent(ray);

                    if (surfaceOpticProperty == nullptr)
                    { // луч поглотился
                        break;
                    }

                    ray        = surfaceOpticProperty->TransformRay(ray, N, intersectionPoint);
                    ray.origin = ray.origin + ray.direction * EPS;

                    if (ray.trash.lastEvent == TransformRayEvent::e_KILL)
                    {
                        break;
                    }
                }

                outLuminance[i][j] = outLuminance[i][j] + currentPhaseLuminance;
            }
        }
    }

#pragma omp parallel for shared(height, width, outLuminance) default(none)
    // Normilize Luminance
    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            outLuminance[i][j] = outLuminance[i][j] / PHASES;
        }
    }
}

void Render::saveResultInFile(
    const std::string& outputFileName, int height, int width, const std::vector<std::vector<Color>>& outLuminance)
{
    std::ofstream out(outputFileName);

    for (int waveLengthIter = 0; waveLengthIter < Color::waveLengths.size(); ++waveLengthIter)
    {
        out << "wave_length " << Color::waveLengths[waveLengthIter] << std::endl;
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
                out << outLuminance[i][j][waveLengthIter] << " ";

            out << std::endl;
        }
        out << std::endl;
    }

    out.close();
}

void Render::saveSparseResult(
    const std::string& outputFileName, const std::vector<Color>& outLuminance)
{
    std::ofstream out(outputFileName);

    for (int waveLengthIter = 0; waveLengthIter < Color::waveLengths.size(); ++waveLengthIter)
    {
        out << "wave_length " << Color::waveLengths[waveLengthIter] << std::endl;

        for (auto lum : outLuminance)
        {
            out << lum[waveLengthIter] << " ";
        }

        out << std::endl;
    }

    out.close();
}

int Render::GetHeight()
{
    return scene->camera_->getHeight();
}

int Render::GetWidth()
{
    return scene->camera_->getWidth();
}

 void Render::CalculateOneRayDiffs(
     const Ray&                                                          inputRay,
     const float                                                         stepSize,
     std::map<int, std::map<coefs, std::map<int, std::vector<Color>>>>&  objectsLuminance
)
{
     std::map<coefs, std::map<int, std::vector<Color>>> luminanceTable;

     int   depth;
     Color currentPhaseLuminance;

    for (int phase = 0; phase < PHASES; ++phase)
    {
        Ray ray = inputRay;

        Color                  color;
        std::map<coefs, float> coefs_map; // kd, ks, ktd, kts, brdf

        coefs_map.insert({coefs::kd, 1});
        // coefs_map.insert({coefs::ks, 1});
        // coefs_map.insert({coefs::ktd, 1});
        // coefs_map.insert({coefs::kts, 1});

        Material* predMaterial = new Material(color, coefs_map);

        for (depth = 1; depth <= MAX_RENDER_DEPTH; ++depth)
        {
            float t = std::numeric_limits<float>().max();
            Vec3f N;

            // передаем предыдущую hittedGeometry, чтобы исключить
            // повторные пересечения с ней Geometry* hittedGeometry = getIntersection(ray, t, N
            int       geometryIndex;
            Geometry* hittedGeometry = getIntersection(ray, t, N, geometryIndex);

            if (hittedGeometry == nullptr)
            { // Нет пересечений
                break;
            }

            if (hittedGeometry->sourceLight_)
            { // Попали в источник света
                // Попали после отражения или преломления
                if (ray.trash.lastEvent == TransformRayEvent::e_START ||
                    ray.trash.lastEvent == TransformRayEvent::e_KS || ray.trash.lastEvent == TransformRayEvent::e_KTS)
                {
                    currentPhaseLuminance =
                        currentPhaseLuminance +
                        ((RectangleLight*)hittedGeometry->sourceLight_)->calculateLuminance(ray.direction);
                }
            }

            Vec3f     intersectionPoint = ray.origin + ray.direction * t;
            Material* material          = scene->materials_[hittedGeometry->materialId_];

            // Считаем освещенность
            Light* light      = chooseLight();
            Vec3f  lightPoint = light->getRandomPointOfSurf();
            // Проверяем есть ли препятствия
            Ray lightRay(intersectionPoint, (lightPoint - intersectionPoint).normalize());
            t = (lightPoint - intersectionPoint).length() - 2 * EPS;
            Ray       lightRayFake(lightRay.origin + lightRay.direction * EPS, lightRay.direction);
            Vec3f     dummy;
            Geometry* shadowGeometry = getIntersection(lightRayFake, t, dummy);

            Color illuminance =
                shadowGeometry == nullptr ? light->calculateIlluminance(intersectionPoint, N, lightPoint) : Color();
            illuminance = illuminance * totalFlux / light->getFlux();

            if (!hittedGeometry->sourceLight_)
            {
                for (auto coef : material->optical_coef_map)
                {
                    Material dummyMaterial = *material;

                    dummyMaterial.optical_coef_map[coef.first]->setCoeff(coef.second->getCoeff() + stepSize);

                    dummyMaterial.NormalizeCoefs(
                        dummyMaterial.optical_coef_map[coef.first]); // TODO: нужно сделать красивее

                    Color curLum = dummyMaterial.CalculateLuminance(illuminance, ray.direction, lightRay.direction,
                    N);

                    if (!objectsLuminance[geometryIndex][coef.first][depth].size())
                    {
                        objectsLuminance[geometryIndex][coef.first][depth].push_back(curLum);
                    }
                    else
                    {
                        objectsLuminance[geometryIndex][coef.first][depth][0] =
                            objectsLuminance[geometryIndex][coef.first][depth][0] + curLum;
                    }
                }
            }

            SurfaceOpticProperty* surfaceOpticProperty = material->chooseEvent(ray);

            if (surfaceOpticProperty == nullptr)
            { // луч поглотился
                break;
            }

            ray        = surfaceOpticProperty->TransformRay(ray, N, intersectionPoint);
            ray.origin = ray.origin + ray.direction * EPS;

            if (ray.trash.lastEvent == TransformRayEvent::e_KILL)
            {
                break;
            }
        }
    }

    // Normilize Luminace
    // Calc Luminances
    for (auto& obj : objectsLuminance)
    {
        for (auto& coef : obj.second)
        {
            for (auto& depth : coef.second)
            {
                for (auto &lum : depth.second)
                {
                    lum = lum / PHASES;
                }
            }
        }
    }
}

void Render::CalculateDiffs(
    std::map<int, Ray>&                    rays,
    std::vector<Color>&                    outLuminance,
    std::map<int, std::map<coefs, Color>>& result,
    const float                            stepSize)
{

    for (auto ray : rays)
    {
        int   depth;
        Color currentPhaseLuminance;

        std::map<coefs, float> weights;
        for (int i = coefs::kd; i <= coefs::kd; ++i)
        {
            coefs coef = static_cast<coefs>(i);
            weights[coef] = 1.0f;
        }

        Color color;
        std::map<coefs, float> coefs_map; // kd, ks, ktd, kts

        coefs_map.insert({coefs::kd, 1});
        //coefs_map.insert({coefs::ks, 1});
        //coefs_map.insert({coefs::ktd, 1});
        //coefs_map.insert({coefs::kts, 1});

        Material* predMaterial = new Material(color, coefs_map);

        for (depth = 1; depth <= MAX_RENDER_DEPTH; ++depth)
        {
            float t = std::numeric_limits<float>().max();
            Vec3f N;
            // передаем предыдущую hittedGeometry, чтобы исключить
            // повторные пересечения с ней Geometry* hittedGeometry = getIntersection(ray, t, N)
            Geometry* hittedGeometry = getIntersection(ray.second, t, N);

            if (hittedGeometry == nullptr)
            { // Нет пересечений
                break;
            }

            if (hittedGeometry->sourceLight_)
            { // Попали в источник света
                // Попали после отражения или преломления
                if (ray.second.trash.lastEvent == TransformRayEvent::e_START ||
                    ray.second.trash.lastEvent == TransformRayEvent::e_KS ||
                    ray.second.trash.lastEvent == TransformRayEvent::e_KTS)
                {
                    currentPhaseLuminance =
                        currentPhaseLuminance +
                        ((RectangleLight*)hittedGeometry->sourceLight_)->calculateLuminance(ray.second.direction);
                }
            }

            Vec3f     intersectionPoint = ray.second.origin + ray.second.direction * t;
            Material* material          = scene->materials_[hittedGeometry->materialId_];

            // Считаем освещенность
            Light* light      = chooseLight();
            Vec3f  lightPoint = light->getRandomPointOfSurf();
            // Проверяем есть ли препятствия
            Ray lightRay(intersectionPoint, (lightPoint - intersectionPoint).normalize());
            t = (lightPoint - intersectionPoint).length() - 2 * EPS;
            Ray       lightRayFake(lightRay.origin + lightRay.direction * EPS, lightRay.direction);
            Vec3f     dummy;
            Geometry* shadowGeometry = getIntersection(lightRayFake, t, dummy);

            Color illuminance =
                shadowGeometry == nullptr ? light->calculateIlluminance(intersectionPoint, N, lightPoint) : Color();
            illuminance = illuminance * totalFlux / light->getFlux();

            // Изменение на дельта для каждого коэффициента
            for (auto coef : material->optical_coef_map)
            {
                Material dummyMaterial = *material;
                dummyMaterial.optical_coef_map[coef.first]
                    ->setCoeff(coef.second->getCoeff() + stepSize);

                dummyMaterial.NormalizeCoefs(dummyMaterial.optical_coef_map[coef.first]); // TODO: нужно сделать красивее

                result[ray.first][coef.first] =
                    result[ray.first][coef.first] +
                    material->CalculateLuminance(illuminance, ray.second.direction, lightRay.direction, N);
            }

            // Считаем яркость
            currentPhaseLuminance =
                 currentPhaseLuminance +
                material->CalculateLuminance(illuminance, ray.second.direction, lightRay.direction, N);

            // Вычисление весов
            for (auto& coef : material->optical_coef_map)
            {
                float pred_coef = predMaterial->optical_coef_map[coef.first]->getCoeff();

                // обновляем вес для текущего коэффициента
                weights[coef.first] = (weights[coef.first] / pred_coef) * coef.second->getCoeff();
            }

            if (depth == 1)
            {
                delete predMaterial;
            }

            predMaterial = material;

            SurfaceOpticProperty* surfaceOpticProperty = material->chooseEvent(ray.second);

            if (surfaceOpticProperty == nullptr)
            { // луч поглотился
                break;
            }

            ray.second        = surfaceOpticProperty->TransformRay(ray.second, N, intersectionPoint);
            ray.second.origin = ray.second.origin + ray.second.direction * EPS;

            if (ray.second.trash.lastEvent == TransformRayEvent::e_KILL)
            {
                break;
            }
        }

        outLuminance[ray.first] = outLuminance[ray.first] + currentPhaseLuminance;
    }
}

void Render::SparseRender(std::vector<Ray>& rays, std::vector<Color>& renderedImage)
{
    for (int i = 0; i < rays.size(); ++i)
    {
        int   depth;
        Color currentPhaseLuminance;

        for (depth = 1; depth <= MAX_RENDER_DEPTH; ++depth)
        {
            float t = std::numeric_limits<float>().max();
            Vec3f N;
            // передаем предыдущую hittedGeometry, чтобы исключить
            // повторные пересечения с ней Geometry* hittedGeometry = getIntersection(ray, t, N
            Geometry* hittedGeometry = getIntersection(rays[i], t, N);

            if (hittedGeometry == nullptr)
            { // Нет пересечений
                break;
            }

            if (hittedGeometry->sourceLight_)
            { // Попали в источник света
                // Попали после отражения или преломления
                if (rays[i].trash.lastEvent == TransformRayEvent::e_START ||
                    rays[i].trash.lastEvent == TransformRayEvent::e_KS ||
                    rays[i].trash.lastEvent == TransformRayEvent::e_KTS)
                {
                    currentPhaseLuminance =
                        currentPhaseLuminance +
                        ((RectangleLight*)hittedGeometry->sourceLight_)->calculateLuminance(rays[i].direction);
                }
            }

            Vec3f     intersectionPoint = rays[i].origin + rays[i].direction * t;
            Material* material          = scene->materials_[hittedGeometry->materialId_];

            // Считаем освещенность
            Light* light      = chooseLight();
            Vec3f  lightPoint = light->getRandomPointOfSurf();
            // Проверяем есть ли препятствия
            Ray lightRay(intersectionPoint, (lightPoint - intersectionPoint).normalize());
            t = (lightPoint - intersectionPoint).length() - 2 * EPS;
            Ray       lightRayFake(lightRay.origin + lightRay.direction * EPS, lightRay.direction);
            Vec3f     dummy;
            Geometry* shadowGeometry = getIntersection(lightRayFake, t, dummy);

            Color illuminance =
                shadowGeometry == nullptr ? light->calculateIlluminance(intersectionPoint, N, lightPoint) : Color();
            illuminance = illuminance * totalFlux / light->getFlux();

            // Считаем яркость
            currentPhaseLuminance = currentPhaseLuminance +
                                    material->CalculateLuminance(illuminance, rays[i].direction, lightRay.direction, N);

            SurfaceOpticProperty* surfaceOpticProperty = material->chooseEvent(rays[i]);

            if (surfaceOpticProperty == nullptr)
            { // луч поглотился
                break;
            }

            rays[i]        = surfaceOpticProperty->TransformRay(rays[i], N, intersectionPoint);
            rays[i].origin = rays[i].origin + rays[i].direction * EPS;

            if (rays[i].trash.lastEvent == TransformRayEvent::e_KILL)
            {
                break;
            }
        }

        renderedImage[i] = renderedImage[i] + currentPhaseLuminance;
    }
}

std::vector<Color> Render::calculateDiff(std::vector<Color>& sampleLuminance, std::vector<Color>& calculateLuminance)
{
    std::vector<Color> diffLuminance(sampleLuminance.size());

    for (int waveLengthIter = 0; waveLengthIter < Color::waveLengths.size(); ++waveLengthIter)
    {
        for (int i = 0; i < diffLuminance.size(); ++i)
            diffLuminance[i][waveLengthIter] =
                sampleLuminance[i][waveLengthIter] - calculateLuminance[i][waveLengthIter];
    }

    return diffLuminance;
}

void Render::renderZBuffer(std::vector<std::vector<Color>>& outLuminance)
{
    int width  = scene->camera_->getWidth();
    int height = scene->camera_->getHeight();

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    initFluxes();                                   // Инициализируем поток
    std::vector<std::vector<Ray>> rays = getRays(); //Получаем лучи

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Rendering time = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "s"
              << std::endl;

    renderImage(height, width, rays, outLuminance);
}

Geometry* Render::getIntersection(const Ray& ray, float& t, Vec3f& N) const
{
    Geometry* hitted = nullptr;
    int       count      = 0; 

    for (const auto& geometry : scene->geometry_)
    {
        if (geometry->hitTest(ray, t, N))
            hitted = geometry;

        count++;
    }

    return hitted;
}

Geometry* Render::getIntersection(const Ray& ray, float& t, Vec3f& N, int& geometryIndex) const
{
    Geometry* hitted = nullptr;
    int       count  = 0;

    for (int i = 0; i < scene->geometry_.size(); ++i)
    {
        if (scene->geometry_[i]->hitTest(ray, t, N))
        {
            hitted        = scene->geometry_[i];
            geometryIndex = i;
        }
    }

    return hitted;
}

Light* Render::chooseLight() const
{
    static std::mt19937                          gen;
    static std::uniform_real_distribution<float> dist(0, 1);

    float xi = dist(gen) * totalFlux;
    int   l = 0, r = scene->lights_.size();
    while (r - l > 1)
    {
        int m = (l + r) / 2;

        if (fluxes[m] < xi)
            l = m;
        else
            r = m;
    }

    return scene->lights_[l];
}
