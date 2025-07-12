# Pyramid Game Engine

A modern, multi-platform game engine with support for multiple graphics APIs. Currently focusing on OpenGL with planned support for DirectX 9/10/11/12.

## Features

- **Multiple Graphics API Support**
  - OpenGL 3.3 - 4.6 (current)
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
  - Multiple API support (OpenGL current, DirectX planned)
  - Version detection
  - Feature management (via `IGraphicsDevice` abstraction)
  - Resource abstraction:
    - Shader system with GLSL support (via `IGraphicsDevice::CreateShader()`)
    - Uniform variable support in shaders (`IShader::SetUniform*` methods)
    - Advanced 2D Texture loading with custom image loader (TGA, BMP, PNG, JPEG) and rendering (`ITexture2D`)
    - Vertex Array Objects (VAOs) with configurable vertex attribute layouts (`BufferLayout`)
    - Vertex Buffer Objects (VBOs) and Index Buffer Objects (IBOs)
  - VSync support
  - Modern OpenGL context

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
