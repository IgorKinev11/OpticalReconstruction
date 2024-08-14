cmake_minimum_required(VERSION 3.17.5)
project(OpticalReconstruction)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -fopenmp)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR})

set(${PROJECT_NAME}_SOURCE_FILES
        structs.h
        Math.h
        Camera.h
        Camera.cpp
        Color.h
        Color.cpp
        Geometry.h
        Geometry.cpp
        Ray.h
        Light.h
        Light.cpp
        Render.h
        Render.cpp
        Scene.h
        Scene.cpp
        Material.h
        Material.cpp
        SurfaceOpticProperties.h
        SurfaceOpticProperties.cpp
        BRDF.h
        BRDF.cpp
        Gradient.cpp
        Gradient.ð
        Curve.cpp
        Curve.h)

add_executable(main main.cpp ${${PROJECT_NAME}_SOURCE_FILES})
target_compile_definitions(main PRIVATE SOURCE_DIR=\"${SOURCE_DIR}\")
