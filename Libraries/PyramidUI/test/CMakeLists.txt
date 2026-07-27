add_executable(PyramidUITests UITests.cpp)
target_link_libraries(PyramidUITests PRIVATE Pyramid::UI)
set_target_properties(PyramidUITests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME UI.Context COMMAND $<TARGET_FILE:PyramidUITests>)
