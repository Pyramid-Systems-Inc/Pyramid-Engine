#pragma once

#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <glad/glad.h>
#include <unordered_map>
#include <memory>

namespace Pyramid
{

    /**
     * @brief Enhanced UBO state management for binding optimization
     */
    class UBOStateManager
    {
    public:
        static UBOStateManager &GetInstance();

        void BindBuffer(u32 bindingPoint, GLuint bufferID);
        void UnbindBuffer(u32 bindingPoint);
        bool IsBufferBound(u32 bindingPoint, GLuint bufferID) const;
        void ClearCache();

    private:
        UBOStateManager() = default;
        std::unordered_map<u32, GLuint> m_boundBuffers;
    };

    /**
     * @brief OpenGL implementation of Uniform Buffer Object (UBO)
     */
    class OpenGLUniformBuffer : public IUniformBuffer
    {
    public:
        OpenGLUniformBuffer();
        ~OpenGLUniformBuffer() override;

        // IUniformBuffer interface
        bool Initialize(size_t size, BufferUsage usage = BufferUsage::Dynamic) override;
        void UpdateData(const void *data, size_t size, size_t offset = 0) override;
        void Bind(u32 bindingPoint) override;
        void Unbind() override;
        size_t GetSize() const override { return m_size; }
        BufferUsage GetUsage() const override { return m_usage; }
        void *Map(BufferAccess access = BufferAccess::WriteOnly) override;
        void Unmap() override;
        bool IsMapped() const override { return m_mapped; }

        // Enhanced UBO features
        void UpdateDataRange(const void *data, size_t offset, size_t size);
        void ClearData(size_t offset = 0, size_t size = 0);
        bool Resize(size_t newSize);
        void BindRange(u32 bindingPoint, size_t offset, size_t size);
        void InvalidateData();

        // Performance optimization features
        void SetPersistentMapping(bool enable);
        bool IsPersistentMapped() const { return m_persistentMapped; }
        void FlushMappedRange(size_t offset, size_t size);

        // State management
        u32 GetCurrentBindingPoint() const { return m_currentBindingPoint; }
        bool IsBoundToPoint(u32 bindingPoint) const;

        /**
         * @brief Get the OpenGL buffer ID
         * @return OpenGL buffer object ID
         */
        GLuint GetBufferID() const { return m_bufferID; }

    private:
        GLuint m_bufferID;
        size_t m_size;
        BufferUsage m_usage;
        bool m_mapped;
        u32 m_currentBindingPoint;

        // Enhanced features
        bool m_persistentMapped;
        void *m_persistentPtr;
        bool m_coherentMapping;
        size_t m_alignment;

        /**
         * @brief Convert engine buffer usage to OpenGL usage
         * @param usage Engine buffer usage
         * @return OpenGL usage enum
         */
        GLenum BufferUsageToGL(BufferUsage usage) const;

        /**
         * @brief Convert engine buffer access to OpenGL access
         * @param access Engine buffer access
         * @return OpenGL access enum
         */
        GLenum BufferAccessToGL(BufferAccess access) const;
    };

} // namespace Pyramid
