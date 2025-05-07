// Engine/Graphics/include/Pyramid/Graphics/Buffer/VertexArray.hpp
// Engine/Graphics/include/Pyramid/Graphics/Buffer/VertexArray.hpp
#pragma once
#include "Pyramid/Core/Prerequisites.hpp"
#include "Pyramid/Graphics/Buffer/BufferLayout.hpp" // Added
#include <memory>
#include <vector> // For GetVertexBuffers (if added later)
#include "Pyramid/Graphics/Buffer/IndexBuffer.hpp" // Added full include

namespace Pyramid
{
    class IVertexBuffer; // Still okay to forward declare if only used in pointers/references by IVertexArray itself
    // class IIndexBuffer; // Removed forward declaration

    class IVertexArray
    {
    public:
        virtual ~IVertexArray() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void AddVertexBuffer(const std::shared_ptr<IVertexBuffer>& vertexBuffer, const BufferLayout& layout) = 0; // Changed
        // virtual const std::vector<std::shared_ptr<IVertexBuffer>>& GetVertexBuffers() const = 0; // Optional future addition

        virtual void SetIndexBuffer(const std::shared_ptr<IIndexBuffer>& indexBuffer) = 0;
        virtual const std::shared_ptr<IIndexBuffer>& GetIndexBuffer() const = 0;
    };
}
