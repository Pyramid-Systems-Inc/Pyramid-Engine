# Pyramid Game Engine Architecture

## Overview

Pyramid is a modern, multi-platform game engine designed with flexibility and performance in mind. The engine supports multiple graphics APIs and provides a clean abstraction layer for rendering. Built with C++17 best practices, the engine features a multi-layered architecture that separates concerns while maintaining high performance through SIMD optimizations and efficient memory management.

## Multi-Layered Architecture

The Pyramid Engine is organized into five distinct layers, each with specific responsibilities:

### 1. Core Layer
- **Game Loop**: Main game loop with update and render cycles
- **Event System**: Platform-independent event handling
- **Memory Management**: Custom allocators and memory pools
- **Configuration**: Engine settings and runtime configuration
- **Platform Abstraction**: Unified interface for platform-specific operations

### 2. Platform Layer
- **Window Management**: Platform-specific window creation and management
- **Input Handling**: Keyboard, mouse, and controller input
- **System Integration**: File I/O, timing, and system services
- **Graphics Context**: Platform-specific graphics context management

### 3. Graphics Layer
- **Graphics Device Abstraction**: Multi-API support (OpenGL, DirectX planned)
- **Rendering Pipeline**: Organized rendering stages with command buffers
- **Resource Management**: Textures, buffers, and shader management
- **Camera System**: Advanced camera with projections and frustum culling
- **Scene Rendering**: Efficient scene rendering with visibility culling

### 4. Math Layer
- **SIMD-Optimized Operations**: Runtime CPU feature detection
- **Vector and Matrix Classes**: Vec2, Vec3, Vec4, Mat3, Mat4
- **Quaternion Support**: Rotations and orientation
- **Geometric Utilities**: Intersection testing and spatial operations
- **Performance Optimizations**: Fast approximations and batch processing

### 5. Utilities Layer
- **Logging System**: Thread-safe logging with file rotation
- **Asset Management**: Resource loading and management
- **Serialization**: Scene and object serialization
- **Debug Tools**: Performance monitoring and visualization
- **Math Utilities**: Additional mathematical functions and utilities

## Core Components

### Enhanced Logging System

The Pyramid Engine features a production-ready logging system located in the Utils module:

#### Key Features

- **Thread Safety**: Full mutex protection with deadlock prevention
- **Multiple Log Levels**: Trace, Debug, Info, Warn, Error, Critical with runtime filtering
- **File Output**: Automatic rotation with configurable size limits (default 10MB, 5 files)
- **Source Location**: Automatic tracking of file, function, and line numbers
- **Structured Logging**: Key-value pair support for analytics and debugging
- **Multiple Interfaces**: C-style, stream-style, and structured logging APIs
- **Performance Optimized**: Early exit filtering and local buffer management

#### Architecture

- **Logger Class**: Singleton pattern with thread-safe operations
- **LoggerConfig**: Runtime configuration for all logging aspects
- **LogEntry**: Structured representation of log messages with metadata
- **SourceLocation**: Automatic capture of source code location information
- **File Rotation**: Automatic log file management with size-based rotation

#### Usage Patterns

```cpp
// Basic logging
PYRAMID_LOG_INFO("Game started with version: ", 1.0f);

// Stream-style logging
PYRAMID_ERROR_STREAM() << "Failed to load: " << filename;

// Structured logging for analytics
std::unordered_map<std::string, std::string> fields;
fields["level"] = "forest_1";
fields["score"] = "1500";
PYRAMID_LOG_STRUCTURED(LogLevel::Info, "Level completed", fields);
```

### Scene Management Core Architecture

The Pyramid Engine features a production-ready scene management system with advanced spatial partitioning capabilities, located in the `Pyramid::SceneManagement` namespace:

#### Key Components

- **SceneManager**: Central scene lifecycle management and organization
- **Octree**: Spatial partitioning system for efficient object queries
- **AABB**: Axis-aligned bounding box implementation with intersection testing
- **Query System**: Multiple spatial query types with performance optimization
- **Performance Monitoring**: Real-time statistics and profiling capabilities

#### Architecture Overview

```cpp
namespace Pyramid::SceneManagement {
    class SceneManager {
        // Scene lifecycle management
        std::shared_ptr<Scene> CreateScene(const std::string& name);
        void SetActiveScene(std::shared_ptr<Scene> scene);

        // Spatial partitioning
        void EnableSpatialPartitioning(bool enable);
        void SetOctreeDepth(u32 depth);
        void RebuildSpatialPartition();

        // Spatial queries
        QueryResult QueryScene(const QueryParams& params);
        std::vector<std::shared_ptr<RenderObject>> GetVisibleObjects(const Camera& camera);
        std::vector<std::shared_ptr<RenderObject>> GetObjectsInRadius(const Vec3& center, f32 radius);
        std::shared_ptr<RenderObject> GetNearestObject(const Vec3& position);

        // Performance monitoring
        const SceneStats& GetStats() const;
    };

    class Octree {
        // Spatial partitioning with configurable depth and object limits
        void Insert(std::shared_ptr<RenderObject> object);
        std::vector<std::shared_ptr<RenderObject>> QueryPoint(const Vec3& point);
        std::vector<std::shared_ptr<RenderObject>> QuerySphere(const Vec3& center, f32 radius);
        std::vector<std::shared_ptr<RenderObject>> QueryBox(const AABB& bounds);
        std::shared_ptr<RenderObject> FindNearest(const Vec3& position);
    };
}
```

#### Spatial Partitioning Features

- **Octree Implementation**: Hierarchical spatial subdivision with configurable depth (default 8 levels)
- **Query Types**: Point, sphere, box, ray, and frustum-based spatial queries
- **Performance**: O(log n) object lookup complexity vs O(n) brute force
- **Memory Efficient**: Smart pointer-based resource management with RAII principles
- **Configurable**: Adjustable octree depth and objects per node for different use cases

#### Query System

The scene management system supports multiple query types optimized for different game scenarios:

- **Point Queries**: Find objects at specific locations
- **Sphere Queries**: Radius-based object discovery (explosions, AI awareness)
- **Box Queries**: Rectangular region object detection (triggers, areas)
- **Ray Queries**: Line-based intersection testing (line of sight, projectiles)
- **Frustum Queries**: Camera-based visibility culling for rendering

#### Performance Monitoring

Real-time statistics tracking for optimization and debugging:

```cpp
struct SceneStats {
    u32 totalNodes;        // Total scene nodes
    u32 totalObjects;      // Total render objects
    u32 visibleObjects;    // Currently visible objects
    u32 octreeNodes;       // Octree node count
    u32 octreeDepth;       // Current octree depth
    f32 lastQueryTime;     // Last query execution time (ms)
    f32 lastUpdateTime;    // Last update execution time (ms)
};
```

### Graphics System

The graphics system is built around a flexible abstraction layer that supports multiple graphics APIs with a focus on performance and modern rendering techniques:

#### Graphics Device Architecture

- **IGraphicsDevice**: Core interface for all graphics implementations
  - OpenGL (3.3 - 4.6) - Currently implemented
  - DirectX 9/10/11/12 (planned)
  - Vulkan (planned)
  - Metal (planned for Apple platforms)

#### Key Graphics Components

- **Command Buffer System**: Efficient GPU command batching and submission
- **Render Pass Framework**: Organized rendering stages with clear separation
- **Resource Management**: Unified handling of textures, buffers, and shaders
- **State Management**: Optimized graphics state changes with minimal overhead
- **Camera System**: Advanced camera with projections and frustum culling
- **Scene Management**: Spatial partitioning and visibility culling

#### Graphics Features

- **Multi-API Support**: Unified interface across different graphics APIs
- **Version Detection**: Automatic feature detection and fallback
- **Modern OpenGL**: UBOs, advanced shaders, instanced rendering
- **Buffer Management**: Vertex, index, uniform, and shader storage buffers
- **Shader System**: Flexible shader creation and management
- **Texture Management**: 2D texture creation and loading from files
- **State Optimization**: Minimal state changes and efficient batching

#### Rendering Pipeline

1. **Geometry Stage**
   - Vertex buffer binding and attribute setup
   - Index buffer management for indexed rendering
   - Instance data for instanced rendering

2. **Shader Stage**
   - Vertex and fragment shader compilation and linking
   - Uniform buffer object management
   - Shader storage buffer for compute operations

3. **Rasterization Stage**
   - Viewport and scissor setup
   - Depth testing and stencil operations
   - Blending and color operations

4. **Output Stage**
   - Framebuffer management
   - Multiple render targets support
   - Present with optional vsync

### Window Management

The window management system provides a platform-independent interface for window creation and management:

#### Components

- **Window**: Abstract window interface
  - Window creation and destruction
  - Event processing
  - Context management
  - Window state handling

- **Win32OpenGLWindow**: Windows implementation
  - Native Win32 API integration
  - OpenGL context management
  - Window procedure handling
  - Message processing

#### Features

- Platform-specific window implementations
- Event handling system
- Input management
- Resolution and display mode handling
- Window state management (minimize, maximize, close)
- Message processing and event dispatch
- Graphics context integration

### Game Loop

The main game loop is managed by the `Game` class, which provides:

- Game state management
- Update and render cycle
- Event processing
- Graphics device management

## Directory Structure

```text
Pyramid/
├── Engine/                # Core engine library
│   ├── Core/             # Core engine functionality
│   │   ├── include/      # Public headers
│   │   └── source/       # Implementation files
│   ├── Graphics/         # Graphics abstraction layer
│   │   ├── include/      # Public headers
│   │   │   └── Scene/    # Scene Management Core Architecture
│   │   │       ├── SceneManager.hpp  # Scene lifecycle management
│   │   │       └── Octree.hpp        # Spatial partitioning system
│   │   └── source/       # Implementation files
│   │       └── Scene/    # Scene Management implementations
│   │           ├── SceneManager.cpp  # Scene manager implementation
│   │           └── Octree.cpp        # Octree spatial partitioning
│   ├── Platform/         # Platform-specific code
│   │   ├── include/      # Public headers
│   │   └── source/       # Implementation files
│   ├── Math/             # SIMD-optimized math library
│   ├── Utils/            # Enhanced logging system & utilities
│   ├── Renderer/         # Rendering system
│   ├── Input/            # Input handling
│   ├── Scene/            # Scene management (legacy)
│   ├── Audio/            # Audio system
│   └── Physics/          # Physics system
├── Examples/             # Example projects
│   └── BasicGame/        # Basic game example
├── Tools/                # Development tools
│   └── AssetProcessor/   # Asset processing tools
├── Tests/                # Test projects
│   ├── Unit/            # Unit tests
│   └── Integration/     # Integration tests
├── vendor/              # Third-party dependencies
│   └── glad/            # OpenGL loader
└── docs/                # Documentation
```

## Dependencies

- GLAD: OpenGL loader
- Windows API: Window management (Windows platform)
- CMake: Build system
- Visual Studio 2022: Development environment

## Build System

The project uses CMake for build configuration:

- Minimum CMake version: 3.16.0
- Multi-configuration support
- Proper dependency management
- Installation rules

## Platform Support

Currently supported platforms:

- Windows 10/11 (primary)

Planned platform support:

- Linux
- macOS

## Graphics Pipeline

The graphics pipeline is designed to be API-agnostic:

1. **Graphics Device Layer**
   - IGraphicsDevice interface
   - API-specific implementations (OpenGL, DirectX)
   - Resource management
   - State management

2. **Shader System**
   - Abstract shader interface
   - GLSL support (OpenGL)
   - HLSL support (planned for DirectX)
   - Shader compilation and linking
   - Uniform/constant buffer management

3. **Buffer Management**
   - Vertex buffer abstraction
   - Index buffer support (planned)
   - Constant/uniform buffer support (planned)
   - Dynamic buffer updates

4. **Rendering**
   - Geometry management
   - Material system (planned)
   - Texture management (planned)
   - Render state management (planned)

## Math Library Architecture

The Pyramid Engine features a comprehensive, SIMD-optimized math library that forms the foundation for all 3D operations:

### Core Math Components

- **Vector Classes**: Vec2, Vec3, Vec4 with full operator support
- **Matrix Classes**: Mat3, Mat4 for transformations and projections
- **Quaternion**: Quat for rotations and orientation
- **Math Utilities**: Constants, functions, and geometric operations
- **SIMD Optimizations**: Runtime CPU feature detection and optimization

### SIMD Optimization Strategy

- **Runtime Detection**: Automatic detection of CPU capabilities (SSE, SSE2, SSE3, SSE4.1, AVX, FMA)
- **Fallback Paths**: Scalar implementations for systems without SIMD support
- **Batch Operations**: SIMD-optimized processing of arrays of mathematical objects
- **Memory Alignment**: 16-byte aligned structures for optimal SIMD performance

### Key Math Features

- **Comprehensive Operations**: All standard 3D math operations with operator overloading
- **Performance Optimizations**: Fast inverse square root, trigonometric approximations
- **Geometric Utilities**: Ray-sphere intersection, frustum culling, spatial operations
- **Transformation Utilities**: TRS (Translate, Rotate, Scale) matrix creation
- **Camera Utilities**: Perspective and orthographic projection matrices

### Usage Examples

```cpp
using namespace Pyramid::Math;

// Vector operations
Vec3 position(1.0f, 2.0f, 3.0f);
Vec3 direction = Vec3::Forward;
Vec3 newPosition = position + direction * 5.0f;

// Matrix transformations
Mat4 translation = Mat4::CreateTranslation(position);
Mat4 rotation = Mat4::CreateRotationY(Radians(45.0f));
Mat4 scale = Mat4::CreateScale(2.0f);
Mat4 transform = translation * rotation * scale;

// Camera matrices
Mat4 view = Mat4::CreateLookAt(Vec3(0, 0, 5), Vec3::Zero, Vec3::Up);
Mat4 projection = Mat4::CreatePerspective(Radians(60.0f), 16.0f/9.0f, 0.1f, 1000.0f);
Mat4 mvp = projection * view * transform;
```

## Thread Safety and Performance

### Logging System Thread Safety

The enhanced logging system is designed for multi-threaded game engines:

- **Mutex Protection**: All logging operations are protected by a single mutex
- **Deadlock Prevention**: Local buffer usage prevents recursive locking
- **Race Condition Elimination**: Configuration access is properly synchronized
- **Performance Optimization**: Early exit checks minimize lock contention

### Math Library Performance

- **SIMD Acceleration**: Automatic utilization of available CPU features
- **Memory Layout**: Structure of Arrays (SoA) for batch operations
- **Cache Optimization**: Cache-friendly data access patterns
- **Branch Reduction**: Minimized branching in critical paths

### Graphics System Performance

- **Command Buffer Batching**: Reduced API call overhead through batching
- **State Management**: Minimal state changes and efficient tracking
- **Resource Management**: Efficient buffer and texture management
- **Visibility Culling**: Frustum and occlusion culling for performance

### Memory Management

- **RAII Principles**: Automatic resource management through smart pointers
- **Memory Pools**: Custom allocators for frequently allocated objects
- **Alignment**: Proper memory alignment for SIMD operations
- **Leak Detection**: Comprehensive memory leak detection and reporting

### Performance Considerations

- **Early Exit**: Log level filtering occurs before expensive operations
- **Local Buffers**: Message formatting uses stack-allocated buffers
- **Minimal Overhead**: Release builds can disable debug/trace logging entirely
- **File I/O Optimization**: Buffered file writing with configurable flush intervals
- **SIMD Utilization**: Automatic detection and use of available CPU features
- **Cache-Friendly Design**: Data structures optimized for cache performance

## Future Development

Planned features and improvements:

- Complete math library
- Physics system
- Audio system
- Scene graph
- Asset management
- Input system
- Networking
- Scripting support
- Remote logging capabilities
- Log analysis and visualization tools
