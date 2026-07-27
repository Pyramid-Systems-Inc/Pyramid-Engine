add_library(PyramidMath)
add_library(Pyramid::Math ALIAS PyramidMath)

set_target_properties(PyramidMath PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    DEBUG_POSTFIX d
    EXPORT_NAME Math
    FOLDER "Libraries"
)

target_sources(PyramidMath
    PRIVATE
        source/Vec2.cpp
        source/Vec3.cpp
        source/Vec4.cpp
        source/Mat3.cpp
        source/Mat4.cpp
        source/Quat.cpp
        source/MathSIMD.cpp
    PRIVATE
        include/Pyramid/Math/MathCommon.hpp
        include/Pyramid/Math/Vec2.hpp
        include/Pyramid/Math/Vec3.hpp
        include/Pyramid/Math/Vec4.hpp
        include/Pyramid/Math/Mat3.hpp
        include/Pyramid/Math/Mat4.hpp
        include/Pyramid/Math/Quat.hpp
        include/Pyramid/Math/MathPerformance.hpp
        include/Pyramid/Math/MathSIMD.hpp
        include/Pyramid/Math/Math.hpp
)

target_include_directories(PyramidMath
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(PyramidMath PUBLIC Pyramid::Foundation)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PyramidMath PRIVATE -Wall -Wextra -Wpedantic)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidMath PRIVATE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(PyramidMath PRIVATE /W4 /permissive-)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidMath PRIVATE /WX)
    endif()
endif()

if(PYRAMID_BUILD_TESTS)
    add_subdirectory(test)
endif()
