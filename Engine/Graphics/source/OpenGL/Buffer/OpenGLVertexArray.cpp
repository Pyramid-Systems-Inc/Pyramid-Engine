// Engine/Graphics/source/OpenGL/Buffer/OpenGLVertexArray.cpp
#include "Pyramid/Graphics/OpenGL/Buffer/OpenGLVertexArray.hpp"
#include "Pyramid/Graphics/Buffer/VertexBuffer.hpp"
#include "Pyramid/Graphics/Buffer/IndexBuffer.hpp"
#include "Pyramid/Graphics/Buffer/BufferLayout.hpp" // Added
#include <glad/glad.h>

namespace Pyramid
{

    // Helper function to convert ShaderDataType to OpenGL base type
    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Float:    return GL_FLOAT;
            case ShaderDataType::Float2:   return GL_FLOAT;
            case ShaderDataType::Float3:   return GL_FLOAT;
            case ShaderDataType::Float4:   return GL_FLOAT;
            case ShaderDataType::Mat3:     return GL_FLOAT;
            case ShaderDataType::Mat4:     return GL_FLOAT;
            case ShaderDataType::Int:      return GL_INT;
            case ShaderDataType::Int2:     return GL_INT;
            case ShaderDataType::Int3:     return GL_INT;
            case ShaderDataType::Int4:     return GL_INT;
            case ShaderDataType::Bool:     return GL_BOOL;
            default: break;
        }
        // PYRAMID_CORE_ASSERT(false, "Unknown ShaderDataType!");
        return 0;
    }


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

    void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<IVertexBuffer>& vertexBuffer, const BufferLayout& layout)
    {
        if (layout.GetElements().empty())
        {
            // PYRAMID_CORE_WARN("Vertex Buffer has no layout!");
            return;
        }

        glBindVertexArray(m_rendererID);
        vertexBuffer->Bind();

        u32 attributeIndex = 0; // Assuming attribute indices start from 0 and are contiguous for this VBO
                               // More complex scenarios might need explicit location binding in shaders.
        for (const auto& element : layout)
        {
            glEnableVertexAttribArray(attributeIndex);
            glVertexAttribPointer(
                attributeIndex,
                ShaderDataTypeComponentCount(element.Type),
                ShaderDataTypeToOpenGLBaseType(element.Type),
                element.Normalized ? GL_TRUE : GL_FALSE,
                layout.GetStride(),
                (const void*)(intptr_t)element.Offset // Cast to intptr_t then to const void* for offset
            );
            attributeIndex++;
        }
        m_vertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IIndexBuffer>& indexBuffer)
    {
        glBindVertexArray(m_rendererID);
        indexBuffer->Bind();
        m_indexBuffer = indexBuffer;
    }

} // namespace Pyramid
