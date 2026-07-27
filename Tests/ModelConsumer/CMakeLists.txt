cmake_minimum_required(VERSION 3.23)
project(PyramidModelConsumer LANGUAGES CXX)

find_package(PyramidModel CONFIG REQUIRED)

add_executable(PyramidModelConsumer main.cpp)
target_link_libraries(PyramidModelConsumer PRIVATE Pyramid::Model)
target_compile_features(PyramidModelConsumer PRIVATE cxx_std_17)
