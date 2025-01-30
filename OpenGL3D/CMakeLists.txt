add_library(OpenGL3D STATIC
    vendor/glad/src/glad.c
    vendor/glad/src/glad_wgl.c
    source/OpenGL3D/Game/OglGame.cpp
    source/OpenGL3D/Game/Win32Src/Win32GameSrc.cpp
    source/OpenGL3D/Graphic/OpenGLGraphicEngine.cpp
    source/OpenGL3D/Graphic/Win32Src/Win32GraphicSrc.cpp
    source/OpenGL3D/Window/Win32OpenGLWindow.cpp
)

target_include_directories(OpenGL3D
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include
)

# Find OpenGL
find_package(OpenGL REQUIRED)

target_link_libraries(OpenGL3D
    PUBLIC
        OpenGL::GL
)

if(WIN32)
    target_link_libraries(OpenGL3D
        PUBLIC
            opengl32
            gdi32
            user32
    )

    target_compile_definitions(OpenGL3D 
        PUBLIC
            UNICODE
            _UNICODE
            WIN32_LEAN_AND_MEAN
    )
endif()
