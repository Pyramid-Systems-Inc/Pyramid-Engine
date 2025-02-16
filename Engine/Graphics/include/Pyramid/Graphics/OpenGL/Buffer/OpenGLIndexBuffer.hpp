// Engine/Graphics/include/Pyramid/Graphics/OpenGL/Buffer/OpenGLIndexBuffer.hpp
#pragma once
#include <Pyramid/Graphics/Buffer/IndexBuffer.hpp>
#include <glad/glad.h>

namespace Pyramid
{

    class OpenGLIndexBuffer : public IIndexBuffer
    {
    public:
        OpenGLIndexBuffer();
        ~OpenGLIndexBuffer() override;

        void Bind() override;
        void Unbind() override;
        void SetData(const void *data, u32 count) override;
        u32 GetCount() const override { return m_count; }

    private:
        GLuint m_bufferId;
        u32 m_count;
    };

} // namespace Pyramid