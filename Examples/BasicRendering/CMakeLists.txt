cmake_minimum_required(VERSION 3.16)

# Basic Rendering Example
project(BasicRenderingExample)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Add executable
add_executable(BasicRenderingExample WIN32
Main.cpp
BasicRendering.cpp
)

# Link with Pyramid Engine
target_link_libraries(BasicRenderingExample PRIVATE PyramidEngine)

# Set include directories
target_include_directories(BasicRenderingExample
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set output directory
set_target_properties(BasicRenderingExample PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Examples/BasicRendering"
)