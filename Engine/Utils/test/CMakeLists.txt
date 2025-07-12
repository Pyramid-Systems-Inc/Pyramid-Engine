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

# JPEG Dequantizer Test
add_executable(TestJPEGDequantizer TestJPEGDequantizer.cpp)

target_link_libraries(TestJPEGDequantizer
    PRIVATE
        PyramidEngine
)

target_include_directories(TestJPEGDequantizer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestJPEGDequantizer PROPERTIES FOLDER "Tests")

# JPEG IDCT Test
add_executable(TestJPEGIDCT TestJPEGIDCT.cpp)

target_link_libraries(TestJPEGIDCT
    PRIVATE
        PyramidEngine
)

target_include_directories(TestJPEGIDCT
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestJPEGIDCT PROPERTIES FOLDER "Tests")

# JPEG Color Converter Test
add_executable(TestJPEGColorConverter TestJPEGColorConverter.cpp)

target_link_libraries(TestJPEGColorConverter
    PRIVATE
        PyramidEngine
)

target_include_directories(TestJPEGColorConverter
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestJPEGColorConverter PROPERTIES FOLDER "Tests")

# JPEG Integration Test
add_executable(TestJPEGIntegration TestJPEGIntegration.cpp)

target_link_libraries(TestJPEGIntegration
    PRIVATE
        PyramidEngine
)

target_include_directories(TestJPEGIntegration
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

set_target_properties(TestJPEGIntegration PROPERTIES FOLDER "Tests")
