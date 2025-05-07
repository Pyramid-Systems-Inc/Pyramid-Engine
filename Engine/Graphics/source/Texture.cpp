#include "Pyramid/Graphics/Texture.hpp"
#include "Pyramid/Graphics/OpenGL/OpenGLTexture.hpp" // For OpenGLTexture2D
// #include "Pyramid/Renderer/RendererAPI.hpp" // Would be needed for multi-API support

namespace Pyramid {

// For now, we assume OpenGL. A more complete system would check the current RendererAPI.
// This is similar to how IGraphicsDevice::Create works.

std::shared_ptr<ITexture2D> ITexture2D::Create(const TextureSpecification& specification, const void* data)
{
    // switch (RendererAPI::GetCurrentAPI()) // Example for future multi-API
    // {
    //     case RendererAPI::API::None:    /* PYRAMID_CORE_ASSERT(false, "RendererAPI::None is not supported!"); */ return nullptr;
    //     case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLTexture2D>(specification, data);
    // }
    // /* PYRAMID_CORE_ASSERT(false, "Unknown RendererAPI!"); */
    
    // Assuming OpenGL for now
    return std::make_shared<OpenGLTexture2D>(specification, data);
}

std::shared_ptr<ITexture2D> ITexture2D::Create(const std::string& filepath, bool srgb, bool generateMips)
{
    // Assuming OpenGL for now
    return std::make_shared<OpenGLTexture2D>(filepath, srgb, generateMips);
}

} // namespace Pyramid
