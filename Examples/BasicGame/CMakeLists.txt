add_executable(BasicGame WIN32
    source/Main.cpp
    source/BasicGame.cpp
    include/BasicGame.hpp
)

target_link_libraries(BasicGame PRIVATE PyramidEngine)

target_include_directories(BasicGame
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/vendor
        ${CMAKE_SOURCE_DIR}/vendor/glad/include
)

set_target_properties(BasicGame PROPERTIES
    FOLDER "Examples"
    DEBUG_POSTFIX d
)
