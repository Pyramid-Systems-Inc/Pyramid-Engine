#pragma once

#include "Pyramid/Graphics/Texture.hpp"
#include <glad/glad.h>

namespace Pyramid {

class OpenGLTexture2D : public ITexture2D
{
public:
    OpenGLTexture2D(const TextureSpecification& specification, const void* data);
    OpenGLTexture2D(const std::string& filepath, bool srgb, bool generateMips);
    ~OpenGLTexture2D() override;

    void Bind(u32 slot = 0) const override;
    void Unbind(u32 slot = 0) const override;

    u32 GetWidth() const override { return m_Specification.Width; }
    u32 GetHeight() const override { return m_Specification.Height; }
    u32 GetRendererID() const override { return m_RendererID; }
    const std::string& GetPath() const override { return m_Filepath; }

    // virtual void SetData(void* data, u32 size) override; // Implement if needed

private:
    void Invalidate(const void* data); // Helper to create/recreate texture
    void SetParameters();

    TextureSpecification m_Specification;
    std::string m_Filepath; // Empty if not loaded from file
    GLuint m_RendererID = 0;
    GLenum m_InternalFormat = GL_RGBA8; // Determined from TextureSpecification::Format
    GLenum m_DataFormat = GL_RGBA;      // Determined from TextureSpecification::Format
};

} // namespace Pyramid
