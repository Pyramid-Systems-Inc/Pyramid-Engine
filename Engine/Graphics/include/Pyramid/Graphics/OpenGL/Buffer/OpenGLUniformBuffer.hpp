#pragma once

#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <glad/glad.h>

namespace Pyramid {

/**
 * @brief OpenGL implementation of Uniform Buffer Object (UBO)
 */
class OpenGLUniformBuffer : public IUniformBuffer {
public:
    OpenGLUniformBuffer();
    ~OpenGLUniformBuffer() override;

    // IUniformBuffer interface
    bool Initialize(size_t size, BufferUsage usage = BufferUsage::Dynamic) override;
    void UpdateData(const void* data, size_t size, size_t offset = 0) override;
    void Bind(u32 bindingPoint) override;
    void Unbind() override;
    size_t GetSize() const override { return m_size; }
    BufferUsage GetUsage() const override { return m_usage; }
    void* Map(BufferAccess access = BufferAccess::WriteOnly) override;
    void Unmap() override;
    bool IsMapped() const override { return m_mapped; }

    /**
     * @brief Get the OpenGL buffer ID
     * @return OpenGL buffer object ID
     */
    GLuint GetBufferID() const { return m_bufferID; }

private:
    GLuint m_bufferID;
    size_t m_size;
    BufferUsage m_usage;
    bool m_mapped;
    u32 m_currentBindingPoint;

    /**
     * @brief Convert engine buffer usage to OpenGL usage
     * @param usage Engine buffer usage
     * @return OpenGL usage enum
     */
    GLenum BufferUsageToGL(BufferUsage usage) const;

    /**
     * @brief Convert engine buffer access to OpenGL access
     * @param access Engine buffer access
     * @return OpenGL access enum
     */
    GLenum BufferAccessToGL(BufferAccess access) const;
};

} // namespace Pyramid
