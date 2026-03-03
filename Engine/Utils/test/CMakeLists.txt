function(add_utils_test target_name source_file)
    add_executable(${target_name} ${source_file})

    target_link_libraries(${target_name}
        PRIVATE
            PyramidEngine
    )

    target_include_directories(${target_name}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )

    set_target_properties(${target_name} PROPERTIES FOLDER "Tests")
    add_test(NAME Utils.${target_name} COMMAND $<TARGET_FILE:${target_name}>)
endfunction()

add_utils_test(TestPNGComponents TestPNGComponents.cpp)
add_utils_test(TestJPEGSimple TestJPEGSimple.cpp)
add_utils_test(TestJPEGHuffman TestJPEGHuffman.cpp)
add_utils_test(TestJPEGHuffmanDebug TestJPEGHuffmanDebug.cpp)
add_utils_test(TestJPEGDequantizer TestJPEGDequantizer.cpp)
add_utils_test(TestJPEGIDCT TestJPEGIDCT.cpp)
add_utils_test(TestJPEGColorConverter TestJPEGColorConverter.cpp)
add_utils_test(TestJPEGIntegration TestJPEGIntegration.cpp)

