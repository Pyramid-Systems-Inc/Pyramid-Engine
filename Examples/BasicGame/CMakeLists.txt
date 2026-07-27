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
