add_library(PyramidImage)
add_library(Pyramid::Image ALIAS PyramidImage)

set_target_properties(PyramidImage PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    DEBUG_POSTFIX d
    EXPORT_NAME Image
    FOLDER "Libraries"
)

target_sources(PyramidImage
    PRIVATE
        source/BitReader.cpp
        source/HuffmanDecoder.cpp
        source/Image.cpp
        source/Inflate.cpp
        source/JPEGLoader.cpp
        source/PNGLoader.cpp
        source/ZLib.cpp
)

target_include_directories(PyramidImage
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PyramidImage PRIVATE -Wall -Wextra -Wpedantic)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidImage PRIVATE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(PyramidImage PRIVATE /W4 /permissive-)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidImage PRIVATE /WX)
    endif()
endif()

if(PYRAMID_BUILD_TESTS)
    add_subdirectory(test)
endif()
