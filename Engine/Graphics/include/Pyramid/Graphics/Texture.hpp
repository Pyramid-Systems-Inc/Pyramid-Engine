#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <string>
#include <memory> // For std::shared_ptr

namespace Pyramid {

enum class TextureFormat
{
    None = 0,
    RGB8,
    RGBA8,
    // Add more as needed (e.g., SRGB, Float16, Depth, Stencil)
};

enum class TextureFilter
{
    None = 0,
    Linear,
    Nearest
};

enum class TextureWrap
{
    None = 0,
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

struct TextureSpecification
{
    u32 Width = 1;
    u32 Height = 1;
    TextureFormat Format = TextureFormat::RGBA8;
    TextureFilter MinFilter = TextureFilter::Linear;
    TextureFilter MagFilter = TextureFilter::Linear;
    TextureWrap WrapS = TextureWrap::Repeat;
    TextureWrap WrapT = TextureWrap::Repeat;
    bool GenerateMips = true;
    // Add border color if ClampToBorder is used
};

class ITexture
{
public:
    virtual ~ITexture() = default;

    virtual void Bind(u32 slot = 0) const = 0;
    virtual void Unbind(u32 slot = 0) const = 0; // Optional, usually unbinding all or binding another is enough

    virtual u32 GetWidth() const = 0;
    virtual u32 GetHeight() const = 0;
    virtual u32 GetRendererID() const = 0; // For debugging or direct OpenGL interop

    virtual const std::string& GetPath() const = 0; // If loaded from file

    // Optionally, methods to set data after creation, e.g., for dynamic textures
    // virtual void SetData(void* data, u32 size) = 0; 
};

// Consider a Texture2D specific interface if 1D/3D/CubeMaps are very different
class ITexture2D : public ITexture
{
public:
    static std::shared_ptr<ITexture2D> Create(const TextureSpecification& specification, const void* data = nullptr);
    static std::shared_ptr<ITexture2D> Create(const std::string& filepath, bool srgb = false, bool generateMips = true);
};

} // namespace Pyramid
