cmake_minimum_required(VERSION 3.23)
project(PyramidImageConsumer LANGUAGES CXX)

find_package(PyramidImage CONFIG REQUIRED)

add_executable(PyramidImageConsumer main.cpp)
target_link_libraries(PyramidImageConsumer PRIVATE Pyramid::Image)
target_compile_features(PyramidImageConsumer PRIVATE cxx_std_17)
