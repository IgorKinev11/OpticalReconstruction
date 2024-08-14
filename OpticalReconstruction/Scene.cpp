#include "Scene.h"
#include "Color.h"
#include "Math.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <unordered_map>
#include <map>

#include <filesystem>

enum GeometryParseState
{
    INITIAL,
    VERTICES,
    TRIANGLES
};

void Scene::readSceneFromFiles(
    const std::string& geometryFile,
    const std::string& materialsFile,
    const std::string& lightsFile,
    const std::string& cameraFile)
{
    readGeometry(geometryFile);
    readMaterials(materialsFile);
    readLights(lightsFile);
    readCamera(cameraFile);
}

Scene::~Scene()
{
    for (Geometry* geom : geometry_)
        delete geom;

    for (Light* light : lights_)
        delete light;

    for (Material* material : materials_)
        delete material;

    delete camera_;
}

void Scene::clearMaterials()
{
    materials_.clear();
}

void Scene::readGeometry(const std::string& fileName)
{
    std::ifstream in(fileName);

    if (!in.is_open())
    {
        std::cerr << "Failed to open the file: " << fileName << std::endl;
        return;
    }

    std::string           line;
    GeometryParseState    state = GeometryParseState::INITIAL;
    std::vector<Vec3f>    vertices;
    std::vector<Triangle> triangles;
    Mesh*                 mesh      = nullptr;
    int                   matrialID = 0;

    while (getline(in, line))
    {
        switch (state)
        {
            case GeometryParseState::INITIAL:
            {
                if (line.find("vertices") != std::string::npos)
                {
                    state = GeometryParseState::VERTICES;
                }
                break;
            }
            case GeometryParseState::VERTICES:
            {
                if (line.find("triangles") != std::string::npos)
                {
                    state            = GeometryParseState::TRIANGLES;
                    mesh             = new Mesh();
                    mesh->ownPoints_ = vertices;
                    break;
                }
                std::istringstream iss(line);
                double             x, y, z;
                iss >> x >> z >> y; // FIXME: Это странно
                vertices.emplace_back(x, y, z);
                break;
            }
            case GeometryParseState::TRIANGLES:
            {
                if (line.find("parts") != std::string::npos)
                {
                    state            = GeometryParseState::INITIAL;
                    mesh->triangles_ = std::move(triangles);
                    geometry_.emplace_back(mesh);
                    geometry_.back()->materialId_ = matrialID++;
                    vertices.clear();
                    triangles.clear();
                    break;
                }
                std::istringstream iss(line);
                int                v1, v2, v3;
                iss >> v1 >> v2 >> v3;

                triangles.emplace_back(mesh, v1, v2, v3);
                break;
            }
        }
    }

    in.close();
}

void Scene::readMaterials(const std::string& fileName)
{
    std::ifstream in(fileName);

    std::string line;
    int         currentObjectId = -1;

    std::map<coefs, float> coefs_map; // kd, ks, ktd, kts, brdf

    std::vector<float> colors;
    std::vector<int>   waveLengths;

    while (getline(in, line))
    {
        if (line.find("id") != std::string::npos)
        {
            if (currentObjectId != -1)
            {
                Color color;
                color.setColors(colors);
                
                materials_.push_back(new Material(color, coefs_map));

                waveLengths.clear();
                colors.clear();
            }
            ++currentObjectId;

            getline(in, line);
            std::istringstream iss(line);
            iss >> coefs_map[coefs::kd];
            //iss >> coefs_map[coefs::ks];
            //iss >> coefs_map[coefs::ktd];
            //iss >> coefs_map[coefs::kts];

            continue;
        }

        if (line.empty() || line == "\r")
        {
            continue;
        }

        std::istringstream iss(line);
        int                waveLength;
        float              color;
        iss >> waveLength >> color;
        waveLengths.push_back(waveLength);
        colors.push_back(color);
    }

    in.close();
}

void Scene::readLights(const std::string& fileName)
{
    std::ifstream                   in(fileName);
    Vec3f                           origin;
    Vec3f                           normal;
    std::vector<float>              intensityTable;
    std::unordered_map<int, double> kds;
    in >> origin.x >> origin.y >> origin.z;
    in >> normal.x >> normal.y >> normal.z;

    std::string line;
    getline(in, line); // '\n'
    getline(in, line);
    std::istringstream iss(line);
    std::string        element;
    while (std::getline(iss, element, ' '))
    {
        intensityTable.push_back(std::stof(element));
    }

    Color              color;
    std::vector<float> colors;

    while (getline(in, line))
    {
        std::istringstream iss(line);
        int                waveLength;
        double             kd;
        iss >> waveLength >> kd;
        colors.push_back(kd);
    }

    color.setColors(std::move(colors));

    //геометрия для протяженного источника света
    std::vector<Vec3f> verts{
        Vec3f{343.0, -227.0, 548}, Vec3f{343.0, -332.0, 548}, Vec3f{213.0, -332.0, 548}, Vec3f{213.0, -227.0, 548}};

    Mesh* lightMesh       = new Mesh();
    lightMesh->ownPoints_ = verts;
    lightMesh->triangles_.emplace_back(lightMesh, 0, 1, 2);
    lightMesh->triangles_.emplace_back(lightMesh, 2, 3, 0);
    lightMesh->materialId_ = 2; // HARDCODE????
    geometry_.push_back(lightMesh);

    RectangleLight* light          = new RectangleLight{color, intensityTable, origin, normal, *lightMesh};
    geometry_.back()->sourceLight_ = light;
    lights_.emplace_back(light);
    in.close();
}

 void Scene::readCamera(const std::string& fileName)
{
    std::ifstream in(fileName);
    std::string   line;

    int   id, width, height;
    float fov;
    Vec3f origin, target, luc, ruc, ldc;

    while (getline(in, line))
    {
        if (line.find("id") != std::string::npos)
        {
            in >> id;
            in >> origin.x >> origin.y >> origin.z;
            in >> target.x >> target.y >> target.z;
            in >> luc.x >> luc.y >> luc.z;
            in >> ruc.x >> ruc.y >> ruc.z;
            in >> ldc.x >> ldc.y >> ldc.z;
            in >> width >> height;
            in >> fov;

            camera_ = new Camera(id, origin, target, luc, ruc, ldc, width, height, fov);
        }
    }

    in.close();
}
