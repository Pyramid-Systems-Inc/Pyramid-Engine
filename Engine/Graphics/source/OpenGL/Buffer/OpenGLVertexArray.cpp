// Engine/Graphics/source/OpenGL/Buffer/OpenGLVertexArray.cpp
#include "Pyramid/Graphics/OpenGL/Buffer/OpenGLVertexArray.hpp"
#include "Pyramid/Graphics/Buffer/VertexBuffer.hpp"
#include "Pyramid/Graphics/Buffer/IndexBuffer.hpp"
#include <glad/glad.h>

namespace Pyramid
{

    OpenGLVertexArray::OpenGLVertexArray()
        : m_rendererID(0)
    {
        glGenVertexArrays(1, &m_rendererID);
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        if (m_rendererID)
            glDeleteVertexArrays(1, &m_rendererID);
    }

    void OpenGLVertexArray::Bind() const
    {
        glBindVertexArray(m_rendererID);
    }

    void OpenGLVertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<IVertexBuffer> &vertexBuffer)
    {
        glBindVertexArray(m_rendererID);
        vertexBuffer->Bind();

        // Set vertex attributes based on layout
        // TODO: Add vertex layout information
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

        m_vertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IIndexBuffer> &indexBuffer)
    {
        glBindVertexArray(m_rendererID);
        indexBuffer->Bind();
        m_indexBuffer = indexBuffer;
    }

} // namespace Pyramid