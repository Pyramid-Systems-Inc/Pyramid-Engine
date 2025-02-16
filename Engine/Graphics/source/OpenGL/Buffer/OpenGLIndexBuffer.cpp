// Engine/Graphics/source/OpenGL/Buffer/OpenGLIndexBuffer.cpp
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLIndexBuffer.hpp>

namespace Pyramid
{

    OpenGLIndexBuffer::OpenGLIndexBuffer()
        : m_bufferId(0), m_count(0)
    {
        glGenBuffers(1, &m_bufferId);
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer()
    {
        if (m_bufferId)
            glDeleteBuffers(1, &m_bufferId);
    }

    void OpenGLIndexBuffer::Bind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferId);
    }

    void OpenGLIndexBuffer::Unbind()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void OpenGLIndexBuffer::SetData(const void *data, u32 count)
    {
        m_count = count;
        Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), data, GL_STATIC_DRAW);
    }

} // namespace Pyramid