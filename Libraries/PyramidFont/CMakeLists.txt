add_library(PyramidFont)
add_library(Pyramid::Font ALIAS PyramidFont)

set_target_properties(PyramidFont PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    DEBUG_POSTFIX d
    EXPORT_NAME Font
    FOLDER "Libraries"
)

target_sources(PyramidFont
    PRIVATE
        source/Font.cpp
    PRIVATE
        include/Pyramid/Font/Font.hpp
)

target_include_directories(PyramidFont
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(PyramidFont PUBLIC Pyramid::Foundation)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PyramidFont PRIVATE -Wall -Wextra -Wpedantic)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidFont PRIVATE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(PyramidFont PRIVATE /W4 /permissive-)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidFont PRIVATE /WX)
    endif()
endif()

if(PYRAMID_BUILD_TESTS)
    add_subdirectory(test)
endif()
