cmake_minimum_required(VERSION 3.23)
project(PyramidFontConsumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PyramidFont CONFIG REQUIRED)

add_executable(PyramidFontConsumer main.cpp)
target_link_libraries(PyramidFontConsumer PRIVATE Pyramid::Font)
