add_executable(BasicRenderingExample WIN32
    Main.cpp
    BasicRendering.cpp
    BasicRendering.hpp
)

target_link_libraries(BasicRenderingExample PRIVATE Pyramid::Engine)
target_include_directories(BasicRenderingExample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(BasicRenderingExample PROPERTIES FOLDER "Examples")

add_custom_command(TARGET BasicRenderingExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:BasicRenderingExample>/Assets"
    COMMENT "Copying BasicRendering model assets"
)
