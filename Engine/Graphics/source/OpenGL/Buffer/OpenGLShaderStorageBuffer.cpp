#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLShaderStorageBuffer.hpp>
#include <Pyramid/Util/Log.hpp>

namespace Pyramid
{

    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer()
        : m_bufferID(0), m_size(0), m_usage(BufferUsage::Dynamic), 
          m_currentBindingPoint(-1), m_initialized(false)
    {
    }

    OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
    {
        if (m_bufferID != 0)
        {
            glDeleteBuffers(1, &m_bufferID);
            PYRAMID_LOG_DEBUG("Deleted OpenGL SSBO with ID: ", m_bufferID);
        }
    }

    bool OpenGLShaderStorageBuffer::Initialize(size_t size, BufferUsage usage)
    {
        if (m_initialized)
        {
            PYRAMID_LOG_WARN("SSBO already initialized");
            return true;
        }

        if (size == 0)
        {
            PYRAMID_LOG_ERROR("SSBO size cannot be zero");
            return false;
        }

        m_size = size;
        m_usage = usage;

        glGenBuffers(1, &m_bufferID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, BufferUsageToGL(usage));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        m_initialized = true;
        PYRAMID_LOG_DEBUG("Initialized OpenGL SSBO with ID: ", m_bufferID, ", size: ", size, " bytes");
        return true;
    }

    void OpenGLShaderStorageBuffer::Bind(u32 bindingPoint)
    {
        if (!m_initialized)
        {
            PYRAMID_LOG_ERROR("Cannot bind uninitialized SSBO");
            return;
        }

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, m_bufferID);
        m_currentBindingPoint = static_cast<i32>(bindingPoint);
        PYRAMID_LOG_DEBUG("Bound SSBO ", m_bufferID, " to binding point ", bindingPoint);
    }

    void OpenGLShaderStorageBuffer::Unbind()
    {
        if (m_currentBindingPoint >= 0)
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_currentBindingPoint, 0);
            m_currentBindingPoint = -1;
        }
    }

    void OpenGLShaderStorageBuffer::SetData(const void* data, size_t size, size_t offset)
    {
        if (!m_initialized)
        {
            PYRAMID_LOG_ERROR("Cannot set data on uninitialized SSBO");
            return;
        }

        if (offset + size > m_size)
        {
            PYRAMID_LOG_ERROR("Data exceeds buffer size. Offset: ", offset, ", Size: ", size, ", Buffer size: ", m_size);
            return;
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        PYRAMID_LOG_DEBUG("Updated SSBO data: ", size, " bytes at offset ", offset);
    }

    void OpenGLShaderStorageBuffer::GetData(void* data, size_t size, size_t offset)
    {
        if (!m_initialized)
        {
            PYRAMID_LOG_ERROR("Cannot get data from uninitialized SSBO");
            return;
        }

        if (offset + size > m_size)
        {
            PYRAMID_LOG_ERROR("Read exceeds buffer size. Offset: ", offset, ", Size: ", size, ", Buffer size: ", m_size);
            return;
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferID);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        PYRAMID_LOG_DEBUG("Read SSBO data: ", size, " bytes from offset ", offset);
    }

    void* OpenGLShaderStorageBuffer::Map(BufferAccess access)
    {
        if (!m_initialized)
        {
            PYRAMID_LOG_ERROR("Cannot map uninitialized SSBO");
            return nullptr;
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferID);
        void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, BufferAccessToGL(access));
        
        if (!ptr)
        {
            PYRAMID_LOG_ERROR("Failed to map SSBO");
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            return nullptr;
        }

        PYRAMID_LOG_DEBUG("Mapped SSBO for access");
        return ptr;
    }

    void OpenGLShaderStorageBuffer::Unmap()
    {
        if (!m_initialized)
        {
            PYRAMID_LOG_ERROR("Cannot unmap uninitialized SSBO");
            return;
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferID);
        GLboolean result = glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        if (result == GL_FALSE)
        {
            PYRAMID_LOG_WARN("SSBO data may have been corrupted during mapping");
        }
        else
        {
            PYRAMID_LOG_DEBUG("Unmapped SSBO successfully");
        }
    }

    GLenum OpenGLShaderStorageBuffer::BufferUsageToGL(BufferUsage usage)
    {
        switch (usage)
        {
        case BufferUsage::Static:  return GL_STATIC_DRAW;
        case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
        case BufferUsage::Stream:  return GL_STREAM_DRAW;
        default:                   return GL_DYNAMIC_DRAW;
        }
    }

    GLenum OpenGLShaderStorageBuffer::BufferAccessToGL(BufferAccess access)
    {
        switch (access)
        {
        case BufferAccess::ReadOnly:  return GL_READ_ONLY;
        case BufferAccess::WriteOnly: return GL_WRITE_ONLY;
        case BufferAccess::ReadWrite: return GL_READ_WRITE;
        default:                      return GL_READ_WRITE;
        }
    }

} // namespace Pyramid
