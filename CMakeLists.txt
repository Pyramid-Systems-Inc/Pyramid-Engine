cmake_minimum_required(VERSION 3.23)

project(Pyramid VERSION 0.6.0 LANGUAGES C CXX)

set(PYRAMID_VERSION_STATUS "pre-alpha" CACHE STRING "Human-readable release status")
set(PYRAMID_VERSION_STRING "${PROJECT_VERSION}-${PYRAMID_VERSION_STATUS}")

option(
    PYRAMID_ALLOW_UNSUPPORTED_HOST_CONFIGURE
    "Allow configure-only validation on unsupported hosts"
    OFF
)

if(NOT WIN32 AND NOT PYRAMID_ALLOW_UNSUPPORTED_HOST_CONFIGURE)
    message(FATAL_ERROR
        "Pyramid Engine currently supports Windows only. "
        "Use Windows 10/11 with a MinGW-w64 toolchain; MSYS2 UCRT64 is recommended.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PYRAMID_BUILD_TESTS "Build test projects" OFF)
option(PYRAMID_BUILD_EXAMPLES "Build example projects" ON)
option(PYRAMID_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

if(PYRAMID_BUILD_TESTS)
    include(CTest)
    enable_testing()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(vendor/glad)
add_subdirectory(Libraries/PyramidFoundation)
add_subdirectory(Libraries/PyramidMath)
add_subdirectory(Libraries/PyramidImage)
add_subdirectory(Libraries/PyramidInput)
add_subdirectory(Libraries/PyramidModel)
add_subdirectory(Engine)

if(PYRAMID_BUILD_EXAMPLES OR PYRAMID_BUILD_TESTS)
    add_subdirectory(Examples/RTSReference)
endif()

if(PYRAMID_BUILD_EXAMPLES)
    add_subdirectory(Examples/BasicGame)
    add_subdirectory(Examples/BasicRendering)
endif()

if(PYRAMID_BUILD_TESTS)
    add_subdirectory(Tests)
endif()

include(GNUInstallDirs)

install(
    DIRECTORY
        Engine/Core/include/
        Engine/Graphics/include/
        Engine/Platform/include/
        vendor/glad/include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT PyramidEngine
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

foreach(library IN ITEMS PyramidFoundation PyramidMath PyramidImage PyramidInput PyramidModel)
    install(
        DIRECTORY "${PROJECT_SOURCE_DIR}/Libraries/${library}/include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        COMPONENT ${library}
        FILES_MATCHING
            PATTERN "*.h"
            PATTERN "*.hpp"
    )
endforeach()

install(
    TARGETS PyramidEngine glad
    EXPORT PyramidEngineTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT PyramidEngine
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT PyramidEngine
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT PyramidEngine
)

foreach(library IN ITEMS PyramidFoundation PyramidMath PyramidImage PyramidInput PyramidModel)
    install(
        TARGETS ${library}
        EXPORT ${library}Targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT ${library}
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT ${library}
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT ${library}
    )
endforeach()

install(
    EXPORT PyramidEngineTargets
    FILE PyramidEngineTargets.cmake
    NAMESPACE Pyramid::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/PyramidEngine"
    COMPONENT PyramidEngine
)

foreach(library IN ITEMS PyramidFoundation PyramidMath PyramidImage PyramidInput PyramidModel)
    install(
        EXPORT ${library}Targets
        FILE ${library}Targets.cmake
        NAMESPACE Pyramid::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${library}"
        COMPONENT ${library}
    )
endforeach()

include(CMakePackageConfigHelpers)

foreach(package IN ITEMS PyramidEngine PyramidFoundation PyramidMath PyramidImage PyramidInput PyramidModel)
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/CMake/${package}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${package}Config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    )

    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES
            "${PROJECT_BINARY_DIR}/${package}Config.cmake"
            "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
        COMPONENT ${package}
    )
endforeach()

message(STATUS "Pyramid Engine version: ${PYRAMID_VERSION_STRING}")
