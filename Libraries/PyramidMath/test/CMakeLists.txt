add_executable(PyramidMathTests MathTests.cpp)
target_link_libraries(PyramidMathTests PRIVATE Pyramid::Math)
set_target_properties(PyramidMathTests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME Math.Core COMMAND $<TARGET_FILE:PyramidMathTests>)
