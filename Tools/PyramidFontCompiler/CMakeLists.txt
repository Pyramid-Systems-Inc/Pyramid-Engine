add_executable(PyramidFontCompiler
    main.cpp
    ${PROJECT_SOURCE_DIR}/assets/branding/pyramid-engine.rc
)
target_link_libraries(PyramidFontCompiler PRIVATE Pyramid::Font)
set_target_properties(PyramidFontCompiler PROPERTIES FOLDER "Tools")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(PyramidFontCompiler PRIVATE -Wall -Wextra -Wpedantic)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidFontCompiler PRIVATE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(PyramidFontCompiler PRIVATE /W4 /permissive-)
    if(PYRAMID_WARNINGS_AS_ERRORS)
        target_compile_options(PyramidFontCompiler PRIVATE /WX)
    endif()
endif()

pyramid_require_mingw_runtime(PyramidFontCompiler)
install(TARGETS PyramidFontCompiler RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT PyramidTools)
