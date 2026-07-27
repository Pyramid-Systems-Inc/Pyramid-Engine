add_executable(PyramidFoundationTests FoundationTests.cpp)
target_link_libraries(PyramidFoundationTests PRIVATE Pyramid::Foundation)
set_target_properties(PyramidFoundationTests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME Foundation.Core COMMAND $<TARGET_FILE:PyramidFoundationTests>)
