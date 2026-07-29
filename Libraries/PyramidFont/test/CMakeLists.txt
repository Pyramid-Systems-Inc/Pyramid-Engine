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

add_executable(PyramidReferenceTypographyTests ReferenceTypographyTests.cpp)
target_link_libraries(PyramidReferenceTypographyTests PRIVATE Pyramid::Font)
target_compile_definitions(PyramidReferenceTypographyTests PRIVATE
    PYRAMID_FONT_TEST_ARABIC_SOURCE="${PROJECT_SOURCE_DIR}/Examples/BasicGame/Assets/Fonts/PyramidArabic.ttf"
    PYRAMID_FONT_TEST_ARABIC_PROCESSED="${PROJECT_SOURCE_DIR}/Examples/BasicGame/Assets/Fonts/PyramidArabic-48.pfont"
)
set_target_properties(PyramidReferenceTypographyTests PROPERTIES FOLDER "Tests/Libraries")
add_test(NAME Font.ReferenceTypography COMMAND $<TARGET_FILE:PyramidReferenceTypographyTests>)

if(TARGET PyramidMinGWRuntime)
    add_dependencies(PyramidReferenceTypographyTests PyramidMinGWRuntime)
endif()
