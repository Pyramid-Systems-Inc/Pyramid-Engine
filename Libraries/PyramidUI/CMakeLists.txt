add_library(PyramidUI)
add_library(Pyramid::UI ALIAS PyramidUI)

set_target_properties(PyramidUI PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    DEBUG_POSTFIX d
    EXPORT_NAME UI
    FOLDER "Libraries"
)

target_sources(PyramidUI
    PRIVATE
        source/UI.cpp
        source/TextEditing.cpp
        source/GameUI.cpp
    PRIVATE
        include/Pyramid/UI/UI.hpp
        include/Pyramid/UI/GameUI.hpp
)

target_include_directories(PyramidUI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(PyramidUI
    PUBLIC
        Pyramid::Foundation
        Pyramid::Math
        Pyramid::Input
        Pyramid::Text
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PyramidUI PRIVATE -Wall -Wextra -Wpedantic)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidUI PRIVATE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(PyramidUI PRIVATE /W4 /permissive-)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidUI PRIVATE /WX)
    endif()
endif()

if(PYRAMID_BUILD_TESTS)
    add_subdirectory(test)
endif()
