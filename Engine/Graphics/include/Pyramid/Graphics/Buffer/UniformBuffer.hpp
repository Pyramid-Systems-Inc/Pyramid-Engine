#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <memory>

namespace Pyramid {

/**
 * @brief Uniform Buffer Object (UBO) interface for efficient uniform data management
 * 
 * Uniform Buffer Objects allow efficient sharing of uniform data between shaders
 * and reduce the number of individual uniform calls needed.
 */
class IUniformBuffer {
public:
    virtual ~IUniformBuffer() = default;

    /**
     * @brief Initialize the uniform buffer with a specific size
     * @param size Size of the buffer in bytes
     * @param usage Buffer usage pattern (static, dynamic, stream)
     * @return true if successful
     */
    virtual bool Initialize(size_t size, BufferUsage usage = BufferUsage::Dynamic) = 0;

    /**
     * @brief Update the buffer data
     * @param data Pointer to the data to upload
     * @param size Size of the data in bytes
     * @param offset Offset into the buffer (default: 0)
     */
    virtual void UpdateData(const void* data, size_t size, size_t offset = 0) = 0;

    /**
     * @brief Bind the uniform buffer to a specific binding point
     * @param bindingPoint The binding point index (0-based)
     */
    virtual void Bind(u32 bindingPoint) = 0;

    /**
     * @brief Unbind the uniform buffer
     */
    virtual void Unbind() = 0;

    /**
     * @brief Get the size of the buffer
     * @return Size in bytes
     */
    virtual size_t GetSize() const = 0;

    /**
     * @brief Get the buffer usage pattern
     * @return Buffer usage
     */
    virtual BufferUsage GetUsage() const = 0;

    /**
     * @brief Map the buffer for direct CPU access
     * @param access Access type (read, write, read-write)
     * @return Pointer to mapped memory, nullptr on failure
     */
    virtual void* Map(BufferAccess access = BufferAccess::WriteOnly) = 0;

    /**
     * @brief Unmap the buffer
     */
    virtual void Unmap() = 0;

    /**
     * @brief Check if the buffer is currently mapped
     * @return true if mapped
     */
    virtual bool IsMapped() const = 0;
};

/**
 * @brief Buffer usage patterns for optimization hints
 */
enum class BufferUsage {
    Static,     // Data is set once and used many times
    Dynamic,    // Data is changed frequently
    Stream      // Data is changed every frame
};

/**
 * @brief Buffer access patterns for mapping
 */
enum class BufferAccess {
    ReadOnly,   // CPU reads, GPU writes
    WriteOnly,  // CPU writes, GPU reads
    ReadWrite   // CPU and GPU both read and write
};

/**
 * @brief Uniform buffer layout helper for structured data
 * 
 * Helps manage std140 layout requirements for uniform buffers
 */
class UniformBufferLayout {
public:
    struct Element {
        std::string name;
        u32 offset;
        u32 size;
        u32 arraySize;
    };

    /**
     * @brief Add a uniform element to the layout
     * @param name Name of the uniform
     * @param size Size in bytes
     * @param arraySize Number of array elements (1 for non-arrays)
     */
    void AddElement(const std::string& name, u32 size, u32 arraySize = 1);

    /**
     * @brief Get the total size of the layout (with std140 alignment)
     * @return Total size in bytes
     */
    u32 GetSize() const { return m_size; }

    /**
     * @brief Get the offset of a specific element
     * @param name Name of the element
     * @return Offset in bytes, or UINT32_MAX if not found
     */
    u32 GetOffset(const std::string& name) const;

    /**
     * @brief Get all elements in the layout
     * @return Vector of layout elements
     */
    const std::vector<Element>& GetElements() const { return m_elements; }

private:
    std::vector<Element> m_elements;
    u32 m_size = 0;

    /**
     * @brief Calculate std140 aligned offset
     * @param size Size of the element
     * @param currentOffset Current buffer offset
     * @return Aligned offset
     */
    u32 CalculateAlignedOffset(u32 size, u32 currentOffset) const;
};

/**
 * @brief Typed uniform buffer wrapper for easier usage
 * 
 * Template wrapper that provides type-safe access to uniform buffers
 */
template<typename T>
class TypedUniformBuffer {
public:
    explicit TypedUniformBuffer(std::shared_ptr<IUniformBuffer> buffer)
        : m_buffer(buffer) {}

    /**
     * @brief Initialize with the size of type T
     * @param usage Buffer usage pattern
     * @return true if successful
     */
    bool Initialize(BufferUsage usage = BufferUsage::Dynamic) {
        return m_buffer->Initialize(sizeof(T), usage);
    }

    /**
     * @brief Update the buffer with typed data
     * @param data Reference to the data structure
     */
    void UpdateData(const T& data) {
        m_buffer->UpdateData(&data, sizeof(T), 0);
    }

    /**
     * @brief Bind to a specific binding point
     * @param bindingPoint The binding point index
     */
    void Bind(u32 bindingPoint) {
        m_buffer->Bind(bindingPoint);
    }

    /**
     * @brief Unbind the buffer
     */
    void Unbind() {
        m_buffer->Unbind();
    }

    /**
     * @brief Map the buffer for direct access
     * @param access Access type
     * @return Pointer to mapped data as type T
     */
    T* Map(BufferAccess access = BufferAccess::WriteOnly) {
        return static_cast<T*>(m_buffer->Map(access));
    }

    /**
     * @brief Unmap the buffer
     */
    void Unmap() {
        m_buffer->Unmap();
    }

    /**
     * @brief Get the underlying buffer
     * @return Shared pointer to the uniform buffer
     */
    std::shared_ptr<IUniformBuffer> GetBuffer() const {
        return m_buffer;
    }

private:
    std::shared_ptr<IUniformBuffer> m_buffer;
};

} // namespace Pyramid
