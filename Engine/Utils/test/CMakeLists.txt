add_executable(TestPNGComponents TestPNGComponents.cpp)

target_link_libraries(TestPNGComponents
    PRIVATE
        PyramidEngine
)

target_include_directories(TestPNGComponents
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestPNGComponents PROPERTIES FOLDER "Tests")

# JPEG Simple Test
add_executable(TestJPEGSimple TestJPEGSimple.cpp)

target_link_libraries(TestJPEGSimple
    PRIVATE
        PyramidEngine
)

target_include_directories(TestJPEGSimple
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestJPEGSimple PROPERTIES FOLDER "Tests")

# JPEG Huffman Test
add_executable(TestJPEGHuffman TestJPEGHuffman.cpp)

target_link_libraries(TestJPEGHuffman
    PRIVATE
        PyramidEngine
)

target_include_directories(TestJPEGHuffman
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestJPEGHuffman PROPERTIES FOLDER "Tests")

# JPEG Huffman Debug Test
add_executable(TestJPEGHuffmanDebug TestJPEGHuffmanDebug.cpp)

target_link_libraries(TestJPEGHuffmanDebug
    PRIVATE
        PyramidEngine
)

target_include_directories(TestJPEGHuffmanDebug
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestJPEGHuffmanDebug PROPERTIES FOLDER "Tests")
