add_executable(PyramidUITests UITests.cpp)
target_link_libraries(PyramidUITests PRIVATE Pyramid::UI)
target_compile_definitions(PyramidUITests PRIVATE
    PYRAMID_UI_LIBRARY_TEST_FONT="${PROJECT_SOURCE_DIR}/Examples/BasicGame/Assets/Fonts/PyramidSans-24.pfont"
)
set_target_properties(PyramidUITests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME UI.Context COMMAND $<TARGET_FILE:PyramidUITests>)

add_executable(PyramidGameUITests GameUITests.cpp)
target_link_libraries(PyramidGameUITests PRIVATE Pyramid::UI)
set_target_properties(PyramidGameUITests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME UI.GameScreens COMMAND $<TARGET_FILE:PyramidGameUITests>)

add_executable(PyramidUITextEditingTests TextEditingTests.cpp)
target_link_libraries(PyramidUITextEditingTests PRIVATE Pyramid::UI)
target_compile_definitions(PyramidUITextEditingTests PRIVATE
    PYRAMID_UI_TEST_FONT="${PROJECT_SOURCE_DIR}/Examples/BasicGame/Assets/Fonts/PyramidSans-24.pfont"
    PYRAMID_UI_TEST_ARABIC_FONT="${PROJECT_SOURCE_DIR}/Examples/BasicGame/Assets/Fonts/PyramidArabic-24.pfont"
)
set_target_properties(PyramidUITextEditingTests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME UI.TextEditing COMMAND $<TARGET_FILE:PyramidUITextEditingTests>)

if(TARGET PyramidMinGWRuntime)
    get_property(_pyramid_library_test_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_pyramid_library_test_target IN LISTS _pyramid_library_test_targets)
        get_target_property(_pyramid_library_test_type "${_pyramid_library_test_target}" TYPE)
        if(_pyramid_library_test_type STREQUAL "EXECUTABLE")
            add_dependencies("${_pyramid_library_test_target}" PyramidMinGWRuntime)
        endif()
    endforeach()
endif()
