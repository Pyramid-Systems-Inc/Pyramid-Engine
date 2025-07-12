# Pyramid Game Engine Roadmap

## Current Version: 0.4.0 (July 2025)

### 🎯 Current Focus: Math Library and Scene Management Foundation

### ✅ Implemented Features (As of July 2025)

- Core Systems
  - Window management system with Unicode support (title usage fixed)
  - Event handling and message processing (game loop verified and fixed)
  - Basic game loop (initialization and message loop corrected)
  - Platform abstraction layer (GraphicsDevice decoupled from concrete Window)
  - CMake build system
  - `Game` class owns `Window` and `IGraphicsDevice`

- Graphics Foundation
  - OpenGL 3.3+ support with GLAD (initialization verified)
  - Graphics API abstraction layer (`IGraphicsDevice` with `CreateShader`)
  - Basic shader system (creation via `IGraphicsDevice`, compilation, linking verified)
  - Vertex buffer management (VBO)
  - Index buffer management (IBO)
  - Vertex Array Objects (VAO) with configurable BufferLayout system
  - Depth testing
  - Basic geometry rendering (triangle example working)
  - Vsync support

### Q2 2025 (July) - Major Image Processing Achievement

- [X] **Complete Custom Image Loading Library**
  - [X] **TGA Support**: Uncompressed RGB/RGBA with proper orientation handling
  - [X] **BMP Support**: Windows Bitmap format with padding and BGR conversion
  - [X] **PNG Support**: Complete implementation with custom DEFLATE decompression
    - [X] All PNG filter types (None, Sub, Up, Average, Paeth)
    - [X] Multiple color types (RGB, RGBA, Grayscale, Indexed)
    - [X] Custom DEFLATE implementation (RFC 1951 compliant)
    - [X] Custom ZLIB wrapper with Adler-32 checksums
  - [X] **JPEG Support**: Complete baseline DCT implementation
    - [X] JPEG marker parsing (SOI, SOF0, DQT, DHT, SOS, EOI)
    - [X] Custom Huffman decoder for DC/AC coefficients
    - [X] Dequantization with quantization table support
    - [X] Inverse DCT (IDCT) implementation
    - [X] YCbCr to RGB color space conversion
  - [X] **Zero External Dependencies**: Completely self-contained implementation
  - [X] **Production Ready**: Comprehensive error handling and test coverage
  - [X] **Real-World Integration**: Successfully integrated with BasicGame example

### Q1 2025 (March) - COMPLETED ✅

- [X] **Graphics Enhancement Phase 1** - COMPLETED
  - [X] Vertex Array Objects (VAO) - Enhanced with `BufferLayout` system for attribute definition
  - [X] Enhanced shader management - `IGraphicsDevice::CreateShader()` added; shader compilation verified; complete `glUniform*` support implemented in `IShader` and `OpenGLShader`
    - [X] Uniform support (Complete `glUniform*` support for Int, Float, Vec2/3/4, Mat3/4)
    - [X] Multiple shader programs (Supported by creating multiple IShader instances with proper management)
  - [X] Texture system implementation (2D texture loading with custom Pyramid image loader supporting all major formats)
    - [X] Custom Pyramid::Util::Image loader (Complete: TGA, BMP, PNG, and JPEG support)
    - [X] Integration with OpenGL texture pipeline (replaced stb_image dependency)
    - [X] PNG support with DEFLATE decompression (Phase 2 complete: full PNG implementation)
    - [X] JPEG support with DCT decompression (Phase 3 complete: full JPEG baseline implementation)
  - [~] Material system foundation (Basic texture binding implemented, advanced material system pending)
  - [~] State management system (Basic OpenGL state management, comprehensive system pending)

- [X] **Core Systems Enhancement** - MOSTLY COMPLETED
  - [X] Enhanced logging system - Production-ready logging with thread safety, file rotation, structured logging, and multiple interfaces (moved from Core to Utils)
  - [~] Resource management system (Basic texture resource management implemented, comprehensive system pending)
  - [~] Memory management improvements (Basic improvements made, advanced optimizations pending)
  - [X] Extended input handling - Game loop message processing fixed and working
  - [ ] File I/O system (Not yet implemented)

### Q3 2025 (September) - CURRENT PRIORITY

- [ ] **Math Library Foundation** - CRITICAL DEPENDENCY
  - [ ] Vector math (Vec2, Vec3, Vec4) with comprehensive operations
  - [ ] Matrix operations (Mat3, Mat4) with transformations
  - [ ] Quaternions for rotations and interpolation
  - [ ] Transformation utilities and coordinate system conversions
  - [ ] Mathematical constants and utility functions

- [ ] **Scene Management System** - HIGH PRIORITY
  - [ ] Scene graph implementation with hierarchical transforms
  - [ ] Transform hierarchy with parent-child relationships
  - [ ] Basic camera system (perspective and orthographic)
  - [ ] View frustum culling for performance optimization
  - [ ] Scene node management and traversal

- [ ] **Input System Enhancement** - HIGH PRIORITY
  - [ ] Keyboard input handling with key state management
  - [ ] Mouse input handling with button and movement tracking
  - [ ] Input event system with callbacks
  - [ ] Input mapping and binding system for customization

### Q4 2025 (December) - GRAPHICS ENHANCEMENT

- [ ] **Graphics Enhancement Phase 2**
  - [ ] Multiple render targets for advanced rendering techniques
  - [ ] Basic post-processing pipeline with shader effects
  - [ ] Render state caching for performance optimization
  - [ ] Primitive batching for efficient rendering
  - [ ] Basic particle system for visual effects

### Q1 2026 - PHYSICS AND AUDIO

- [ ] **Physics Integration**
  - [ ] Collision detection system with basic shapes
  - [ ] Rigid body dynamics with mass and velocity
  - [ ] Physics debug rendering for visualization
  - [ ] Ray casting for interaction and queries
  - [ ] Integration with Scene Management system

- [ ] **Audio Foundation**
  - [ ] Audio device abstraction layer
  - [ ] Basic sound playback with format support
  - [ ] 3D audio positioning with spatial audio
  - [ ] Sound resource management and streaming

### Q2 2026 - ASSET PIPELINE AND TOOLS

- [ ] **Asset Pipeline**
  - [ ] Asset importing system for models and animations
  - [ ] Comprehensive resource management with caching
  - [ ] Hot reloading support for development workflow
  - [ ] Asset optimization tools and compression

- [ ] **Developer Tools**
  - [ ] Debug UI system with ImGui integration
  - [ ] Performance profiling with frame time analysis
  - [ ] Memory tracking and leak detection
  - [ ] Scene inspector for runtime debugging
  - [ ] Enhanced log analysis tools and viewers
  - [ ] Remote logging capabilities for deployed builds

### Q3-Q4 2026 - ADVANCED FEATURES

- [ ] **Advanced Graphics**
  - [ ] PBR (Physically Based Rendering) materials with metallic-roughness workflow
  - [ ] Shadow mapping with cascaded shadow maps
  - [ ] Advanced particle system with GPU simulation
  - [ ] Deferred rendering pipeline for complex lighting

- [ ] **Engine Features**
  - [ ] Entity Component System (ECS) for flexible game object architecture
  - [ ] Scripting system with Lua or C# integration
  - [ ] Animation system with skeletal and keyframe animation
  - [ ] Networking foundation for multiplayer support

### Future Considerations

- DirectX 12 renderer
- Vulkan support
- Cross-platform support
  - Linux
  - macOS
- Mobile platform support
- VR/AR capabilities
- Ray tracing integration

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines and coding standards.

## Note

This roadmap is maintained based on project progress and community feedback. Priorities and timelines may adjust based on development needs.

## Version History

See [CHANGELOG.md](CHANGELOG.md) for detailed version history and changes.
