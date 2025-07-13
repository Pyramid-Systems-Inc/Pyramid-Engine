#pragma once
#include <Pyramid/Graphics/Buffer/InstanceBuffer.hpp>
#include <glad/glad.h>

namespace Pyramid
{

    /**
     * @brief OpenGL implementation of instance buffer
     */
    class OpenGLInstanceBuffer : public IInstanceBuffer
    {
    public:
        OpenGLInstanceBuffer();
        virtual ~OpenGLInstanceBuffer();

        void Bind() override;
        void Unbind() override;
        void SetData(const void* data, u32 size, u32 instanceCount) override;
        void UpdateData(const void* data, u32 size, u32 offset = 0) override;
        
        u32 GetInstanceCount() const override { return m_instanceCount; }
        u32 GetInstanceSize() const override { return m_instanceSize; }

        /**
         * @brief Get the OpenGL buffer ID
         * @return GLuint buffer ID
         */
        GLuint GetBufferID() const { return m_bufferId; }

    private:
        GLuint m_bufferId;
        u32 m_instanceCount;
        u32 m_instanceSize;
        u32 m_bufferSize;
    };

} // namespace Pyramid
