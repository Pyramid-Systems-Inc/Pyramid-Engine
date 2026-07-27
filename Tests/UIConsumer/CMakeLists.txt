cmake_minimum_required(VERSION 3.23)
project(PyramidUIConsumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PyramidUI CONFIG REQUIRED)

add_executable(PyramidUIConsumer main.cpp)
target_link_libraries(PyramidUIConsumer PRIVATE Pyramid::UI)
