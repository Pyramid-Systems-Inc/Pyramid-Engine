add_executable(PyramidFontTests FontTests.cpp)
target_link_libraries(PyramidFontTests PRIVATE Pyramid::Font)
target_compile_definitions(PyramidFontTests PRIVATE
    PYRAMID_FONT_TEST_ASSET="${PROJECT_SOURCE_DIR}/Examples/BasicGame/Assets/Fonts/PyramidSans.ttf"
)
set_target_properties(PyramidFontTests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME Font.TrueTypePipeline COMMAND $<TARGET_FILE:PyramidFontTests>)

if(TARGET PyramidMinGWRuntime)
    add_dependencies(PyramidFontTests PyramidMinGWRuntime)
endif()
