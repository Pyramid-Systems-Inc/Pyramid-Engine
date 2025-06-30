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
