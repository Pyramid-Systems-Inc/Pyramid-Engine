cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

# Set project name and languages
project(Pyramids 
    VERSION 1.0.0
    DESCRIPTION "Pyramid Game Engine"
    LANGUAGES C CXX
)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Enable folders for IDE organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Platform specific settings
if(WIN32)
    set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
    
    if(NOT CMAKE_VS_PLATFORM_NAME)
        set(CMAKE_VS_PLATFORM_NAME "x64")
    endif()
    
    if(NOT ("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64"
         OR "${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86"))
        message(FATAL_ERROR "${CMAKE_VS_PLATFORM_NAME} arch is not supported!")
    endif()
    
    message(STATUS "Building for ${CMAKE_VS_PLATFORM_NAME} architecture")
endif()

# Build type configuration
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# Compiler specific options
if(MSVC)
    # Enable multi-processor compilation
    add_compile_options(/MP)
    
    # Use static runtime libraries
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # Warning level
    add_compile_options(/W4)
else()
    # GCC/Clang warnings
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add subdirectories first
add_subdirectory(OpenGL3D)
add_subdirectory(Game)

# Define source files
set(PYRAMID_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/source/OpenGL3D/Game/OglGame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/source/OpenGL3D/Game/Win32Src/Win32GameSrc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/source/OpenGL3D/Graphic/OpenGLGraphicEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/source/OpenGL3D/Graphic/Win32Src/Win32GraphicSrc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/source/OpenGL3D/Window/Win32OpenGLWindow.cpp
)

set(PYRAMID_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/include/OpenGL3D/Game/OglGame.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/include/OpenGL3D/Graphic/OpenGLGraphicEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/include/OpenGL3D/Helpers/OGLVector4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/include/OpenGL3D/OGLPrerequisites.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/include/OpenGL3D/Window/OglWindow.h
)

# Main library target
add_library(${PROJECT_NAME} STATIC ${PYRAMID_SOURCES} ${PYRAMID_HEADERS})

# Create source groups for better IDE organization
source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX "Source Files"
    FILES ${PYRAMID_SOURCES}
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX "Header Files"
    FILES ${PYRAMID_HEADERS}
)

# Include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/source
)

# Dependencies
include(CMake/Dependencies.cmake)

# Installation rules
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${PYRAMID_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)
