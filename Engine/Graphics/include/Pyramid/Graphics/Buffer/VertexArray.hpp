// Engine/Graphics/include/Pyramid/Graphics/Buffer/VertexArray.hpp
#pragma once
#include "Pyramid/Core/Prerequisites.hpp" // Changed from Core/include/Pyramid/Core/Prerequisites.hpp
#include <memory>

namespace Pyramid
{
    class IVertexBuffer;
    class IIndexBuffer;

    class IVertexArray
    {
    public:
        virtual ~IVertexArray() = default;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        virtual void AddVertexBuffer(const std::shared_ptr<IVertexBuffer> &vertexBuffer) = 0;
        virtual void SetIndexBuffer(const std::shared_ptr<IIndexBuffer> &indexBuffer) = 0;
        virtual const std::shared_ptr<IIndexBuffer> &GetIndexBuffer() const = 0;
    };
}