add_library(PyramidModel)
add_library(Pyramid::Model ALIAS PyramidModel)

set_target_properties(PyramidModel PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    DEBUG_POSTFIX d
    EXPORT_NAME Model
    FOLDER "Libraries"
)

target_sources(PyramidModel
    PRIVATE
        source/Model.cpp
        source/ObjImporter.cpp
    PRIVATE
        include/Pyramid/Model/Model.hpp
        include/Pyramid/Model/ObjImporter.hpp
)

target_include_directories(PyramidModel
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(PyramidModel
    PUBLIC
        Pyramid::Foundation
        Pyramid::Math
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PyramidModel PRIVATE -Wall -Wextra -Wpedantic)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidModel PRIVATE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(PyramidModel PRIVATE /W4 /permissive-)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidModel PRIVATE /WX)
    endif()
endif()

if(PYRAMID_BUILD_TESTS)
    add_subdirectory(test)
endif()
