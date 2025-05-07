# Pyramid Game Engine Roadmap

## Current Version: 0.3.8

### ✅ Implemented Features (As of May 2025)

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

### Q1 2025 (March) - Progress Update

- [X] Graphics Enhancement Phase 1
  - [X] Vertex Array Objects (VAO) - Enhanced with `BufferLayout` system for attribute definition.
  - [X] Enhanced shader management - `IGraphicsDevice::CreateShader()` added; shader compilation verified; basic `glUniform*` style support implemented in `IShader` and `OpenGLShader`.
    - [~] Uniform buffer support (Basic `glUniform*` support added; true Uniform Buffer Objects (UBOs) still pending)
    - [ ] Multiple shader programs (Supported by creating multiple IShader instances, advanced management pending)
  - [~] Texture system implementation (Basic 2D texture loading via stb_image and rendering implemented)
  - Material system foundation
  - State management system

- [ ] Core Systems Enhancement
  - Resource management system
  - Memory management improvements
  - [~] Extended input handling - Game loop message processing fixed, foundational for input.
  - [ ] File I/O system

### Q2 2025 (June)

- [ ] Graphics Enhancement Phase 2
  - Multiple render targets
  - Basic post-processing pipeline
  - Render state caching
  - Primitive batching
  - Basic particle system

- [ ] Scene Management
  - Scene graph implementation
  - Transform hierarchy
  - Basic camera system
  - View frustum culling

### Q3 2025 (September)

- [ ] Physics Integration
  - Collision detection
  - Rigid body dynamics
  - Physics debug rendering
  - Ray casting

- [ ] Audio Foundation
  - Audio device abstraction
  - Basic sound playback
  - 3D audio positioning
  - Sound resource management

### Q4 2025 (December)

- [ ] Asset Pipeline
  - Asset importing system
  - Resource management
  - Hot reloading support
  - Asset optimization tools

- [ ] Developer Tools
  - Debug UI system
  - Performance profiling
  - Memory tracking
  - Scene inspector

### 2026 Planning

- [ ] Advanced Graphics
  - PBR materials
  - Shadow mapping
  - Particle system
  - Deferred rendering

- [ ] Engine Features
  - Entity Component System (ECS)
  - Scripting system
  - Animation system
  - Networking foundation

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
