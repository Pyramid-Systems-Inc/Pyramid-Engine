#pragma once
#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp> // For BufferUsage and BufferAccess

namespace Pyramid
{

    /**
     * @brief Interface for Shader Storage Buffer Object (SSBO) implementations
     * SSBOs provide read/write access to large amounts of data from shaders
     */
    class IShaderStorageBuffer
    {
    public:
        virtual ~IShaderStorageBuffer() = default;

        /**
         * @brief Initialize the SSBO with a specific size
         * @param size Size of the buffer in bytes
         * @param usage Buffer usage pattern (static, dynamic, stream)
         * @return true if initialization was successful
         */
        virtual bool Initialize(size_t size, BufferUsage usage) = 0;

        /**
         * @brief Bind the SSBO to a specific binding point
         * @param bindingPoint The binding point index
         */
        virtual void Bind(u32 bindingPoint) = 0;

        /**
         * @brief Unbind the SSBO from its current binding point
         */
        virtual void Unbind() = 0;

        /**
         * @brief Upload data to the buffer
         * @param data Pointer to the data
         * @param size Size of the data in bytes
         * @param offset Offset in bytes from the start of the buffer
         */
        virtual void SetData(const void *data, size_t size, size_t offset = 0) = 0;

        /**
         * @brief Read data from the buffer
         * @param data Pointer to the destination buffer
         * @param size Size of the data to read in bytes
         * @param offset Offset in bytes from the start of the buffer
         */
        virtual void GetData(void *data, size_t size, size_t offset = 0) = 0;

        /**
         * @brief Map the buffer for direct CPU access
         * @param access Access mode (read, write, read-write)
         * @return Pointer to the mapped memory, or nullptr on failure
         */
        virtual void *Map(BufferAccess access) = 0;

        /**
         * @brief Unmap the buffer
         */
        virtual void Unmap() = 0;

        /**
         * @brief Get the size of the buffer in bytes
         * @return Size in bytes
         */
        virtual size_t GetSize() const = 0;

        /**
         * @brief Get the current binding point
         * @return Binding point index, or -1 if not bound
         */
        virtual i32 GetBindingPoint() const = 0;
    };

} // namespace Pyramid
