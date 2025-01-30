add_library(PyramidEngine)

# Add subdirectories
add_subdirectory(Core)
add_subdirectory(Graphics)
add_subdirectory(Platform)
add_subdirectory(Math)
add_subdirectory(Utils)
add_subdirectory(Renderer)
add_subdirectory(Input)
add_subdirectory(Scene)
add_subdirectory(Audio)
add_subdirectory(Physics)

# Set target properties
set_target_properties(PyramidEngine PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    DEBUG_POSTFIX d
)

# Link dependencies
target_link_libraries(PyramidEngine
    PRIVATE
        glad
        opengl32
)

# IDE organization
set_target_properties(PyramidEngine PROPERTIES FOLDER "Engine")
