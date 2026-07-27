add_executable(PyramidModelObjImportTests ObjImportTests.cpp)
target_link_libraries(PyramidModelObjImportTests PRIVATE Pyramid::Model)
set_target_properties(PyramidModelObjImportTests PROPERTIES FOLDER "Tests/Model")
add_test(NAME Model.ObjImport COMMAND $<TARGET_FILE:PyramidModelObjImportTests>)

add_executable(PyramidModelObjFailureTests ObjFailureTests.cpp)
target_link_libraries(PyramidModelObjFailureTests PRIVATE Pyramid::Model)
set_target_properties(PyramidModelObjFailureTests PROPERTIES FOLDER "Tests/Model")
add_test(NAME Model.ObjFailures COMMAND $<TARGET_FILE:PyramidModelObjFailureTests>)

add_executable(PyramidModelObjFileTests ObjFileTests.cpp)
target_link_libraries(PyramidModelObjFileTests PRIVATE Pyramid::Model)
set_target_properties(PyramidModelObjFileTests PROPERTIES FOLDER "Tests/Model")
add_test(NAME Model.ObjFileImport COMMAND $<TARGET_FILE:PyramidModelObjFileTests>)
