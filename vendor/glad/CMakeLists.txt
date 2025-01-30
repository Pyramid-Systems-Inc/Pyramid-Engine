cmake_minimum_required(VERSION 3.16.0)

add_library(glad
    src/glad.c
    src/glad_wgl.c
)

target_include_directories(glad
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(glad PROPERTIES
    FOLDER "vendor"
)
