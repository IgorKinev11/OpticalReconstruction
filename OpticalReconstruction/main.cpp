#include "Render.h"
#include "Scene.h"
#include "Gradient.h"

#include <vector>

int main()
{
    Color::setWaveLengths(std::vector<int>{400, 500, 600, 700});

    Scene       scene;
    std::string geometryFile      = std::string(SOURCE_DIR) + "/cornel_box0/small_cube_camera/cornel_box0.shp";
    std::string materialsFile     = std::string(SOURCE_DIR) + "/cornel_box0/small_cube_camera/cube_materials.txt";
    std::string cameraFile        = std::string(SOURCE_DIR) + "/cornel_box0/small_cube_camera/small_cube.txt";
    std::string lightsFile        = std::string(SOURCE_DIR) + "/cornel_box0/lights.txt";
    std::string curveFile         = std::string(SOURCE_DIR) + "/cornel_box0/Curve.txt";
    std::string resultPath        = std::string(SOURCE_DIR) + "/result/output_test_1.txt"
    std::string originalImagePath = std::string(SOURCE_DIR) + "/result/small_cube/small_kd_0_2_10.txt";
    
    scene.readMaterials(materialsFile);
    scene.readGeometry(geometryFile);
    scene.readCamera(cameraFile);
    scene.readLights(lightsFile);

    Color::readCurve(curveFile);

    Render render(&scene);
    DiffRender diffRender(&render);

    int                             height = render.GetHeight();
    int                             width  = render.GetWidth();
    std::vector<std::vector<Color>> originalImage(height, std::vector<Color>(width));
    diffRender.ReadOriginalImage(originalImagePath, height, width, originalImage);

    float stepSize     = 0.001;
    float learningRate = 0.0001;
    float epsilon      = 0.00000001;
    int   numOfEpochs  = 10;
    diffRender.gradientDescent(originalImage, stepSize, learningRate, epsilon, numOfEpochs);

}
