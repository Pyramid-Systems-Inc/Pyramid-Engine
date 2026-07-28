add_executable(PyramidTextTests TextTests.cpp)
target_link_libraries(PyramidTextTests PRIVATE Pyramid::Text)
target_compile_definitions(PyramidTextTests PRIVATE
    PYRAMID_TEXT_TEST_FONT="${PROJECT_SOURCE_DIR}/Examples/BasicGame/Assets/Fonts/PyramidSans-24.pfont"
)
set_target_properties(PyramidTextTests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME Text.DebugFont COMMAND $<TARGET_FILE:PyramidTextTests>)

if(TARGET PyramidMinGWRuntime)
    get_property(_pyramid_library_test_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_pyramid_library_test_target IN LISTS _pyramid_library_test_targets)
        get_target_property(_pyramid_library_test_type "${_pyramid_library_test_target}" TYPE)
        if(_pyramid_library_test_type STREQUAL "EXECUTABLE")
            add_dependencies("${_pyramid_library_test_target}" PyramidMinGWRuntime)
        endif()
    endforeach()
endif()
