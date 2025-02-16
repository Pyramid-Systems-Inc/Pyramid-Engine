// Engine/Graphics/include/Pyramid/Graphics/Buffer/IndexBuffer.hpp
#pragma once
#include <Pyramid/Core/Prerequisites.hpp>

namespace Pyramid
{

    class IIndexBuffer
    {
    public:
        virtual ~IIndexBuffer() = default;
        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual void SetData(const void *data, u32 count) = 0;
        virtual u32 GetCount() const = 0;
    };

} // namespace Pyramid