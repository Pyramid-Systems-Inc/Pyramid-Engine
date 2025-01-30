#pragma once
#include <Pyramid/Graphics/Buffer/VertexBuffer.hpp>
#include <glad/glad.h>

namespace Pyramid {

/**
 * @brief OpenGL implementation of vertex buffer
 */
class OpenGLVertexBuffer : public IVertexBuffer {
public:
    OpenGLVertexBuffer();
    virtual ~OpenGLVertexBuffer();

    void Bind() override;
    void Unbind() override;
    void SetData(const void* data, u32 size) override;

private:
    GLuint m_bufferId;
};

} // namespace Pyramid
