#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLVertexBuffer.hpp>

namespace Pyramid {

OpenGLVertexBuffer::OpenGLVertexBuffer()
    : m_bufferId(0)
{
    glGenBuffers(1, &m_bufferId);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
    if (m_bufferId)
        glDeleteBuffers(1, &m_bufferId);
}

void OpenGLVertexBuffer::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferId);
}

void OpenGLVertexBuffer::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLVertexBuffer::SetData(const void* data, u32 size)
{
    Bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

} // namespace Pyramid
