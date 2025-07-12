#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLUniformBuffer.hpp>
#include <Pyramid/Util/Log.hpp>

namespace Pyramid {

OpenGLUniformBuffer::OpenGLUniformBuffer()
    : m_bufferID(0)
    , m_size(0)
    , m_usage(BufferUsage::Dynamic)
    , m_mapped(false)
    , m_currentBindingPoint(0)
{
}

OpenGLUniformBuffer::~OpenGLUniformBuffer()
{
    if (m_bufferID != 0)
    {
        if (m_mapped)
        {
            glUnmapBuffer(GL_UNIFORM_BUFFER);
        }
        glDeleteBuffers(1, &m_bufferID);
    }
}

bool OpenGLUniformBuffer::Initialize(size_t size, BufferUsage usage)
{
    if (m_bufferID != 0)
    {
        PYRAMID_LOG_ERROR("Uniform buffer already initialized");
        return false;
    }

    if (size == 0)
    {
        PYRAMID_LOG_ERROR("Uniform buffer size cannot be zero");
        return false;
    }

    m_size = size;
    m_usage = usage;

    // Generate buffer
    glGenBuffers(1, &m_bufferID);
    if (m_bufferID == 0)
    {
        PYRAMID_LOG_ERROR("Failed to generate OpenGL uniform buffer");
        return false;
    }

    // Bind and allocate storage
    glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, BufferUsageToGL(usage));

    // Check for errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        PYRAMID_LOG_ERROR("Failed to allocate uniform buffer storage. OpenGL error: ", error);
        glDeleteBuffers(1, &m_bufferID);
        m_bufferID = 0;
        return false;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    PYRAMID_LOG_INFO("Created uniform buffer with size: ", size, " bytes");
    return true;
}

void OpenGLUniformBuffer::UpdateData(const void* data, size_t size, size_t offset)
{
    if (m_bufferID == 0)
    {
        PYRAMID_LOG_ERROR("Uniform buffer not initialized");
        return;
    }

    if (data == nullptr)
    {
        PYRAMID_LOG_ERROR("Data pointer is null");
        return;
    }

    if (offset + size > m_size)
    {
        PYRAMID_LOG_ERROR("Data size exceeds buffer bounds. Offset: ", offset, ", Size: ", size, ", Buffer size: ", m_size);
        return;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLUniformBuffer::Bind(u32 bindingPoint)
{
    if (m_bufferID == 0)
    {
        PYRAMID_LOG_ERROR("Uniform buffer not initialized");
        return;
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_bufferID);
    m_currentBindingPoint = bindingPoint;
}

void OpenGLUniformBuffer::Unbind()
{
    if (m_bufferID == 0)
    {
        return;
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, m_currentBindingPoint, 0);
}

void* OpenGLUniformBuffer::Map(BufferAccess access)
{
    if (m_bufferID == 0)
    {
        PYRAMID_LOG_ERROR("Uniform buffer not initialized");
        return nullptr;
    }

    if (m_mapped)
    {
        PYRAMID_LOG_ERROR("Buffer is already mapped");
        return nullptr;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);
    void* ptr = glMapBuffer(GL_UNIFORM_BUFFER, BufferAccessToGL(access));
    
    if (ptr != nullptr)
    {
        m_mapped = true;
    }
    else
    {
        PYRAMID_LOG_ERROR("Failed to map uniform buffer");
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    return ptr;
}

void OpenGLUniformBuffer::Unmap()
{
    if (m_bufferID == 0 || !m_mapped)
    {
        return;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_bufferID);
    GLboolean result = glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    if (result == GL_FALSE)
    {
        PYRAMID_LOG_WARN("Uniform buffer data may have been corrupted during unmapping");
    }

    m_mapped = false;
}

GLenum OpenGLUniformBuffer::BufferUsageToGL(BufferUsage usage) const
{
    switch (usage)
    {
    case BufferUsage::Static:
        return GL_STATIC_DRAW;
    case BufferUsage::Dynamic:
        return GL_DYNAMIC_DRAW;
    case BufferUsage::Stream:
        return GL_STREAM_DRAW;
    default:
        return GL_DYNAMIC_DRAW;
    }
}

GLenum OpenGLUniformBuffer::BufferAccessToGL(BufferAccess access) const
{
    switch (access)
    {
    case BufferAccess::ReadOnly:
        return GL_READ_ONLY;
    case BufferAccess::WriteOnly:
        return GL_WRITE_ONLY;
    case BufferAccess::ReadWrite:
        return GL_READ_WRITE;
    default:
        return GL_WRITE_ONLY;
    }
}

} // namespace Pyramid
