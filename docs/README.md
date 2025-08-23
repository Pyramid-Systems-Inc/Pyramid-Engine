# Pyramid Game Engine Documentation

Welcome to the Pyramid Game Engine documentation! Pyramid is a modern, multi-platform game engine designed with flexibility and performance in mind. This documentation provides comprehensive information about the engine's architecture, API reference, and usage examples.

## Quick Links

- [Getting Started](GettingStarted.md) - Setup guide and basic usage
- [Build Guide](BuildGuide.md) - Detailed build and installation instructions
- [Architecture Overview](Architecture.md) - Engine architecture and design principles
- [API Reference](API_Reference/) - Detailed API documentation
- [Examples and Tutorials](Examples/) - Example projects and tutorials
- [Contributing](Contributing.md) - Contributing guidelines and development setup

## Engine Overview

Pyramid Engine is a production-ready game engine featuring:

- **High-Performance Math Library**: SIMD-optimized mathematical operations with runtime CPU feature detection
- **Modern Rendering Architecture**: Multi-API graphics abstraction with OpenGL 3.3-4.6 support
- **Advanced Scene Management**: Spatial partitioning with octree-based object queries
- **Comprehensive Logging System**: Thread-safe logging with file rotation and structured output
- **Modular Design**: Clean separation between engine modules for extensibility

## Key Features

### Math Library
- SIMD-accelerated vector and matrix operations
- Comprehensive 3D math utilities (Vec2, Vec3, Vec4, Mat3, Mat4, Quat)
- Batch processing for arrays of mathematical objects
- Runtime CPU feature detection for optimal performance

### Graphics System
- Multi-API abstraction layer (OpenGL, with DirectX planned)
- Advanced camera system with frustum culling
- Flexible shader system with uniform buffer objects
- Efficient buffer management and state handling

### Scene Management
- Octree-based spatial partitioning
- Multiple query types (point, sphere, box, frustum, ray)
- Performance monitoring and statistics
- Scene serialization and deserialization

### Platform Support
- Windows 10/11 (primary)
- Linux and macOS (planned)
- Multiple graphics APIs (OpenGL 3.3-4.6, DirectX planned)

## Documentation Structure

```
docs/
├── README.md                           # This file - main entry point
├── GettingStarted.md                   # Setup and basic usage guide
├── Architecture.md                     # Engine architecture overview
├── Enhanced_Architecture.md            # Detailed architecture information
├── API_Reference/                      # API documentation
│   ├── Math/
│   │   └── MathLibrary.md             # Math library reference
│   ├── Graphics/
│   │   ├── GraphicsDevice.md          # Graphics device abstraction
│   │   ├── Camera.md                  # Camera system
│   │   └── Scene/
│   │       ├── SceneManager.md        # Scene management
│   │       └── Octree.md              # Spatial partitioning
│   └── Core/
│       └── Game.md                    # Core game class
└── Examples/                          # Example projects and tutorials
    ├── BasicGame/                     # Basic game example
    ├── HelloTriangle/                 # Simple rendering example
    └── AdvancedScenes/                # Advanced scene management
```

## Getting Started

1. **Prerequisites**
   - Windows 10 or later
   - Visual Studio 2022
   - CMake 3.16.0 or later
   - Graphics card supporting OpenGL 3.3 or later

2. **Build the Engine**
   ```bash
   git clone https://github.com/yourusername/Pyramid.git
   cd Pyramid
   cmake -B build -S .
   cmake --build build --config Debug
   ```

3. **Create Your First Game**
   - See [Getting Started](GettingStarted.md) for a complete tutorial
   - Check the [Examples](../Examples/) directory for sample projects
   - Review the [API Reference](API_Reference/) for detailed documentation

## Architecture Overview

The Pyramid Engine follows a modular architecture with clear separation of concerns:

### Core Layers
- **Core**: Fundamental engine functionality and game loop
- **Platform**: Platform-specific implementations (Windows, Linux, macOS)
- **Graphics**: Rendering abstraction and graphics API implementations
- **Math**: SIMD-optimized mathematical operations
- **Utilities**: Logging, asset management, and utility functions

### Key Design Principles
- **Performance-First**: SIMD optimizations, efficient memory layouts, and minimal overhead
- **Modern C++**: C++17 features with RAII principles and smart pointer management
- **Modular Design**: Clean interfaces and dependency injection for extensibility
- **Multi-API Support**: Abstraction layers for multiple graphics APIs and platforms

## API Reference

The API Reference provides detailed documentation for all engine components:

### Core Engine
- [Game Class](API_Reference/Core/Game.md) - Main game loop and engine management
- Window management and platform abstraction
- Event handling and cross-platform support

### Graphics System
- [Graphics Device](API_Reference/Graphics/GraphicsDevice.md) - Low-level graphics interface
- [Render System](API_Reference/Graphics/RenderSystem.md) - High-level rendering pipeline
- [Camera System](API_Reference/Graphics/Camera.md) - Advanced camera management
- [Scene Manager](API_Reference/Graphics/SceneManager.md) - Spatial partitioning and object management
- [Texture System](API_Reference/Graphics/Texture.md) - Comprehensive texture loading and management

### Math Library
- [Math Library Overview](API_Reference/Math/MathLibrary.md) - SIMD-optimized mathematical operations
- Vector types: Vec2, Vec3, Vec4 with full SIMD support
- Matrix types: Mat3, Mat4 for transformations
- Quaternion: Quat for rotations
- Runtime CPU feature detection and optimization

### Platform Layer
- [Window System](API_Reference/Platform/Window.md) - Cross-platform window management
- OpenGL context creation and management
- Event processing and input integration

### Utilities and Services
- [Logging System](API_Reference/Utils/Logging.md) - Production-ready logging with file rotation
- [Image Loading](API_Reference/Utils/ImageLoading.md) - Zero-dependency image format support

### Planned Systems
- [Audio System](API_Reference/Audio/AudioSystem.md) - 3D spatial audio (in development)
- [Physics System](API_Reference/Physics/PhysicsSystem.md) - Rigid body dynamics (in development)
- [Input System](API_Reference/Input/InputSystem.md) - Multi-device input handling (in development)

## Examples and Tutorials

The engine includes several example projects to help you get started:

### Basic Game Example
Located in `Examples/BasicGame/`, this demonstrates:
- Basic game setup
- Window management
- Simple rendering
- Input handling

### Hello Triangle Example
A minimal example showing:
- Basic graphics device setup
- Shader creation and usage
- Vertex buffer management
- Simple triangle rendering

### Advanced Scene Management
Demonstrates:
- Spatial partitioning with octrees
- Multiple query types
- Performance monitoring
- Scene serialization

## Community and Support

- **GitHub**: [https://github.com/yourusername/Pyramid](https://github.com/yourusername/Pyramid)
- **Issues**: Report bugs and request features on GitHub Issues
- **Discussions**: Join our community discussions on GitHub
- **Discord**: Real-time chat with the community (link coming soon)

## Contributing

We welcome contributions to the Pyramid Engine! Please see our contributing guidelines for more information:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

Pyramid Engine is licensed under the MIT License. See the [LICENSE](../LICENSE) file for details.

## Changelog

See the [CHANGELOG.md](../CHANGELOG.md) file for information about recent changes and updates to the engine.

---

*This documentation is continuously updated. For the latest information, always check the GitHub repository.*