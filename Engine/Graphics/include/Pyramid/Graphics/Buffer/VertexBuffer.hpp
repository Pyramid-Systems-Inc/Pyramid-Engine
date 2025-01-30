#pragma once
#include <Pyramid/Core/Prerequisites.hpp>

namespace Pyramid {

/**
 * @brief Interface for vertex buffer implementations
 */
class IVertexBuffer {
public:
    virtual ~IVertexBuffer() = default;

    /**
     * @brief Bind the vertex buffer for rendering
     */
    virtual void Bind() = 0;

    /**
     * @brief Unbind the vertex buffer
     */
    virtual void Unbind() = 0;

    /**
     * @brief Upload vertex data to the buffer
     * @param data Pointer to vertex data
     * @param size Size of data in bytes
     */
    virtual void SetData(const void* data, u32 size) = 0;
};

} // namespace Pyramid
