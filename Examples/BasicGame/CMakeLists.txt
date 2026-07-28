add_executable(BasicGame WIN32
    source/Main.cpp
    source/BasicGame.cpp
    include/BasicGame.hpp
)

target_link_libraries(BasicGame PRIVATE Pyramid::Engine Pyramid::RTSReference)

target_include_directories(BasicGame
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(BasicGame PROPERTIES FOLDER "Examples")

pyramid_require_mingw_runtime(BasicGame)
install(TARGETS BasicGame RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT PyramidExamples)

set(PYRAMID_BASIC_GAME_FONT_ASSETS
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Fonts/PyramidSans.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Fonts/PyramidSans-24.pfont"
)
add_custom_command(TARGET BasicGame POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:BasicGame>/Fonts"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        ${PYRAMID_BASIC_GAME_FONT_ASSETS}
        "$<TARGET_FILE_DIR:BasicGame>/Fonts"
    VERBATIM
)
install(FILES ${PYRAMID_BASIC_GAME_FONT_ASSETS}
    DESTINATION "${CMAKE_INSTALL_BINDIR}/Fonts"
    COMPONENT PyramidExamples
)
