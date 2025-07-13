#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <glad/glad.h>
#include <vector>
#include <memory>
#include <string>

namespace Pyramid
{

    /**
     * @brief Framebuffer attachment types
     */
    enum class FramebufferAttachmentType
    {
        Color = 0,
        Depth,
        Stencil,
        DepthStencil
    };

    /**
     * @brief Framebuffer attachment specification
     */
    struct FramebufferAttachmentSpec
    {
        FramebufferAttachmentType type = FramebufferAttachmentType::Color;
        GLenum internalFormat = GL_RGBA8;
        GLenum format = GL_RGBA;
        GLenum dataType = GL_UNSIGNED_BYTE;
        bool multisampled = false;
        u32 samples = 1;

        // For color attachments
        u32 colorAttachmentIndex = 0;

        // Texture parameters
        GLenum minFilter = GL_LINEAR;
        GLenum magFilter = GL_LINEAR;
        GLenum wrapS = GL_CLAMP_TO_EDGE;
        GLenum wrapT = GL_CLAMP_TO_EDGE;
    };

    /**
     * @brief Framebuffer specification
     */
    struct FramebufferSpec
    {
        u32 width = 1920;
        u32 height = 1080;
        std::vector<FramebufferAttachmentSpec> attachments;
        bool swapChainTarget = false; // Is this the main window framebuffer?
        u32 samples = 1;              // MSAA samples
    };

    /**
     * @brief OpenGL Framebuffer Object implementation
     */
    class OpenGLFramebuffer
    {
    public:
        OpenGLFramebuffer(const FramebufferSpec &spec);
        ~OpenGLFramebuffer();

        // Core functionality
        bool Initialize();
        void Bind() const;
        void Unbind() const;
        void Clear(f32 r = 0.0f, f32 g = 0.0f, f32 b = 0.0f, f32 a = 1.0f) const;
        void ClearAttachment(u32 attachmentIndex, const void *value) const;

        // Resize functionality
        bool Resize(u32 width, u32 height);
        void Invalidate(); // Recreate framebuffer

        // Texture access
        GLuint GetColorAttachmentTexture(u32 index = 0) const;
        GLuint GetDepthAttachmentTexture() const;
        GLuint GetStencilAttachmentTexture() const;

        // Multiple render targets
        void SetDrawBuffers(const std::vector<u32> &colorAttachments) const;
        void SetReadBuffer(u32 colorAttachment) const;

        // Blitting operations
        void BlitTo(const OpenGLFramebuffer &target,
                    u32 srcX0, u32 srcY0, u32 srcX1, u32 srcY1,
                    u32 dstX0, u32 dstY0, u32 dstX1, u32 dstY1,
                    GLbitfield mask = GL_COLOR_BUFFER_BIT,
                    GLenum filter = GL_LINEAR) const;

        // State queries
        bool IsComplete() const;
        bool IsMultisampled() const { return m_spec.samples > 1; }
        u32 GetColorAttachmentCount() const;

        // Accessors
        const FramebufferSpec &GetSpecification() const { return m_spec; }
        u32 GetWidth() const { return m_spec.width; }
        u32 GetHeight() const { return m_spec.height; }
        GLuint GetFramebufferID() const { return m_framebufferID; }

        // Utility methods
        void SaveColorAttachmentToFile(u32 attachmentIndex, const std::string &filepath) const;
        std::vector<u8> ReadColorAttachmentPixels(u32 attachmentIndex) const;

    private:
        void CreateAttachments();
        void CreateColorAttachment(const FramebufferAttachmentSpec &spec);
        void CreateDepthAttachment(const FramebufferAttachmentSpec &spec);
        void CreateStencilAttachment(const FramebufferAttachmentSpec &spec);
        void CreateDepthStencilAttachment(const FramebufferAttachmentSpec &spec);

        GLuint CreateTexture2D(u32 width, u32 height, GLenum internalFormat,
                               GLenum format, GLenum type, bool multisampled, u32 samples);
        GLuint CreateRenderbuffer(u32 width, u32 height, GLenum internalFormat,
                                  bool multisampled, u32 samples);

        void AttachTexture2D(GLuint texture, GLenum attachment, u32 mipLevel = 0);
        void AttachRenderbuffer(GLuint renderbuffer, GLenum attachment);

        static GLenum GetColorAttachmentEnum(u32 index);
        static std::string GetFramebufferStatusString(GLenum status);

    private:
        FramebufferSpec m_spec;
        GLuint m_framebufferID = 0;

        // Attachment storage
        std::vector<GLuint> m_colorAttachmentTextures;
        std::vector<GLuint> m_colorAttachmentRenderbuffers;
        GLuint m_depthAttachmentTexture = 0;
        GLuint m_depthAttachmentRenderbuffer = 0;
        GLuint m_stencilAttachmentTexture = 0;
        GLuint m_stencilAttachmentRenderbuffer = 0;
        GLuint m_depthStencilAttachmentTexture = 0;
        GLuint m_depthStencilAttachmentRenderbuffer = 0;

        bool m_initialized = false;
    };

    /**
     * @brief Framebuffer utility functions
     */
    namespace FramebufferUtils
    {
        // Common framebuffer specifications
        FramebufferSpec CreateColorOnlySpec(u32 width, u32 height, GLenum format = GL_RGBA8);
        FramebufferSpec CreateColorDepthSpec(u32 width, u32 height, GLenum colorFormat = GL_RGBA8);
        FramebufferSpec CreateGBufferSpec(u32 width, u32 height); // For deferred rendering
        FramebufferSpec CreateShadowMapSpec(u32 width, u32 height);
        FramebufferSpec CreateHDRSpec(u32 width, u32 height);
        FramebufferSpec CreateMultisampledSpec(u32 width, u32 height, u32 samples, GLenum format = GL_RGBA8);

        // Validation
        bool ValidateSpec(const FramebufferSpec &spec);
        u32 GetMaxColorAttachments();
        u32 GetMaxSamples();
        bool IsFormatSupported(GLenum internalFormat);
    }

} // namespace Pyramid
