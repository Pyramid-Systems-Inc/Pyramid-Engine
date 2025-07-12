# Pyramid Game Engine

A modern, multi-platform game engine with support for multiple graphics APIs. Currently focusing on OpenGL with planned support for DirectX 9/10/11/12.

## Features

- **Advanced Math Library with SIMD Optimization**
  - **SIMD-Accelerated Operations**: SSE/AVX optimized vector and matrix operations
  - **Comprehensive 3D Math**: Vec2, Vec3, Vec4, Mat3, Mat4, Quaternion classes
  - **Runtime SIMD Detection**: Automatic CPU feature detection with graceful fallbacks
  - **Performance Optimizations**: Fast inverse square root, trigonometric approximations
  - **Production-Ready**: Thread-safe, cache-friendly, and extensively tested

- **Enhanced Rendering System Architecture**
  - **Command Buffer System**: Efficient GPU command submission and batching
  - **Render Pass Framework**: Organized rendering stages (forward, deferred, post-processing)
  - **Modern OpenGL 4.6**: Uniform Buffer Objects, advanced shaders, instanced rendering
  - **PBR Material System**: Physically-based rendering with metallic-roughness workflow
  - **Camera System**: Perspective/orthographic projections with frustum culling
  - **Scene Graph**: Hierarchical transforms with efficient update propagation

- **Multiple Graphics API Support**
  - OpenGL 4.6 (current, fully implemented)
  - DirectX 9/10/11/12 (planned)
  - Clean abstraction layer
  - Automatic version detection

- **Custom Image Processing Library**
  - **Zero External Dependencies**: Completely self-contained implementation
  - **TGA Support**: Uncompressed RGB/RGBA with proper orientation handling
  - **BMP Support**: Windows Bitmap format with padding and BGR conversion
  - **PNG Support**: Complete implementation with custom DEFLATE decompression
    - All PNG filter types (None, Sub, Up, Average, Paeth)
    - Multiple color types (RGB, RGBA, Grayscale, Indexed)
    - Custom DEFLATE implementation (RFC 1951 compliant)
  - **JPEG Support**: Complete baseline DCT implementation
    - JPEG marker parsing and validation
    - Custom Huffman decoder for DC/AC coefficients
    - Dequantization with quantization table support
    - Inverse DCT (IDCT) implementation
    - YCbCr to RGB color space conversion
  - **Production Ready**: Comprehensive error handling and test coverage

- **Enhanced Logging System**
  - Thread-safe logging with mutex protection
  - Multiple log levels (Trace, Debug, Info, Warn, Error, Critical)
  - File output with automatic rotation and size limits
  - Configurable console and file logging levels
  - Source location tracking (file, function, line)
  - Structured logging with key-value pairs
  - Stream-style and C-style logging interfaces
  - Performance optimizations with early exit filtering

- **Modern C++ Design**
  - C++17 features
  - RAII resource management
  - Smart pointer usage
  - Exception safety

- **Window Management**
  - Native window handling
  - Event processing
  - Multiple monitor support
  - Resolution management
  - Window state management
  - Message processing
  - Graphics context integration

- **Graphics Features**
  - **Modern Rendering Pipeline**: Command buffers, render passes, and efficient batching
  - **OpenGL 4.6 Implementation**: Uniform Buffer Objects, advanced shaders, instanced rendering
  - **Resource Management**: Smart pointer-based resource lifecycle management
  - **Shader System**: GLSL support with uniform blocks and binding point management
  - **Texture System**: Advanced 2D texture loading with custom image loaders
  - **Vertex Management**: VAOs with configurable layouts, VBOs, and IBOs
  - **Camera System**: Perspective/orthographic projections with view frustum culling
  - **Material System**: PBR-ready material framework with texture binding
  - **VSync Support**: Configurable vertical synchronization
  - **Multi-API Ready**: Clean abstraction layer for future DirectX support

## Requirements

- Windows 10 or later
- Visual Studio 2022
- CMake 3.16.0 or later
- Graphics card supporting:
  - OpenGL 3.3+ (current)
  - DirectX 9+ (for future features)

## Quick Start

1. Clone the repository:

```bash
git clone https://github.com/yourusername/Pyramid.git
cd Pyramid
```

2. Generate project files:

```bash
cmake -B build -S .
```

3. Build the engine:

```bash
cmake --build build --config Debug
```

4. Run the example game:

```bash
./build/bin/Debug/BasicGame.exe
```

## Usage Examples

### Enhanced Logging System

The Pyramid Engine features a production-ready logging system with multiple interfaces:

```cpp
#include <Pyramid/Util/Log.hpp>

// Configure the logger (optional - has sensible defaults)
Pyramid::Util::LoggerConfig config;
config.enableConsole = true;
config.enableFile = true;
config.consoleLevel = Pyramid::Util::LogLevel::Info;
config.fileLevel = Pyramid::Util::LogLevel::Debug;
config.logFilePath = "game.log";
config.maxFileSize = 10 * 1024 * 1024; // 10MB
config.maxFiles = 5; // Keep 5 rotated files
PYRAMID_CONFIGURE_LOGGER(config);

// Basic logging with multiple arguments
PYRAMID_LOG_INFO("Game started with version: ", 1.0f);
PYRAMID_LOG_WARN("Low memory warning: ", availableMemory, " MB remaining");
PYRAMID_LOG_ERROR("Failed to load texture: ", filename);

// Stream-style logging
PYRAMID_INFO_STREAM() << "Player position: " << player.x << ", " << player.y;
PYRAMID_ERROR_STREAM() << "Critical error in " << __FUNCTION__;

// Structured logging for analytics
std::unordered_map<std::string, std::string> fields;
fields["user_id"] = "12345";
fields["level"] = "forest_1";
fields["score"] = "1500";
PYRAMID_LOG_STRUCTURED(Pyramid::Util::LogLevel::Info, "Level completed", fields);

// Enhanced assertions with logging
PYRAMID_ASSERT(player != nullptr, "Player object should not be null");
PYRAMID_CORE_ASSERT(device->IsValid(), "Graphics device must be valid");
```

### Enhanced Math Library with SIMD

The Pyramid Engine features a production-ready math library with SIMD optimizations:

```cpp
#include <Pyramid/Math/Math.hpp>

using namespace Pyramid::Math;

// SIMD-optimized vector operations
Vec3 position(1.0f, 2.0f, 3.0f);
Vec3 velocity(0.5f, -1.0f, 0.2f);
Vec3 newPosition = position + velocity * deltaTime;

// Quaternion rotations with SIMD acceleration
Quat rotation = Quat::FromEuler(Radians(45.0f), Radians(30.0f), 0.0f);
Vec3 forward = rotation.RotateVector(Vec3::Forward);

// Matrix transformations with SIMD
Mat4 transform = Mat4::CreateTranslation(position) *
                 rotation.ToMatrix4() *
                 Mat4::CreateScale(Vec3(2.0f));

// Camera matrices
Mat4 view = Mat4::CreateLookAt(cameraPos, target, Vec3::Up);
Mat4 projection = Mat4::CreatePerspective(Radians(60.0f), aspectRatio, 0.1f, 100.0f);

// Check SIMD capabilities at runtime
if (SIMD::IsAvailable()) {
    PYRAMID_LOG_INFO("SIMD Features: ", SIMD::GetFeatureString());
    // Use SIMD-optimized batch operations
    SIMD::Batch::NormalizeVectors(vectors, results, count);
}
```

### Enhanced Rendering System

```cpp
#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Graphics/Scene.hpp>

// Create rendering system
auto renderSystem = std::make_unique<Renderer::RenderSystem>();
renderSystem->Initialize(graphicsDevice);

// Setup camera with enhanced math
Camera camera(Math::Radians(60.0f), 16.0f/9.0f, 0.1f, 1000.0f);
camera.SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
camera.LookAt(Math::Vec3::Zero);

// Create scene with hierarchical transforms
auto scene = SceneUtils::CreateTestScene();
auto cubeNode = scene->CreateNode("Cube");
cubeNode->SetLocalPosition(Math::Vec3(2.0f, 0.0f, 0.0f));

// Render with command buffers and render passes
renderSystem->BeginFrame();
renderSystem->Render(*scene, camera);
renderSystem->EndFrame();
```

### Graphics System

```cpp
// Create and use graphics resources
auto device = GetGraphicsDevice();
auto shader = device->CreateShader();
shader->Compile(vertexSource, fragmentSource);

auto texture = device->CreateTexture2D("assets/player.tga");
shader->SetUniformInt("u_Texture", 0);
```

## Documentation

- [Getting Started Guide](docs/GettingStarted.md)
- [Architecture Overview](docs/Architecture.md)
- [Changelog](CHANGELOG.md)

## Project Structure

```
Pyramid/
├── Engine/                 # Core engine library
│   ├── Core/              # Core engine functionality
│   ├── Graphics/          # Graphics abstraction layer
│   ├── Platform/          # Platform-specific code
│   ├── Math/              # Math library
│   ├── Utils/             # Enhanced logging system & utilities
│   ├── Renderer/          # Rendering system
│   ├── Input/             # Input handling
│   ├── Scene/             # Scene management
│   ├── Audio/             # Audio system
│   └── Physics/           # Physics system
├── Examples/              # Example projects
│   └── BasicGame/         # Basic game example
├── Tools/                 # Development tools
│   └── AssetProcessor/    # Asset processing tools
├── Tests/                 # Test projects
│   ├── Unit/             # Unit tests
│   └── Integration/       # Integration tests
├── vendor/               # Third-party dependencies
│   └── glad/             # OpenGL loader
└── docs/                 # Documentation
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Submit a pull request
4. Follow our coding standards

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- GLAD for OpenGL loading
- Windows API for window management
- CMake for build system
