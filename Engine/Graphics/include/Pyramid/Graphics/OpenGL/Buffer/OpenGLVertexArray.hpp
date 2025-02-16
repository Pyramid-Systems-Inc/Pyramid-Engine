// Engine/Graphics/include/Pyramid/Graphics/OpenGL/Buffer/OpenGLVertexArray.hpp
#pragma once
#include "Pyramid/Graphics/Buffer/VertexArray.hpp"
#include <glad/glad.h>
#include <vector>

namespace Pyramid
{

    class OpenGLVertexArray : public IVertexArray
    {
    public:
        OpenGLVertexArray();
        ~OpenGLVertexArray() override;

        void Bind() const override;
        void Unbind() const override;
        void AddVertexBuffer(const std::shared_ptr<IVertexBuffer> &vertexBuffer) override;
        void SetIndexBuffer(const std::shared_ptr<IIndexBuffer> &indexBuffer) override;
        const std::shared_ptr<IIndexBuffer> &GetIndexBuffer() const override { return m_indexBuffer; }

    private:
        GLuint m_rendererID;
        std::vector<std::shared_ptr<IVertexBuffer>> m_vertexBuffers;
        std::shared_ptr<IIndexBuffer> m_indexBuffer;
    };

} // namespace Pyramid