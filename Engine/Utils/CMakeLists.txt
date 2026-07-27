target_sources(PyramidEngine
    PRIVATE
        source/Log.cpp
)

target_include_directories(PyramidEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
