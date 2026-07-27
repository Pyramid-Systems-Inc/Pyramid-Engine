add_library(PyramidInput)
add_library(Pyramid::Input ALIAS PyramidInput)

set_target_properties(PyramidInput PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    DEBUG_POSTFIX d
    EXPORT_NAME Input
    FOLDER "Libraries"
)

target_sources(PyramidInput
    PRIVATE
        source/Input.cpp
        source/InputActions.cpp
    PRIVATE
        include/Pyramid/Platform/Input.hpp
        include/Pyramid/Input/InputActions.hpp
)

target_include_directories(PyramidInput
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(PyramidInput PUBLIC Pyramid::Foundation)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PyramidInput PRIVATE -Wall -Wextra -Wpedantic)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidInput PRIVATE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(PyramidInput PRIVATE /W4 /permissive-)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidInput PRIVATE /WX)
    endif()
endif()
