add_executable(PyramidTextTests TextTests.cpp)
target_link_libraries(PyramidTextTests PRIVATE Pyramid::Text)
target_compile_definitions(PyramidTextTests PRIVATE
    PYRAMID_TEXT_TEST_FONT="${PROJECT_SOURCE_DIR}/Examples/BasicGame/Assets/Fonts/PyramidSans-64-sdf.pfont"
)
set_target_properties(PyramidTextTests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME Text.DebugFont COMMAND $<TARGET_FILE:PyramidTextTests>)


add_executable(PyramidInternationalTextTests InternationalTextTests.cpp)
target_link_libraries(PyramidInternationalTextTests PRIVATE Pyramid::Text)
target_compile_definitions(PyramidInternationalTextTests PRIVATE
    PYRAMID_TEXT_TEST_FONT="${PROJECT_SOURCE_DIR}/Examples/BasicGame/Assets/Fonts/PyramidSans-64-sdf.pfont"
    PYRAMID_TEXT_TEST_ARABIC_FONT="${PROJECT_SOURCE_DIR}/Examples/BasicGame/Assets/Fonts/PyramidArabic-64-sdf.pfont"
)
set_target_properties(PyramidInternationalTextTests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME Text.International COMMAND $<TARGET_FILE:PyramidInternationalTextTests>)

add_executable(PyramidTextBufferTests TextBufferTests.cpp)
target_link_libraries(PyramidTextBufferTests PRIVATE Pyramid::Text)
set_target_properties(PyramidTextBufferTests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME Text.TextBuffer COMMAND $<TARGET_FILE:PyramidTextBufferTests>)

if(TARGET PyramidMinGWRuntime)
    get_property(_pyramid_library_test_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_pyramid_library_test_target IN LISTS _pyramid_library_test_targets)
        get_target_property(_pyramid_library_test_type "${_pyramid_library_test_target}" TYPE)
        if(_pyramid_library_test_type STREQUAL "EXECUTABLE")
            add_dependencies("${_pyramid_library_test_target}" PyramidMinGWRuntime)
        endif()
    endforeach()
endif()
