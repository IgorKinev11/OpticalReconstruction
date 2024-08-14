#include "Camera.h"
#include "structs.h"

#include <cmath>
#include <cstdlib>

int Camera::getId() const
{
    return id;
}

Vec3f Camera::getEye() const
{
    return eye;
}

Vec3f Camera::getCenter() const
{
    return center;
}

Vec3f Camera::getLeftUpCorner() const
{
    return leftUpCorner;
}

Vec3f Camera::getRightUpCorner() const
{
    return rightUpCorner;
}

Vec3f Camera::getLeftDownCorner() const
{
    return leftDownCorner;
}

int Camera::getWidth() const
{
    return WIDTH;
}

int Camera::getHeight() const
{
    return HEIGHT;
}

float Camera::getFov() const
{
    return fov;
}

Camera::Camera(
    int   id,
    Vec3f eye,
    Vec3f center,
    Vec3f leftUpCorner,
    Vec3f rightUpCorner,
    Vec3f leftDownCorner,
    int   width,
    int   height,
    float fov)
    : id(id)
    , eye(eye)
    , center(center)
    , leftUpCorner(leftUpCorner)
    , rightUpCorner(rightUpCorner)
    , leftDownCorner(leftDownCorner)
    , WIDTH(width)
    , HEIGHT(height)
    , fov(fov)

{}

Ray Camera::castRay(int X, int Y)
{
    if (X == -1)
    {
        X = rand() % (HEIGHT);
        Y = rand() % (WIDTH);
    }

    Vec3f oxN      = (rightUpCorner - leftUpCorner).normalize();
    Vec3f oyN      = (leftDownCorner - leftUpCorner).normalize();
    float pixSizeX = (rightUpCorner - leftUpCorner).length() / WIDTH;
    float pixSizeY = (leftDownCorner - leftUpCorner).length() / HEIGHT;

    float lenghtX = pixSizeX * X;
    float lenghtY = pixSizeY * Y;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    double randNum1 = static_cast<double>(std::rand()) / RAND_MAX;
    randNum1        = randNum1 - (pixSizeX / 2);

    // float displacedX = lenghtX + (pixSizeX / 2) + randNum1;
    float displacedX = lenghtX + (pixSizeX / 2);
    // float displacedX = lenghtX;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    double randNum2 = static_cast<double>(std::rand()) / RAND_MAX;
    randNum2        = randNum2 - (pixSizeY / 2);

    // float displacedY = lenghtY + (pixSizeY / 2) + randNum2;
    float displacedY = lenghtY + (pixSizeY / 2);
    // float displacedY = lenghtY;

    Vec3f shiftX = leftUpCorner + (oxN * displacedX);
    Vec3f shiftY = shiftX + (oyN * displacedY);

    Vec3f direction = (shiftY - eye).normalize();

    Ray newRay;
    newRay.origin    = eye;
    newRay.direction = direction;
    /////////////цвет////////////////////////
    newRay.color = Color(1.0 / Color::waveLengths.size());

    return newRay;
}

std::vector<std::vector<Ray>> Camera::render()
{
    /*
     * output: matrix - матрица лучей (с их точками начала и направления)
     *
     * функция по лучу в каждую точку камеры
     * */
    std::vector<std::vector<Ray>> matrix(HEIGHT);
    for (int y = 0; y < HEIGHT; y++)
        matrix[y].resize(WIDTH);

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
            matrix[y][x] = castRay(x, y);
    }

    return matrix;
}