add_library(PyramidFoundation)
add_library(Pyramid::Foundation ALIAS PyramidFoundation)

set_target_properties(PyramidFoundation PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    DEBUG_POSTFIX d
    EXPORT_NAME Foundation
    FOLDER "Libraries"
)

target_sources(PyramidFoundation
    PRIVATE
        source/Prerequisites.cpp
        source/Log.cpp
    PRIVATE
        include/Pyramid/Core/Prerequisites.hpp
        include/Pyramid/Util/Log.hpp
)

target_include_directories(PyramidFoundation
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PyramidFoundation PRIVATE -Wall -Wextra -Wpedantic)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidFoundation PRIVATE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(PyramidFoundation PRIVATE /W4 /permissive-)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidFoundation PRIVATE /WX)
    endif()
endif()

if(PYRAMID_BUILD_TESTS)
    add_subdirectory(test)
endif()
