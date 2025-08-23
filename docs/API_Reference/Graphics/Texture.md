# Texture System API Reference

## Overview

The Pyramid Engine's Texture System provides comprehensive texture management and loading capabilities. It supports multiple image formats through custom loaders and integrates seamlessly with the graphics pipeline.

## Classes

### ITexture2D Interface

The base interface for all 2D texture implementations.

```cpp
class ITexture2D {
public:
    virtual ~ITexture2D() = default;
    
    virtual void Bind(u32 slot = 0) = 0;
    virtual void Unbind() = 0;
    
    virtual u32 GetWidth() const = 0;
    virtual u32 GetHeight() const = 0;
    virtual u32 GetFormat() const = 0;
    virtual u32 GetHandle() const = 0;
    
    virtual void SetData(const void* data, u32 size) = 0;
    virtual void GenerateMipmaps() = 0;
    
    static std::shared_ptr<ITexture2D> Create(const TextureSpecification& spec);
    static std::shared_ptr<ITexture2D> Create(const std::string& path);
};
```

### TextureSpecification

Structure defining texture creation parameters:

```cpp
struct TextureSpecification {
    u32 Width = 1;
    u32 Height = 1;
    TextureFormat Format = TextureFormat::RGBA8;
    TextureWrap WrapS = TextureWrap::Repeat;
    TextureWrap WrapT = TextureWrap::Repeat;
    TextureFilter MinFilter = TextureFilter::Linear;
    TextureFilter MagFilter = TextureFilter::Linear;
    bool GenerateMipmaps = true;
};
```

### TextureFormat Enumeration

Supported texture formats:

```cpp
enum class TextureFormat {
    None = 0,
    
    // Color formats
    R8,
    RG8,
    RGB8,
    RGBA8,
    
    // Floating point formats
    R16F,
    RG16F,
    RGB16F,
    RGBA16F,
    
    R32F,
    RG32F,
    RGB32F,
    RGBA32F,
    
    // Depth formats
    Depth24Stencil8,
    Depth32F,
    
    // Compressed formats
    DXT1,
    DXT3,
    DXT5,
    BC7
};
```

### TextureWrap Enumeration

Texture wrapping modes:

```cpp
enum class TextureWrap {
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};
```

### TextureFilter Enumeration

Texture filtering modes:

```cpp
enum class TextureFilter {
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};
```

## Supported Image Formats

The engine includes custom image loaders for multiple formats:

### TGA (Targa) Support
- Uncompressed RGB and RGBA
- Proper orientation handling
- Support for alpha channels

### BMP (Windows Bitmap) Support
- Windows Bitmap format
- Automatic BGR to RGB conversion
- Padding and alignment handling

### PNG Support
- Complete PNG implementation with custom DEFLATE decompression
- All PNG filter types (None, Sub, Up, Average, Paeth)
- Multiple color types (RGB, RGBA, Grayscale, Indexed)
- Custom DEFLATE implementation (RFC 1951 compliant)

### JPEG Support
- Complete baseline DCT implementation
- JPEG marker parsing and validation
- Custom Huffman decoder for DC/AC coefficients
- Dequantization with quantization table support
- Inverse DCT (IDCT) implementation
- YCbCr to RGB color space conversion

## Usage Examples

### Creating Textures from Specifications

```cpp
#include <Pyramid/Graphics/Texture.hpp>

// Create a render target texture
TextureSpecification spec;
spec.Width = 1920;
spec.Height = 1080;
spec.Format = TextureFormat::RGBA8;
spec.WrapS = TextureWrap::ClampToEdge;
spec.WrapT = TextureWrap::ClampToEdge;
spec.MinFilter = TextureFilter::Linear;
spec.MagFilter = TextureFilter::Linear;
spec.GenerateMipmaps = false;

auto renderTarget = ITexture2D::Create(spec);
```

### Loading Textures from Files

```cpp
// Load texture from file (automatically detects format)
auto playerTexture = ITexture2D::Create("assets/textures/player.tga");
auto backgroundTexture = ITexture2D::Create("assets/textures/background.png");
auto hudTexture = ITexture2D::Create("assets/textures/hud.jpg");

// Check if loading was successful
if (!playerTexture) {
    PYRAMID_LOG_ERROR("Failed to load player texture");
    return false;
}
```

### Using Textures in Shaders

```cpp
// Bind texture to a specific slot
playerTexture->Bind(0);  // Texture unit 0
backgroundTexture->Bind(1);  // Texture unit 1

// Set shader uniforms
shader->SetUniformInt("u_PlayerTexture", 0);
shader->SetUniformInt("u_BackgroundTexture", 1);

// Render with textures
device->DrawIndexed(indexCount);

// Cleanup
playerTexture->Unbind();
backgroundTexture->Unbind();
```

### Creating Procedural Textures

```cpp
// Create a procedural texture
TextureSpecification spec;
spec.Width = 256;
spec.Height = 256;
spec.Format = TextureFormat::RGBA8;

auto proceduralTexture = ITexture2D::Create(spec);

// Generate procedural data
std::vector<u8> data(256 * 256 * 4);
for (int y = 0; y < 256; ++y) {
    for (int x = 0; x < 256; ++x) {
        int index = (y * 256 + x) * 4;
        data[index + 0] = x;        // Red
        data[index + 1] = y;        // Green
        data[index + 2] = 128;      // Blue
        data[index + 3] = 255;      // Alpha
    }
}

proceduralTexture->SetData(data.data(), data.size());
proceduralTexture->GenerateMipmaps();
```

### Texture Arrays and Atlases

```cpp
// Create texture atlas specification
TextureSpecification atlasSpec;
atlasSpec.Width = 2048;
atlasSpec.Height = 2048;
atlasSpec.Format = TextureFormat::RGBA8;
atlasSpec.GenerateMipmaps = true;

auto textureAtlas = ITexture2D::Create(atlasSpec);

// Load and combine multiple textures into atlas
// (Implementation would depend on specific atlas packing algorithm)
```

## Integration with Material System

```cpp
// Materials can hold references to textures
struct Material {
    std::shared_ptr<ITexture2D> DiffuseTexture;
    std::shared_ptr<ITexture2D> NormalTexture;
    std::shared_ptr<ITexture2D> SpecularTexture;
    std::shared_ptr<ITexture2D> EmissiveTexture;
    
    Vec3 BaseColor = Vec3(1.0f);
    f32 Metallic = 0.0f;
    f32 Roughness = 0.5f;
    f32 AO = 1.0f;
};

// Apply material textures
void ApplyMaterial(const Material& material, IShader* shader) {
    if (material.DiffuseTexture) {
        material.DiffuseTexture->Bind(0);
        shader->SetUniformInt("u_DiffuseTexture", 0);
    }
    
    if (material.NormalTexture) {
        material.NormalTexture->Bind(1);
        shader->SetUniformInt("u_NormalTexture", 1);
    }
    
    // ... bind other textures
    
    shader->SetUniformVec3("u_BaseColor", material.BaseColor);
    shader->SetUniformFloat("u_Metallic", material.Metallic);
    shader->SetUniformFloat("u_Roughness", material.Roughness);
}
```

## Performance Considerations

### Memory Management
- Textures are automatically managed through smart pointers
- Large textures are loaded asynchronously when possible
- Unused textures are automatically freed when references are released

### GPU Memory Optimization
- Automatic texture compression when supported
- Mipmap generation for improved performance and quality
- Texture streaming for large worlds

### Loading Performance
- Custom image loaders optimized for performance
- Zero external dependencies reduce loading overhead
- Parallel loading of multiple textures

## Advanced Features

### Framebuffer Integration
```cpp
// Create framebuffer with color and depth textures
auto colorTexture = ITexture2D::Create(colorSpec);
auto depthTexture = ITexture2D::Create(depthSpec);

auto framebuffer = IFramebuffer::Create();
framebuffer->AttachColorTexture(colorTexture, 0);
framebuffer->AttachDepthTexture(depthTexture);
```

### Texture Streaming
```cpp
// Placeholder for future texture streaming implementation
class TextureStreamer {
public:
    void RequestTexture(const std::string& path, u32 priority);
    void UpdateStreaming();
    bool IsTextureReady(const std::string& path);
};
```

### Compressed Texture Support
```cpp
// Support for compressed texture formats
TextureSpecification compressedSpec;
compressedSpec.Format = TextureFormat::DXT5;  // Block compression
auto compressedTexture = ITexture2D::Create(compressedSpec);
```

## Error Handling

### Texture Loading Errors
```cpp
auto texture = ITexture2D::Create("path/to/texture.png");
if (!texture) {
    PYRAMID_LOG_ERROR("Failed to load texture: path/to/texture.png");
    // Use fallback texture
    texture = GetDefaultTexture();
}
```

### Format Validation
```cpp
// The texture system validates format support
if (!IsFormatSupported(TextureFormat::BC7)) {
    PYRAMID_LOG_WARN("BC7 compression not supported, falling back to RGBA8");
    spec.Format = TextureFormat::RGBA8;
}
```

## See Also

- [Graphics Device API](GraphicsDevice.md) - Low-level graphics operations
- [Shader System](../Shaders/ShaderSystem.md) - Shader management and uniforms
- [Material System](../Materials/MaterialSystem.md) - Material properties and textures
- [Framebuffer System](Framebuffer.md) - Render targets and framebuffers
