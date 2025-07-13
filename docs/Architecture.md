# Pyramid Game Engine Architecture

## Overview

Pyramid is a modern, multi-platform game engine designed with flexibility and performance in mind. The engine supports multiple graphics APIs and provides a clean abstraction layer for rendering.

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

The graphics system is built around a flexible abstraction layer that supports multiple graphics APIs:

- **IGraphicsDevice**: Core interface for all graphics implementations
  - OpenGL (3.3 - 4.6)
  - DirectX 9 (planned)
  - DirectX 10 (planned)
  - DirectX 11 (planned)
  - DirectX 12 (planned)

#### Graphics Features

- Multiple graphics API support
- Version detection and fallback
- Automatic feature detection
- Unified resource management
- Shader system
- Vertex buffer abstraction

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

## Thread Safety and Performance

### Logging System Thread Safety

The enhanced logging system is designed for multi-threaded game engines:

- **Mutex Protection**: All logging operations are protected by a single mutex
- **Deadlock Prevention**: Local buffer usage prevents recursive locking
- **Race Condition Elimination**: Configuration access is properly synchronized
- **Performance Optimization**: Early exit checks minimize lock contention

### Performance Considerations

- **Early Exit**: Log level filtering occurs before expensive operations
- **Local Buffers**: Message formatting uses stack-allocated buffers
- **Minimal Overhead**: Release builds can disable debug/trace logging entirely
- **File I/O Optimization**: Buffered file writing with configurable flush intervals

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
