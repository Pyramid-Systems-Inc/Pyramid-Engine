add_library(PyramidText)
add_library(Pyramid::Text ALIAS PyramidText)

set_target_properties(PyramidText PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    DEBUG_POSTFIX d
    EXPORT_NAME Text
    FOLDER "Libraries"
)

target_sources(PyramidText
    PRIVATE
        source/Text.cpp
    PRIVATE
        include/Pyramid/Text/Text.hpp
)

target_include_directories(PyramidText
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(PyramidText
    PUBLIC
        Pyramid::Foundation
        Pyramid::Math
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PyramidText PRIVATE -Wall -Wextra -Wpedantic)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidText PRIVATE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(PyramidText PRIVATE /W4 /permissive-)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidText PRIVATE /WX)
    endif()
endif()

if(PYRAMID_BUILD_TESTS)
    add_subdirectory(test)
endif()
