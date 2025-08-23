# Image Loading System API Reference

## Overview

The Pyramid Engine features a comprehensive, zero-dependency image loading system that supports multiple formats including TGA, BMP, PNG, and JPEG. All loaders are implemented from scratch for maximum performance and compatibility.

## Supported Formats

### TGA (Targa) Support
- **File Extensions**: `.tga`, `.targa`
- **Color Modes**: RGB, RGBA
- **Compression**: Uncompressed only
- **Special Features**: Proper orientation handling, alpha channel support

### BMP (Windows Bitmap) Support
- **File Extensions**: `.bmp`
- **Color Modes**: RGB (24-bit)
- **Special Features**: Automatic BGR to RGB conversion, padding handling

### PNG Support
- **File Extensions**: `.png`
- **Color Modes**: RGB, RGBA, Grayscale, Indexed color
- **Compression**: Custom DEFLATE implementation (RFC 1951 compliant)
- **Filters**: All PNG filter types (None, Sub, Up, Average, Paeth)
- **Special Features**: Complete PNG specification support

### JPEG Support
- **File Extensions**: `.jpg`, `.jpeg`
- **Color Modes**: RGB (baseline DCT)
- **Compression**: Full JPEG baseline implementation
- **Special Features**: Custom Huffman decoder, IDCT implementation, YCbCr to RGB conversion

## Core Classes

### Image Class

The main image container class:

```cpp
class Image {
public:
    Image();
    Image(u32 width, u32 height, u32 channels, const u8* data = nullptr);
    ~Image();
    
    // Loading methods
    bool LoadFromFile(const std::string& path);
    bool LoadFromMemory(const u8* data, size_t size);
    
    // Access methods
    u32 GetWidth() const;
    u32 GetHeight() const;
    u32 GetChannels() const;
    u32 GetPitch() const;
    const u8* GetData() const;
    u8* GetData();
    
    // Utility methods
    bool IsValid() const;
    size_t GetSizeInBytes() const;
    void Clear();
    
    // Pixel access
    void SetPixel(u32 x, u32 y, const Color& color);
    Color GetPixel(u32 x, u32 y) const;
    
private:
    u32 m_width;
    u32 m_height;
    u32 m_channels;
    std::unique_ptr<u8[]> m_data;
};
```

### Format-Specific Loaders

#### PNGLoader
```cpp
class PNGLoader {
public:
    static std::unique_ptr<Image> Load(const std::string& path);
    static std::unique_ptr<Image> LoadFromMemory(const u8* data, size_t size);
    
private:
    // PNG-specific implementation details
    static bool ValidateSignature(const u8* data);
    static bool ProcessChunks(const u8* data, size_t size, PNGInfo& info);
    static bool DecodeImageData(const u8* compressedData, size_t size, PNGInfo& info);
};
```

#### JPEGLoader
```cpp
class JPEGLoader {
public:
    static std::unique_ptr<Image> Load(const std::string& path);
    static std::unique_ptr<Image> LoadFromMemory(const u8* data, size_t size);
    
private:
    // JPEG-specific implementation
    static bool ParseMarkers(const u8* data, size_t size, JPEGInfo& info);
    static bool DecodeHuffman(const u8* data, JPEGInfo& info);
    static bool PerformIDCT(const f32* coefficients, u8* output);
    static void YCbCrToRGB(const u8* ycbcr, u8* rgb, u32 width, u32 height);
};
```

## Usage Examples

### Basic Image Loading

```cpp
#include <Pyramid/Util/Image.hpp>

// Load any supported image format
auto image = std::make_unique<Pyramid::Util::Image>();
if (!image->LoadFromFile("assets/textures/player.png")) {
    PYRAMID_LOG_ERROR("Failed to load image: player.png");
    return nullptr;
}

PYRAMID_LOG_INFO("Loaded image: ", image->GetWidth(), "x", image->GetHeight(), 
                 " (", image->GetChannels(), " channels)");
```

### Format-Specific Loading

```cpp
// PNG loading with custom error handling
auto pngImage = Pyramid::Util::PNGLoader::Load("texture.png");
if (!pngImage) {
    PYRAMID_LOG_ERROR("PNG loading failed");
    return false;
}

// JPEG loading
auto jpegImage = Pyramid::Util::JPEGLoader::Load("photo.jpg");
if (!jpegImage) {
    PYRAMID_LOG_ERROR("JPEG loading failed");
    return false;
}

// TGA loading (through generic Image loader)
auto tgaImage = std::make_unique<Pyramid::Util::Image>();
if (!tgaImage->LoadFromFile("sprite.tga")) {
    PYRAMID_LOG_ERROR("TGA loading failed");
    return false;
}
```

### Memory-Based Loading

```cpp
// Load image from memory buffer (useful for embedded resources)
std::vector<u8> imageData = LoadEmbeddedResource("embedded_texture.png");

auto image = std::make_unique<Pyramid::Util::Image>();
if (!image->LoadFromMemory(imageData.data(), imageData.size())) {
    PYRAMID_LOG_ERROR("Failed to load image from memory");
    return false;
}
```

### Pixel Manipulation

```cpp
// Create a procedural image
auto proceduralImage = std::make_unique<Pyramid::Util::Image>(256, 256, 4);

// Generate a gradient pattern
for (u32 y = 0; y < 256; ++y) {
    for (u32 x = 0; x < 256; ++x) {
        Color pixel;
        pixel.r = static_cast<f32>(x) / 255.0f;
        pixel.g = static_cast<f32>(y) / 255.0f;
        pixel.b = 0.5f;
        pixel.a = 1.0f;
        
        proceduralImage->SetPixel(x, y, pixel);
    }
}
```

### Integration with Graphics System

```cpp
// Convert Image to graphics texture
auto image = std::make_unique<Pyramid::Util::Image>();
if (image->LoadFromFile("texture.png")) {
    // Create texture specification based on image properties
    TextureSpecification spec;
    spec.Width = image->GetWidth();
    spec.Height = image->GetHeight();
    
    switch (image->GetChannels()) {
        case 1: spec.Format = TextureFormat::R8; break;
        case 2: spec.Format = TextureFormat::RG8; break;
        case 3: spec.Format = TextureFormat::RGB8; break;
        case 4: spec.Format = TextureFormat::RGBA8; break;
        default:
            PYRAMID_LOG_ERROR("Unsupported channel count: ", image->GetChannels());
            return nullptr;
    }
    
    // Create texture and upload image data
    auto texture = ITexture2D::Create(spec);
    texture->SetData(image->GetData(), image->GetSizeInBytes());
    
    return texture;
}
```

## Advanced Features

### Custom DEFLATE Implementation

The PNG loader includes a complete DEFLATE decompressor:

```cpp
class Inflate {
public:
    static bool Decompress(const u8* compressedData, size_t compressedSize,
                          u8* outputData, size_t outputSize);
    
private:
    // RFC 1951 compliant implementation
    static bool ProcessBlock(BitReader& reader, u8* output, size_t& outputPos);
    static bool BuildHuffmanTree(const u32* codeLengths, u32 count, HuffmanTree& tree);
};
```

### JPEG DCT Implementation

The JPEG loader includes a complete Inverse Discrete Cosine Transform:

```cpp
class JPEGIDCT {
public:
    static void PerformIDCT(const f32* coefficients, u8* output);
    
private:
    static void IDCT1D(f32* data);
    static constexpr f32 CosineTable[8][8] = { /* precomputed values */ };
};
```

### PNG Filter Processing

```cpp
class PNGFilters {
public:
    static void ApplyFilter(u8 filterType, u8* currentRow, const u8* previousRow,
                           u32 width, u32 bytesPerPixel);
    
private:
    static u8 PaethPredictor(u8 a, u8 b, u8 c);
    static void SubFilter(u8* current, u32 width, u32 bpp);
    static void UpFilter(u8* current, const u8* previous, u32 width);
    static void AverageFilter(u8* current, const u8* previous, u32 width, u32 bpp);
    static void PaethFilter(u8* current, const u8* previous, u32 width, u32 bpp);
};
```

## Performance Considerations

### Memory Management
- **Zero Copy**: Direct loading into destination buffers when possible
- **RAII**: Automatic memory cleanup with smart pointers
- **Pool Allocation**: Reusable memory pools for frequent loading

### Loading Performance
- **Single Pass**: Most formats loaded in a single pass through the data
- **Minimal Allocations**: Reduced memory allocations during loading
- **Cache Efficiency**: Data structures optimized for cache performance

### Error Handling
```cpp
// Comprehensive error checking
auto image = std::make_unique<Pyramid::Util::Image>();
if (!image->LoadFromFile("texture.png")) {
    // Check specific error conditions
    if (!std::filesystem::exists("texture.png")) {
        PYRAMID_LOG_ERROR("Image file not found: texture.png");
    } else {
        PYRAMID_LOG_ERROR("Image format not supported or file corrupted: texture.png");
    }
    
    // Use fallback image
    image = CreateDefaultTexture();
}
```

## Format Implementation Details

### PNG Implementation
- **Complete Specification**: Supports all PNG features including interlacing
- **Custom DEFLATE**: Zero-dependency decompression implementation
- **All Filter Types**: Proper handling of all PNG filter algorithms
- **Color Type Support**: RGB, RGBA, Grayscale, and Indexed color modes

### JPEG Implementation
- **Baseline DCT**: Standard JPEG baseline implementation
- **Huffman Decoding**: Custom Huffman decoder for DC/AC coefficients
- **Quantization**: Full dequantization support with custom tables
- **Color Conversion**: Optimized YCbCr to RGB conversion

### TGA Implementation
- **Fast Loading**: Optimized for common TGA variants
- **Orientation**: Proper handling of image orientation flags
- **Alpha Channel**: Full support for RGBA TGA files

### BMP Implementation
- **Windows Bitmap**: Standard Windows BMP format support
- **Color Conversion**: Automatic BGR to RGB conversion
- **Padding**: Proper handling of row padding requirements

## Testing and Validation

The image loading system includes comprehensive test coverage:

```cpp
// Example test usage
void TestImageLoading() {
    // Test various formats
    auto pngImage = LoadTestImage("test.png");
    auto jpegImage = LoadTestImage("test.jpg");
    auto tgaImage = LoadTestImage("test.tga");
    auto bmpImage = LoadTestImage("test.bmp");
    
    // Validate image properties
    PYRAMID_ASSERT(pngImage->GetWidth() == 256, "PNG width mismatch");
    PYRAMID_ASSERT(jpegImage->GetChannels() == 3, "JPEG channel count mismatch");
    
    // Test pixel data integrity
    ValidateImageData(pngImage.get());
    ValidateImageData(jpegImage.get());
}
```

## See Also

- [Texture System](../Graphics/Texture.md) - Graphics texture management
- [Resource Management](../Core/ResourceManager.md) - Asset loading and caching
- [File System](../Utils/FileSystem.md) - File I/O operations
- [Error Handling](../Core/ErrorHandling.md) - Error reporting and recovery
