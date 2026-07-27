cmake_minimum_required(VERSION 3.23)
project(PyramidLibrariesConsumer LANGUAGES CXX)

find_package(PyramidFoundation CONFIG REQUIRED)
find_package(PyramidMath CONFIG REQUIRED)
find_package(PyramidInput CONFIG REQUIRED)

add_executable(PyramidLibrariesConsumer main.cpp)
target_link_libraries(PyramidLibrariesConsumer
    PRIVATE
        Pyramid::Foundation
        Pyramid::Math
        Pyramid::Input
)
target_compile_features(PyramidLibrariesConsumer PRIVATE cxx_std_17)
