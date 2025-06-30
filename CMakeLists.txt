cmake_minimum_required(VERSION 3.16.0)
project(Pyramid VERSION 0.3.3)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(PYRAMID_BUILD_TESTS "Build test projects" OFF)
option(PYRAMID_BUILD_EXAMPLES "Build example projects" ON)
option(PYRAMID_BUILD_TOOLS "Build tool projects" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add subdirectories
add_subdirectory(vendor/glad)
add_subdirectory(Engine)

if(PYRAMID_BUILD_EXAMPLES)
    add_subdirectory(Examples/BasicGame)
endif()

if(PYRAMID_BUILD_TOOLS)
    add_subdirectory(Tools/AssetProcessor)
endif()

if(PYRAMID_BUILD_TESTS)
    enable_testing()
    # add_subdirectory(Tests/Unit)
    # add_subdirectory(Tests/Integration)
endif()

# Install rules
include(GNUInstallDirs)

install(DIRECTORY Engine/Core/include/ Engine/Graphics/include/ Engine/Platform/include/
        Engine/Math/include/ Engine/Utils/include/ Engine/Renderer/include/
        Engine/Input/include/ Engine/Scene/include/ Engine/Audio/include/
        Engine/Physics/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

install(TARGETS PyramidEngine
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
