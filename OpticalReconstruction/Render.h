#pragma once

#include "Math.h"
#include "Ray.h"
#include "Scene.h"
#include <algorithm>
#include <unordered_map>

class Render
{
public:
    Render(Scene* scene_);

    Scene* GetScene();

    //	void render(const std::string& outputFileName);
    void               renderZBuffer(std::vector<std::vector<Color>>& outLuminance);
    void               SparseRender(std::vector<Ray>& rays, std::vector<Color>& renderedImage);
    void changeSceneMaterials(const std::string& materialsFile);
    void saveSparseResult(const std::string& outputFileName, const std::vector<Color>& outLuminance);
    std::vector<Color> calculateDiff(std::vector<Color>& sampleLuminance, std::vector<Color>& calculateLuminance);
    void               CalculateDiffs(
                      std::map<int, Ray>&                                        rays,
                      std::vector<Color>&                                      outLuminance,
                      std::map<int, std::map<coefs, Color>>& result, const float stepSize = 0.25);

    void CalculateOneRayDiffs(
        const Ray&                                                               ray,
        const float                                                        stepSize,
        std::map<int, std::map<coefs, std::map<int, std::vector<Color>>>>& objectsLuminance);
    
    std::map<int, Ray> getRaysWithIntersection(
        int                                    height,
        int                                    width,
        const std::vector<std::vector<Ray>>&   rays,
        int                                    raysLimit,
        const std::vector<std::vector<Color>>& originalImage);

    static void saveResultInFile(
    const std::string& outputFileName, int height, int width, const std::vector<std::vector<Color>>& outLuminance);

    std::vector<std::vector<Ray>> getRays();

    int GetHeight();
    int GetWidth();

    void initFluxes();

private:
    struct HitPoint
    {
        Vec3f point;
        int   triangleId;
    };

    struct SparseRenderInfo
    {
        std::vector<Ray>                  sparseRays;
        std::vector<Color>                outLuminance;
        std::vector<std::vector<RayPath>> rayPaths;
    };

private:
    Light*                            chooseLight() const;
    Geometry*                         getIntersection(const Ray& ray, float& t, Vec3f& N) const;
    Geometry*                         getIntersection(const Ray& ray, float& t, Vec3f& N, int& geometryIndex) const;
    void renderImage(
        int                                  height,
        int                                  width,
        const std::vector<std::vector<Ray>>& rays,
        std::vector<std::vector<Color>>&     outLuminance);

private:
    static const int   ANTIALIASING_FACTOR = 4;
    static const int   MAX_RENDER_DEPTH    = 1000;
    static const int   PHASES              = 10;
    static const float EPS;

    float              totalFlux = 0;
    std::vector<float> fluxes;

    Scene*           scene;
    SparseRenderInfo sparseRenderInfo;
};
