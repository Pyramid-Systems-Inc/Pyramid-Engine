#pragma once
#include <Pyramid/Graphics/Buffer/ShaderStorageBuffer.hpp>
#include <glad/glad.h>

namespace Pyramid
{

    /**
     * @brief OpenGL implementation of Shader Storage Buffer Object (SSBO)
     */
    class OpenGLShaderStorageBuffer : public IShaderStorageBuffer
    {
    public:
        OpenGLShaderStorageBuffer();
        virtual ~OpenGLShaderStorageBuffer();

        bool Initialize(size_t size, BufferUsage usage) override;
        void Bind(u32 bindingPoint) override;
        void Unbind() override;
        void SetData(const void* data, size_t size, size_t offset = 0) override;
        void GetData(void* data, size_t size, size_t offset = 0) override;
        void* Map(BufferAccess access) override;
        void Unmap() override;
        
        size_t GetSize() const override { return m_size; }
        i32 GetBindingPoint() const override { return m_currentBindingPoint; }

        /**
         * @brief Get the OpenGL buffer ID
         * @return GLuint buffer ID
         */
        GLuint GetBufferID() const { return m_bufferID; }

    private:
        GLuint m_bufferID;
        size_t m_size;
        BufferUsage m_usage;
        i32 m_currentBindingPoint;
        bool m_initialized;
        
        GLenum BufferUsageToGL(BufferUsage usage);
        GLenum BufferAccessToGL(BufferAccess access);
    };

} // namespace Pyramid
