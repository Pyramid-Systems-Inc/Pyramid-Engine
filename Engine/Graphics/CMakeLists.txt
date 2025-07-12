target_sources(PyramidEngine
    PRIVATE
        source/GraphicsDevice.cpp
        source/OpenGL/OpenGLDevice.cpp
        source/OpenGL/Buffer/OpenGLVertexBuffer.cpp
        source/OpenGL/Buffer/OpenGLIndexBuffer.cpp
        source/OpenGL/Buffer/OpenGLVertexArray.cpp
        source/OpenGL/Buffer/OpenGLUniformBuffer.cpp
        source/Buffer/UniformBuffer.cpp
        source/OpenGL/Shader/OpenGLShader.cpp
        source/Texture.cpp # Added
        source/OpenGL/OpenGLTexture.cpp # Added
    PUBLIC
        include/Pyramid/Graphics/GraphicsDevice.hpp
        include/Pyramid/Graphics/Buffer/VertexBuffer.hpp
        include/Pyramid/Graphics/Buffer/IndexBuffer.hpp
        include/Pyramid/Graphics/Buffer/VertexArray.hpp
        include/Pyramid/Graphics/Buffer/BufferLayout.hpp
        include/Pyramid/Graphics/Buffer/UniformBuffer.hpp # Added
        include/Pyramid/Graphics/Shader/Shader.hpp
        include/Pyramid/Graphics/Texture.hpp # Added
        include/Pyramid/Graphics/Geometry/Vertex.hpp
        include/Pyramid/Graphics/OpenGL/OpenGLDevice.hpp
        include/Pyramid/Graphics/OpenGL/Buffer/OpenGLIndexBuffer.hpp
        include/Pyramid/Graphics/OpenGL/Buffer/OpenGLVertexArray.hpp
        include/Pyramid/Graphics/OpenGL/Buffer/OpenGLVertexBuffer.hpp
        include/Pyramid/Graphics/OpenGL/Shader/OpenGLShader.hpp
        include/Pyramid/Graphics/OpenGL/OpenGLTexture.hpp # Added
)

target_include_directories(PyramidEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)
