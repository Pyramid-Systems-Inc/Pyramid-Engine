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

# JPEG Parser Test
add_executable(TestJPEGParser TestJPEGParser.cpp)

target_link_libraries(TestJPEGParser
    PRIVATE
        PyramidEngine
)

target_include_directories(TestJPEGParser
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestJPEGParser PROPERTIES FOLDER "Tests")
