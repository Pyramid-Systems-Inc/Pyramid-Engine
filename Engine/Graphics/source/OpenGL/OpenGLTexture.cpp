#include "Pyramid/Graphics/OpenGL/OpenGLTexture.hpp"
#include <Pyramid/Core/Log.hpp>
#include <Pyramid/Util/Image.hpp> // Our custom image loader
#include <glad/glad.h>

namespace Pyramid
{

    // Helper to convert engine enums to OpenGL enums
    static GLenum TextureFormatToGLInternalFormat(TextureFormat format, bool srgb)
    {
        if (srgb)
        {
            switch (format)
            {
            case TextureFormat::RGB8:
                return GL_SRGB8;
            case TextureFormat::RGBA8:
                return GL_SRGB8_ALPHA8;
            default:
                break;
            }
        }
        else
        {
            switch (format)
            {
            case TextureFormat::RGB8:
                return GL_RGB8;
            case TextureFormat::RGBA8:
                return GL_RGBA8;
            default:
                break;
            }
        }
        // PYRAMID_CORE_ASSERT(false, "Unknown TextureFormat for internal format!");
        return GL_RGBA8; // Default
    }

    static GLenum TextureFormatToGLDataFormat(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGB8:
            return GL_RGB;
        case TextureFormat::RGBA8:
            return GL_RGBA;
        default:
            break;
        }
        // PYRAMID_CORE_ASSERT(false, "Unknown TextureFormat for data format!");
        return GL_RGBA; // Default
    }

    static GLenum TextureFilterToGLFilter(TextureFilter filter)
    {
        switch (filter)
        {
        case TextureFilter::Linear:
            return GL_LINEAR;
        case TextureFilter::Nearest:
            return GL_NEAREST;
        default:
            break;
        }
        return GL_LINEAR;
    }

    static GLenum TextureWrapToGLWrap(TextureWrap wrap)
    {
        switch (wrap)
        {
        case TextureWrap::Repeat:
            return GL_REPEAT;
        case TextureWrap::MirroredRepeat:
            return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge:
            return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder:
            return GL_CLAMP_TO_BORDER;
        default:
            break;
        }
        return GL_REPEAT;
    }

    OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &specification, const void *data)
        : m_Specification(specification), m_Filepath("")
    {
        m_InternalFormat = TextureFormatToGLInternalFormat(m_Specification.Format, false /* TODO: handle srgb from spec? */);
        m_DataFormat = TextureFormatToGLDataFormat(m_Specification.Format);
        Invalidate(data);
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string &filepath, bool srgb, bool generateMips)
        : m_Filepath(filepath)
    {
        // Load image using our custom Pyramid image loader
        Util::ImageData imageData = Util::Image::Load(filepath);

        if (imageData.Data != nullptr)
        {
            m_Specification.Width = imageData.Width;
            m_Specification.Height = imageData.Height;
            m_Specification.GenerateMips = generateMips;

            if (imageData.Channels == 4)
            {
                m_Specification.Format = TextureFormat::RGBA8;
                m_InternalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
                m_DataFormat = GL_RGBA;
            }
            else if (imageData.Channels == 3)
            {
                m_Specification.Format = TextureFormat::RGB8;
                m_InternalFormat = srgb ? GL_SRGB8 : GL_RGB8;
                m_DataFormat = GL_RGB;
            }
            else
            {
                PYRAMID_LOG_ERROR("Unsupported image format for ", filepath, ". Channels: ", imageData.Channels);
                Util::Image::Free(imageData.Data);
                m_RendererID = 0; // Indicate failure
                return;
            }

            // Note: Our custom image loader already handles the coordinate system correctly for OpenGL
            // (bottom-left origin), so we don't need to flip like we did with stb_image
            Invalidate(imageData.Data);
            Util::Image::Free(imageData.Data);
        }
        else
        {
            PYRAMID_LOG_ERROR("Failed to load image: ", filepath, " using Pyramid image loader");
            m_RendererID = 0; // Indicate failure
        }
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        if (m_RendererID)
        {
            glDeleteTextures(1, &m_RendererID);
        }
    }

    void OpenGLTexture2D::Invalidate(const void *data)
    {
        if (m_RendererID) // If re-creating, delete old one
        {
            glDeleteTextures(1, &m_RendererID);
            m_RendererID = 0;
        }

        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);

        // Upload data
        // GL_UNSIGNED_BYTE is assumed for RGB8/RGBA8 from stb_image
        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Specification.Width, m_Specification.Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);

        SetParameters();

        if (m_Specification.GenerateMips && data) // Only generate mips if data was provided (not for empty textures initially)
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind
    }

    void OpenGLTexture2D::SetParameters()
    {
        // Assumes texture is already bound
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilterToGLFilter(m_Specification.MinFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilterToGLFilter(m_Specification.MagFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrapToGLWrap(m_Specification.WrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrapToGLWrap(m_Specification.WrapT));

        // TODO: Add support for GL_TEXTURE_BORDER_COLOR if WrapMode is ClampToBorder
        // float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }

    void OpenGLTexture2D::Bind(u32 slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
    }

    void OpenGLTexture2D::Unbind(u32 slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Definition for ITexture2D static factory methods
    // These should ideally be in a Texture.cpp or similar, and decide which concrete type to make.
    // For now, we'll put placeholder definitions here or assume they will be linked if defined elsewhere.
    // This part needs careful thought on how the API should work (GraphicsDevice creates textures vs static Texture::Create)

    // Placeholder - actual implementation would depend on current graphics API
    // std::shared_ptr<ITexture2D> ITexture2D::Create(const TextureSpecification& specification, const void* data)
    // {
    //     // switch (RendererAPI::GetAPI()) // Or however the current API is determined
    //     // {
    //     //    case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(specification, data);
    //     // }
    //     // PYRAMID_CORE_ASSERT(false, "Unknown RendererAPI!");
    //     return std::make_shared<OpenGLTexture2D>(specification, data); // Assume OpenGL for now
    // }

    // std::shared_ptr<ITexture2D> ITexture2D::Create(const std::string& filepath, bool srgb, bool generateMips)
    // {
    //     return std::make_shared<OpenGLTexture2D>(filepath, srgb, generateMips); // Assume OpenGL for now
    // }

} // namespace Pyramid
