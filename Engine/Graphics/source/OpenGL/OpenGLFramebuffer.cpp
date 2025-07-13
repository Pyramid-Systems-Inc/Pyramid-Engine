#include <Pyramid/Graphics/OpenGL/OpenGLFramebuffer.hpp>
#include <Pyramid/Util/Log.hpp>
#include <algorithm>
#include <fstream>

namespace Pyramid
{

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpec &spec)
        : m_spec(spec)
    {
        // Validate specification
        if (!FramebufferUtils::ValidateSpec(m_spec))
        {
            PYRAMID_LOG_ERROR("Invalid framebuffer specification");
            return;
        }

        // Reserve space for attachments
        m_colorAttachmentTextures.reserve(8); // OpenGL guarantees at least 8 color attachments
        m_colorAttachmentRenderbuffers.reserve(8);
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        if (m_framebufferID != 0)
        {
            glDeleteFramebuffers(1, &m_framebufferID);
        }

        // Clean up color attachments
        if (!m_colorAttachmentTextures.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(m_colorAttachmentTextures.size()),
                             m_colorAttachmentTextures.data());
        }
        if (!m_colorAttachmentRenderbuffers.empty())
        {
            glDeleteRenderbuffers(static_cast<GLsizei>(m_colorAttachmentRenderbuffers.size()),
                                  m_colorAttachmentRenderbuffers.data());
        }

        // Clean up depth/stencil attachments
        if (m_depthAttachmentTexture != 0)
        {
            glDeleteTextures(1, &m_depthAttachmentTexture);
        }
        if (m_depthAttachmentRenderbuffer != 0)
        {
            glDeleteRenderbuffers(1, &m_depthAttachmentRenderbuffer);
        }
        if (m_stencilAttachmentTexture != 0)
        {
            glDeleteTextures(1, &m_stencilAttachmentTexture);
        }
        if (m_stencilAttachmentRenderbuffer != 0)
        {
            glDeleteRenderbuffers(1, &m_stencilAttachmentRenderbuffer);
        }
        if (m_depthStencilAttachmentTexture != 0)
        {
            glDeleteTextures(1, &m_depthStencilAttachmentTexture);
        }
        if (m_depthStencilAttachmentRenderbuffer != 0)
        {
            glDeleteRenderbuffers(1, &m_depthStencilAttachmentRenderbuffer);
        }
    }

    bool OpenGLFramebuffer::Initialize()
    {
        if (m_initialized)
        {
            PYRAMID_LOG_WARN("Framebuffer already initialized");
            return true;
        }

        // Don't create framebuffer for swap chain target
        if (m_spec.swapChainTarget)
        {
            m_initialized = true;
            return true;
        }

        // Generate framebuffer
        glGenFramebuffers(1, &m_framebufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);

        // Create attachments
        CreateAttachments();

        // Check framebuffer completeness
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            PYRAMID_LOG_ERROR("Framebuffer not complete: ", GetFramebufferStatusString(status));
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }

        // Set up draw buffers for multiple render targets
        std::vector<GLenum> drawBuffers;
        for (const auto &attachment : m_spec.attachments)
        {
            if (attachment.type == FramebufferAttachmentType::Color)
            {
                drawBuffers.push_back(GetColorAttachmentEnum(attachment.colorAttachmentIndex));
            }
        }

        if (!drawBuffers.empty())
        {
            glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
        }
        else
        {
            // No color attachments, disable color drawing
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_initialized = true;
        PYRAMID_LOG_INFO("Framebuffer created successfully: ", m_spec.width, "x", m_spec.height,
                         " with ", m_spec.attachments.size(), " attachments");
        return true;
    }

    void OpenGLFramebuffer::Bind() const
    {
        if (m_spec.swapChainTarget)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind default framebuffer
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
        }
        glViewport(0, 0, m_spec.width, m_spec.height);
    }

    void OpenGLFramebuffer::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Clear(f32 r, f32 g, f32 b, f32 a) const
    {
        Bind();
        glClearColor(r, g, b, a);

        GLbitfield clearMask = 0;

        // Check what attachments we have
        for (const auto &attachment : m_spec.attachments)
        {
            switch (attachment.type)
            {
            case FramebufferAttachmentType::Color:
                clearMask |= GL_COLOR_BUFFER_BIT;
                break;
            case FramebufferAttachmentType::Depth:
            case FramebufferAttachmentType::DepthStencil:
                clearMask |= GL_DEPTH_BUFFER_BIT;
                break;
            case FramebufferAttachmentType::Stencil:
                clearMask |= GL_STENCIL_BUFFER_BIT;
                break;
            }
        }

        if (clearMask != 0)
        {
            glClear(clearMask);
        }
    }

    void OpenGLFramebuffer::ClearAttachment(u32 attachmentIndex, const void *value) const
    {
        if (attachmentIndex >= m_spec.attachments.size())
        {
            PYRAMID_LOG_ERROR("Invalid attachment index: ", attachmentIndex);
            return;
        }

        Bind();

        const auto &attachment = m_spec.attachments[attachmentIndex];
        if (attachment.type == FramebufferAttachmentType::Color)
        {
            GLenum drawBuffer = GetColorAttachmentEnum(attachment.colorAttachmentIndex);

            // Determine clear value type based on format
            if (attachment.dataType == GL_FLOAT)
            {
                glClearBufferfv(GL_COLOR, attachment.colorAttachmentIndex, static_cast<const GLfloat *>(value));
            }
            else if (attachment.dataType == GL_INT)
            {
                glClearBufferiv(GL_COLOR, attachment.colorAttachmentIndex, static_cast<const GLint *>(value));
            }
            else
            {
                glClearBufferuiv(GL_COLOR, attachment.colorAttachmentIndex, static_cast<const GLuint *>(value));
            }
        }
    }

    bool OpenGLFramebuffer::Resize(u32 width, u32 height)
    {
        if (m_spec.width == width && m_spec.height == height)
        {
            return true; // No change needed
        }

        m_spec.width = width;
        m_spec.height = height;

        if (m_spec.swapChainTarget)
        {
            return true; // Swap chain target doesn't need recreation
        }

        // Recreate framebuffer with new size
        Invalidate();
        return Initialize();
    }

    void OpenGLFramebuffer::Invalidate()
    {
        if (m_framebufferID != 0)
        {
            glDeleteFramebuffers(1, &m_framebufferID);
            m_framebufferID = 0;
        }

        // Clean up all attachments
        if (!m_colorAttachmentTextures.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(m_colorAttachmentTextures.size()),
                             m_colorAttachmentTextures.data());
            m_colorAttachmentTextures.clear();
        }
        if (!m_colorAttachmentRenderbuffers.empty())
        {
            glDeleteRenderbuffers(static_cast<GLsizei>(m_colorAttachmentRenderbuffers.size()),
                                  m_colorAttachmentRenderbuffers.data());
            m_colorAttachmentRenderbuffers.clear();
        }

        // Reset all attachment IDs
        m_depthAttachmentTexture = 0;
        m_depthAttachmentRenderbuffer = 0;
        m_stencilAttachmentTexture = 0;
        m_stencilAttachmentRenderbuffer = 0;
        m_depthStencilAttachmentTexture = 0;
        m_depthStencilAttachmentRenderbuffer = 0;

        m_initialized = false;
    }

    GLuint OpenGLFramebuffer::GetColorAttachmentTexture(u32 index) const
    {
        if (index >= m_colorAttachmentTextures.size())
        {
            PYRAMID_LOG_ERROR("Color attachment index out of range: ", index);
            return 0;
        }
        return m_colorAttachmentTextures[index];
    }

    GLuint OpenGLFramebuffer::GetDepthAttachmentTexture() const
    {
        return m_depthStencilAttachmentTexture != 0 ? m_depthStencilAttachmentTexture : m_depthAttachmentTexture;
    }

    GLuint OpenGLFramebuffer::GetStencilAttachmentTexture() const
    {
        return m_depthStencilAttachmentTexture != 0 ? m_depthStencilAttachmentTexture : m_stencilAttachmentTexture;
    }

    void OpenGLFramebuffer::SetDrawBuffers(const std::vector<u32> &colorAttachments) const
    {
        Bind();

        std::vector<GLenum> drawBuffers;
        drawBuffers.reserve(colorAttachments.size());

        for (u32 attachment : colorAttachments)
        {
            drawBuffers.push_back(GetColorAttachmentEnum(attachment));
        }

        if (!drawBuffers.empty())
        {
            glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
        }
        else
        {
            glDrawBuffer(GL_NONE);
        }
    }

    void OpenGLFramebuffer::SetReadBuffer(u32 colorAttachment) const
    {
        Bind();
        glReadBuffer(GetColorAttachmentEnum(colorAttachment));
    }

    void OpenGLFramebuffer::BlitTo(const OpenGLFramebuffer &target,
                                   u32 srcX0, u32 srcY0, u32 srcX1, u32 srcY1,
                                   u32 dstX0, u32 dstY0, u32 dstX1, u32 dstY1,
                                   GLbitfield mask, GLenum filter) const
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.GetFramebufferID());

        glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1,
                          dstX0, dstY0, dstX1, dstY1,
                          mask, filter);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    bool OpenGLFramebuffer::IsComplete() const
    {
        if (m_spec.swapChainTarget)
        {
            return true;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    u32 OpenGLFramebuffer::GetColorAttachmentCount() const
    {
        u32 count = 0;
        for (const auto &attachment : m_spec.attachments)
        {
            if (attachment.type == FramebufferAttachmentType::Color)
            {
                count++;
            }
        }
        return count;
    }

    void OpenGLFramebuffer::SaveColorAttachmentToFile(u32 attachmentIndex, const std::string &filepath) const
    {
        if (attachmentIndex >= m_colorAttachmentTextures.size())
        {
            PYRAMID_LOG_ERROR("Color attachment index out of range: ", attachmentIndex);
            return;
        }

        // Read pixels from the attachment
        auto pixels = ReadColorAttachmentPixels(attachmentIndex);
        if (pixels.empty())
        {
            PYRAMID_LOG_ERROR("Failed to read pixels from color attachment ", attachmentIndex);
            return;
        }

        // Save to file (simple PPM format for now)
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open())
        {
            PYRAMID_LOG_ERROR("Failed to open file for writing: ", filepath);
            return;
        }

        // Write PPM header
        file << "P6\n"
             << m_spec.width << " " << m_spec.height << "\n255\n";

        // Write pixel data (assuming RGBA format, convert to RGB)
        for (size_t i = 0; i < pixels.size(); i += 4)
        {
            file.write(reinterpret_cast<const char *>(&pixels[i]), 3); // Write RGB, skip A
        }

        file.close();
        PYRAMID_LOG_INFO("Saved framebuffer attachment to: ", filepath);
    }

    std::vector<u8> OpenGLFramebuffer::ReadColorAttachmentPixels(u32 attachmentIndex) const
    {
        if (attachmentIndex >= m_colorAttachmentTextures.size())
        {
            PYRAMID_LOG_ERROR("Color attachment index out of range: ", attachmentIndex);
            return {};
        }

        Bind();
        SetReadBuffer(attachmentIndex);

        std::vector<u8> pixels(m_spec.width * m_spec.height * 4); // Assuming RGBA
        glReadPixels(0, 0, m_spec.width, m_spec.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        return pixels;
    }

    void OpenGLFramebuffer::CreateAttachments()
    {
        for (const auto &attachmentSpec : m_spec.attachments)
        {
            switch (attachmentSpec.type)
            {
            case FramebufferAttachmentType::Color:
                CreateColorAttachment(attachmentSpec);
                break;
            case FramebufferAttachmentType::Depth:
                CreateDepthAttachment(attachmentSpec);
                break;
            case FramebufferAttachmentType::Stencil:
                CreateStencilAttachment(attachmentSpec);
                break;
            case FramebufferAttachmentType::DepthStencil:
                CreateDepthStencilAttachment(attachmentSpec);
                break;
            }
        }
    }

    void OpenGLFramebuffer::CreateColorAttachment(const FramebufferAttachmentSpec &spec)
    {
        GLuint texture = CreateTexture2D(m_spec.width, m_spec.height,
                                         spec.internalFormat, spec.format, spec.dataType,
                                         spec.multisampled, spec.samples);

        if (texture == 0)
        {
            PYRAMID_LOG_ERROR("Failed to create color attachment texture");
            return;
        }

        // Ensure we have enough space in the vector
        while (m_colorAttachmentTextures.size() <= spec.colorAttachmentIndex)
        {
            m_colorAttachmentTextures.push_back(0);
        }

        m_colorAttachmentTextures[spec.colorAttachmentIndex] = texture;
        AttachTexture2D(texture, GetColorAttachmentEnum(spec.colorAttachmentIndex));

        PYRAMID_LOG_DEBUG("Created color attachment ", spec.colorAttachmentIndex, " with texture ID ", texture);
    }

    void OpenGLFramebuffer::CreateDepthAttachment(const FramebufferAttachmentSpec &spec)
    {
        GLuint texture = CreateTexture2D(m_spec.width, m_spec.height,
                                         spec.internalFormat, GL_DEPTH_COMPONENT, GL_FLOAT,
                                         spec.multisampled, spec.samples);

        if (texture == 0)
        {
            PYRAMID_LOG_ERROR("Failed to create depth attachment texture");
            return;
        }

        m_depthAttachmentTexture = texture;
        AttachTexture2D(texture, GL_DEPTH_ATTACHMENT);

        PYRAMID_LOG_DEBUG("Created depth attachment with texture ID ", texture);
    }

    void OpenGLFramebuffer::CreateStencilAttachment(const FramebufferAttachmentSpec &spec)
    {
        GLuint texture = CreateTexture2D(m_spec.width, m_spec.height,
                                         spec.internalFormat, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
                                         spec.multisampled, spec.samples);

        if (texture == 0)
        {
            PYRAMID_LOG_ERROR("Failed to create stencil attachment texture");
            return;
        }

        m_stencilAttachmentTexture = texture;
        AttachTexture2D(texture, GL_STENCIL_ATTACHMENT);

        PYRAMID_LOG_DEBUG("Created stencil attachment with texture ID ", texture);
    }

    void OpenGLFramebuffer::CreateDepthStencilAttachment(const FramebufferAttachmentSpec &spec)
    {
        GLuint texture = CreateTexture2D(m_spec.width, m_spec.height,
                                         spec.internalFormat, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                                         spec.multisampled, spec.samples);

        if (texture == 0)
        {
            PYRAMID_LOG_ERROR("Failed to create depth-stencil attachment texture");
            return;
        }

        m_depthStencilAttachmentTexture = texture;
        AttachTexture2D(texture, GL_DEPTH_STENCIL_ATTACHMENT);

        PYRAMID_LOG_DEBUG("Created depth-stencil attachment with texture ID ", texture);
    }

    GLuint OpenGLFramebuffer::CreateTexture2D(u32 width, u32 height, GLenum internalFormat,
                                              GLenum format, GLenum type, bool multisampled, u32 samples)
    {
        GLuint texture;
        glGenTextures(1, &texture);

        if (multisampled && samples > 1)
        {
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_TRUE);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);

            // Set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glBindTexture(multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 0);
        return texture;
    }

    GLuint OpenGLFramebuffer::CreateRenderbuffer(u32 width, u32 height, GLenum internalFormat,
                                                 bool multisampled, u32 samples)
    {
        GLuint renderbuffer;
        glGenRenderbuffers(1, &renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);

        if (multisampled && samples > 1)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, width, height);
        }
        else
        {
            glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        return renderbuffer;
    }

    void OpenGLFramebuffer::AttachTexture2D(GLuint texture, GLenum attachment, u32 mipLevel)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, mipLevel);
    }

    void OpenGLFramebuffer::AttachRenderbuffer(GLuint renderbuffer, GLenum attachment)
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer);
    }

    GLenum OpenGLFramebuffer::GetColorAttachmentEnum(u32 index)
    {
        return GL_COLOR_ATTACHMENT0 + index;
    }

    std::string OpenGLFramebuffer::GetFramebufferStatusString(GLenum status)
    {
        switch (status)
        {
        case GL_FRAMEBUFFER_COMPLETE:
            return "Complete";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "Incomplete attachment";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "Missing attachment";
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            return "Incomplete draw buffer";
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            return "Incomplete read buffer";
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "Unsupported";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "Incomplete multisample";
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            return "Incomplete layer targets";
        default:
            return "Unknown error";
        }
    }

    // FramebufferUtils Implementation
    namespace FramebufferUtils
    {

        FramebufferSpec CreateColorOnlySpec(u32 width, u32 height, GLenum format)
        {
            FramebufferSpec spec;
            spec.width = width;
            spec.height = height;

            FramebufferAttachmentSpec colorAttachment;
            colorAttachment.type = FramebufferAttachmentType::Color;
            colorAttachment.internalFormat = format;
            colorAttachment.format = (format == GL_RGBA8) ? GL_RGBA : GL_RGB;
            colorAttachment.dataType = GL_UNSIGNED_BYTE;
            colorAttachment.colorAttachmentIndex = 0;

            spec.attachments.push_back(colorAttachment);
            return spec;
        }

        FramebufferSpec CreateColorDepthSpec(u32 width, u32 height, GLenum colorFormat)
        {
            FramebufferSpec spec;
            spec.width = width;
            spec.height = height;

            // Color attachment
            FramebufferAttachmentSpec colorAttachment;
            colorAttachment.type = FramebufferAttachmentType::Color;
            colorAttachment.internalFormat = colorFormat;
            colorAttachment.format = (colorFormat == GL_RGBA8) ? GL_RGBA : GL_RGB;
            colorAttachment.dataType = GL_UNSIGNED_BYTE;
            colorAttachment.colorAttachmentIndex = 0;

            // Depth attachment
            FramebufferAttachmentSpec depthAttachment;
            depthAttachment.type = FramebufferAttachmentType::Depth;
            depthAttachment.internalFormat = GL_DEPTH_COMPONENT24;
            depthAttachment.format = GL_DEPTH_COMPONENT;
            depthAttachment.dataType = GL_FLOAT;

            spec.attachments.push_back(colorAttachment);
            spec.attachments.push_back(depthAttachment);
            return spec;
        }

        FramebufferSpec CreateGBufferSpec(u32 width, u32 height)
        {
            FramebufferSpec spec;
            spec.width = width;
            spec.height = height;

            // G-Buffer layout for deferred rendering:
            // RT0: Albedo (RGB) + Metallic (A)
            FramebufferAttachmentSpec albedoMetallic;
            albedoMetallic.type = FramebufferAttachmentType::Color;
            albedoMetallic.internalFormat = GL_RGBA8;
            albedoMetallic.format = GL_RGBA;
            albedoMetallic.dataType = GL_UNSIGNED_BYTE;
            albedoMetallic.colorAttachmentIndex = 0;

            // RT1: Normal (RGB) + Roughness (A)
            FramebufferAttachmentSpec normalRoughness;
            normalRoughness.type = FramebufferAttachmentType::Color;
            normalRoughness.internalFormat = GL_RGBA16F;
            normalRoughness.format = GL_RGBA;
            normalRoughness.dataType = GL_HALF_FLOAT;
            normalRoughness.colorAttachmentIndex = 1;

            // RT2: Motion Vectors (RG) + AO (B) + Material ID (A)
            FramebufferAttachmentSpec motionAO;
            motionAO.type = FramebufferAttachmentType::Color;
            motionAO.internalFormat = GL_RGBA16F;
            motionAO.format = GL_RGBA;
            motionAO.dataType = GL_HALF_FLOAT;
            motionAO.colorAttachmentIndex = 2;

            // RT3: Emissive (RGB) + Flags (A)
            FramebufferAttachmentSpec emissive;
            emissive.type = FramebufferAttachmentType::Color;
            emissive.internalFormat = GL_RGBA16F;
            emissive.format = GL_RGBA;
            emissive.dataType = GL_HALF_FLOAT;
            emissive.colorAttachmentIndex = 3;

            // Depth-Stencil
            FramebufferAttachmentSpec depthStencil;
            depthStencil.type = FramebufferAttachmentType::DepthStencil;
            depthStencil.internalFormat = GL_DEPTH24_STENCIL8;
            depthStencil.format = GL_DEPTH_STENCIL;
            depthStencil.dataType = GL_UNSIGNED_INT_24_8;

            spec.attachments.push_back(albedoMetallic);
            spec.attachments.push_back(normalRoughness);
            spec.attachments.push_back(motionAO);
            spec.attachments.push_back(emissive);
            spec.attachments.push_back(depthStencil);

            return spec;
        }

        FramebufferSpec CreateShadowMapSpec(u32 width, u32 height)
        {
            FramebufferSpec spec;
            spec.width = width;
            spec.height = height;

            // Depth-only for shadow mapping
            FramebufferAttachmentSpec depthAttachment;
            depthAttachment.type = FramebufferAttachmentType::Depth;
            depthAttachment.internalFormat = GL_DEPTH_COMPONENT32F;
            depthAttachment.format = GL_DEPTH_COMPONENT;
            depthAttachment.dataType = GL_FLOAT;
            depthAttachment.minFilter = GL_LINEAR;
            depthAttachment.magFilter = GL_LINEAR;
            depthAttachment.wrapS = GL_CLAMP_TO_BORDER;
            depthAttachment.wrapT = GL_CLAMP_TO_BORDER;

            spec.attachments.push_back(depthAttachment);
            return spec;
        }

        FramebufferSpec CreateHDRSpec(u32 width, u32 height)
        {
            FramebufferSpec spec;
            spec.width = width;
            spec.height = height;

            // HDR color attachment
            FramebufferAttachmentSpec hdrColor;
            hdrColor.type = FramebufferAttachmentType::Color;
            hdrColor.internalFormat = GL_RGBA16F;
            hdrColor.format = GL_RGBA;
            hdrColor.dataType = GL_HALF_FLOAT;
            hdrColor.colorAttachmentIndex = 0;

            // Depth attachment
            FramebufferAttachmentSpec depthAttachment;
            depthAttachment.type = FramebufferAttachmentType::Depth;
            depthAttachment.internalFormat = GL_DEPTH_COMPONENT24;
            depthAttachment.format = GL_DEPTH_COMPONENT;
            depthAttachment.dataType = GL_FLOAT;

            spec.attachments.push_back(hdrColor);
            spec.attachments.push_back(depthAttachment);
            return spec;
        }

        FramebufferSpec CreateMultisampledSpec(u32 width, u32 height, u32 samples, GLenum format)
        {
            FramebufferSpec spec;
            spec.width = width;
            spec.height = height;
            spec.samples = samples;

            // Multisampled color attachment
            FramebufferAttachmentSpec colorAttachment;
            colorAttachment.type = FramebufferAttachmentType::Color;
            colorAttachment.internalFormat = format;
            colorAttachment.format = (format == GL_RGBA8) ? GL_RGBA : GL_RGB;
            colorAttachment.dataType = GL_UNSIGNED_BYTE;
            colorAttachment.multisampled = true;
            colorAttachment.samples = samples;
            colorAttachment.colorAttachmentIndex = 0;

            // Multisampled depth attachment
            FramebufferAttachmentSpec depthAttachment;
            depthAttachment.type = FramebufferAttachmentType::Depth;
            depthAttachment.internalFormat = GL_DEPTH_COMPONENT24;
            depthAttachment.format = GL_DEPTH_COMPONENT;
            depthAttachment.dataType = GL_FLOAT;
            depthAttachment.multisampled = true;
            depthAttachment.samples = samples;

            spec.attachments.push_back(colorAttachment);
            spec.attachments.push_back(depthAttachment);
            return spec;
        }

        bool ValidateSpec(const FramebufferSpec &spec)
        {
            if (spec.width == 0 || spec.height == 0)
            {
                PYRAMID_LOG_ERROR("Invalid framebuffer dimensions: ", spec.width, "x", spec.height);
                return false;
            }

            if (spec.attachments.empty())
            {
                PYRAMID_LOG_ERROR("Framebuffer must have at least one attachment");
                return false;
            }

            u32 colorAttachmentCount = 0;
            u32 depthAttachmentCount = 0;
            u32 stencilAttachmentCount = 0;

            for (const auto &attachment : spec.attachments)
            {
                switch (attachment.type)
                {
                case FramebufferAttachmentType::Color:
                    colorAttachmentCount++;
                    if (attachment.colorAttachmentIndex >= GetMaxColorAttachments())
                    {
                        PYRAMID_LOG_ERROR("Color attachment index exceeds maximum: ",
                                          attachment.colorAttachmentIndex, " >= ", GetMaxColorAttachments());
                        return false;
                    }
                    break;
                case FramebufferAttachmentType::Depth:
                case FramebufferAttachmentType::DepthStencil:
                    depthAttachmentCount++;
                    break;
                case FramebufferAttachmentType::Stencil:
                    stencilAttachmentCount++;
                    break;
                }
            }

            if (depthAttachmentCount > 1)
            {
                PYRAMID_LOG_ERROR("Framebuffer can only have one depth attachment");
                return false;
            }

            if (stencilAttachmentCount > 1)
            {
                PYRAMID_LOG_ERROR("Framebuffer can only have one stencil attachment");
                return false;
            }

            return true;
        }

        u32 GetMaxColorAttachments()
        {
            static u32 maxColorAttachments = 0;
            if (maxColorAttachments == 0)
            {
                GLint max;
                glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max);
                maxColorAttachments = static_cast<u32>(max);
            }
            return maxColorAttachments;
        }

        u32 GetMaxSamples()
        {
            static u32 maxSamples = 0;
            if (maxSamples == 0)
            {
                GLint max;
                glGetIntegerv(GL_MAX_SAMPLES, &max);
                maxSamples = static_cast<u32>(max);
            }
            return maxSamples;
        }

        bool IsFormatSupported(GLenum internalFormat)
        {
            // Check if the format is supported by querying OpenGL
            GLint supported;
            glGetInternalformativ(GL_TEXTURE_2D, internalFormat, GL_INTERNALFORMAT_SUPPORTED, 1, &supported);
            return supported == GL_TRUE;
        }

    } // namespace FramebufferUtils

} // namespace Pyramid
