#pragma once
#include <Pyramid/Core/Prerequisites.hpp>

namespace Pyramid
{

    /**
     * @brief Interface for instance buffer implementations
     * Instance buffers store per-instance data for instanced rendering
     */
    class IInstanceBuffer
    {
    public:
        virtual ~IInstanceBuffer() = default;

        /**
         * @brief Bind the instance buffer for rendering
         */
        virtual void Bind() = 0;

        /**
         * @brief Unbind the instance buffer
         */
        virtual void Unbind() = 0;

        /**
         * @brief Upload instance data to the buffer
         * @param data Pointer to instance data
         * @param size Size of data in bytes
         * @param instanceCount Number of instances
         */
        virtual void SetData(const void* data, u32 size, u32 instanceCount) = 0;

        /**
         * @brief Update a portion of the instance buffer
         * @param data Pointer to instance data
         * @param size Size of data in bytes
         * @param offset Offset in bytes from the start of the buffer
         */
        virtual void UpdateData(const void* data, u32 size, u32 offset = 0) = 0;

        /**
         * @brief Get the number of instances in the buffer
         * @return Number of instances
         */
        virtual u32 GetInstanceCount() const = 0;

        /**
         * @brief Get the size of each instance in bytes
         * @return Size per instance in bytes
         */
        virtual u32 GetInstanceSize() const = 0;
    };

} // namespace Pyramid
