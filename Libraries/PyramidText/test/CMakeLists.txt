add_executable(PyramidTextTests TextTests.cpp)
target_link_libraries(PyramidTextTests PRIVATE Pyramid::Text)
set_target_properties(PyramidTextTests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME Text.DebugFont COMMAND $<TARGET_FILE:PyramidTextTests>)
