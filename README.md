# Pyramid Game Engine

A modern, multi-platform game engine with support for multiple graphics APIs. Currently focusing on OpenGL with planned support for DirectX 9/10/11/12.

## Features

- **Multiple Graphics API Support**
  - OpenGL 3.3 - 4.6 (current)
  - DirectX 9/10/11/12 (planned)
  - Clean abstraction layer
  - Automatic version detection

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
    - Basic 2D Texture loading (PNG, JPG, etc. via stb_image) and rendering (`ITexture2D`)
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
│   ├── Utils/             # Utility functions
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
│   ├── glad/             # OpenGL loader
│   └── stb/              # Image loading
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
