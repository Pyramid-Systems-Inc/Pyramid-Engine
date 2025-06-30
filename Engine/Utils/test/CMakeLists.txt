add_executable(TestPNGComponents TestPNGComponents.cpp)

target_link_libraries(TestPNGComponents
    PRIVATE
        PyramidEngine
)

target_include_directories(TestPNGComponents
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestPNGComponents PROPERTIES FOLDER "Tests")

# JPEG Simple Test
add_executable(TestJPEGSimple TestJPEGSimple.cpp)

target_link_libraries(TestJPEGSimple
    PRIVATE
        PyramidEngine
)

target_include_directories(TestJPEGSimple
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestJPEGSimple PROPERTIES FOLDER "Tests")
