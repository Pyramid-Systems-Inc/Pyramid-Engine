# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# GLAD
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL3D/vendor/glad/include
)

# Add other dependencies as needed
# Examples:
# find_package(SDL2 REQUIRED)
# find_package(GLFW REQUIRED)
# find_package(GLM REQUIRED)
