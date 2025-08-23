# Pyramid Engine API Reference

## Overview

This section contains detailed API documentation for all Pyramid Engine modules. Each module is documented with comprehensive examples, usage patterns, and integration guidelines.

## Core Engine

### [Game](Core/Game.md)
The main game engine class that manages the game loop, graphics device, and provides the foundation for all Pyramid Engine applications.

**Key Features:**
- Game loop management
- Graphics device initialization
- Virtual methods for game logic
- Cross-platform abstractions

## Graphics System

### [Graphics Device](Graphics/GraphicsDevice.md)
Low-level graphics device interface providing hardware abstraction and rendering capabilities.

**Key Features:**
- Multi-API support (OpenGL 3.3-4.6, DirectX planned)
- Buffer management
- Shader compilation and management
- Rendering commands

### [Render System](Graphics/RenderSystem.md)
High-level rendering system that provides command buffers, render passes, and efficient rendering pipeline management.

**Key Features:**
- Command buffer system
- Multiple render pass types
- Performance optimization
- Modern rendering techniques

### [Camera](Graphics/Camera.md)
Advanced camera system supporting multiple projection types, frustum culling, and smooth camera controls.

**Key Features:**
- Perspective and orthographic projections
- View matrix calculations
- Frustum culling support
- Flexible camera controls

### [Scene Manager](Graphics/SceneManager.md)
Production-ready scene management system with spatial partitioning and efficient object queries.

**Key Features:**
- Octree spatial partitioning
- Multiple query types (point, sphere, box, frustum, ray)
- Performance monitoring
- Scene serialization support

### [Texture System](Graphics/Texture.md)
Comprehensive texture management with support for multiple image formats through custom loaders.

**Key Features:**
- Multiple format support (TGA, BMP, PNG, JPEG)
- Zero external dependencies
- Efficient memory management
- GPU texture creation and management

## Math Library

### [Math Library](Math/MathLibrary.md)
SIMD-optimized mathematical library providing high-performance vector, matrix, and quaternion operations.

**Key Features:**
- SIMD acceleration (SSE, AVX)
- Runtime CPU feature detection
- Comprehensive 3D math operations
- Cache-friendly data structures

## Platform Abstraction

### [Window System](Platform/Window.md)
Cross-platform window management with OpenGL context creation and event handling.

**Key Features:**
- Native window management
- OpenGL context handling
- Event processing
- Multi-monitor support

## Utilities and Services

### [Logging System](Utils/Logging.md)
Production-ready, thread-safe logging system with multiple output formats and configurable log levels.

**Key Features:**
- Thread-safe operations
- Multiple log levels
- File rotation
- Structured logging
- Performance optimized

### [Image Loading](Utils/ImageLoading.md)
Comprehensive image loading system supporting multiple formats with zero external dependencies.

**Key Features:**
- TGA, BMP, PNG, JPEG support
- Custom format implementations
- Efficient memory usage
- Integration with texture system

## Planned Systems

The following systems are currently in development and will be documented as they are implemented:

### [Audio System](Audio/AudioSystem.md)
3D spatial audio system with format support and effects processing.

**Planned Features:**
- 3D spatial audio
- Multiple format support
- Real-time effects
- Streaming audio support

### [Physics System](Physics/PhysicsSystem.md)
Realistic physics simulation with rigid body dynamics and collision detection.

**Planned Features:**
- Rigid body dynamics
- Collision detection and response
- Constraints and joints
- Character controller

### [Input System](Input/InputSystem.md)
Comprehensive input handling for keyboard, mouse, and gamepad devices.

**Planned Features:**
- Multi-device support
- Action-based input mapping
- Event-driven architecture
- Hot-plugging support

## Module Status

| Module | Status | Documentation | Examples |
|--------|--------|---------------|----------|
| Core | ✅ Complete | ✅ Complete | ✅ Available |
| Graphics | ✅ Complete | ✅ Complete | ✅ Available |
| Math | ✅ Complete | ✅ Complete | ✅ Available |
| Platform | ✅ Complete | ✅ Complete | ✅ Available |
| Utils | ✅ Complete | ✅ Complete | ✅ Available |
| Audio | 🔄 In Development | ✅ Planned API | ⏳ Planned |
| Physics | 🔄 In Development | ✅ Planned API | ⏳ Planned |
| Input | 🔄 In Development | ✅ Planned API | ⏳ Planned |

## Usage Patterns

### Basic Engine Initialization

```cpp
#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Graphics/GraphicsDevice.hpp>

class MyGame : public Pyramid::Game {
public:
    void onCreate() override {
        // Initialize game resources
        auto* device = GetGraphicsDevice();
        // ... setup code
    }
    
    void onUpdate(float deltaTime) override {
        // Update game logic
    }
    
    void onRender() override {
        // Render frame
    }
};

int main() {
    MyGame game;
    game.run();
    return 0;
}
```

### Advanced Rendering Setup

```cpp
#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>
#include <Pyramid/Graphics/Scene/SceneManager.hpp>

// Initialize rendering system
auto renderSystem = std::make_unique<RenderSystem>();
renderSystem->Initialize(graphicsDevice);

// Setup scene management
auto sceneManager = SceneUtils::CreateSceneManager();
sceneManager->EnableSpatialPartitioning(true);

// Render loop
renderSystem->BeginFrame();
renderSystem->Render(*scene, camera);
renderSystem->EndFrame();
```

### Math Library Usage

```cpp
#include <Pyramid/Math/Math.hpp>

using namespace Pyramid::Math;

// SIMD-optimized operations
Vec3 position(1.0f, 2.0f, 3.0f);
Vec3 velocity = Vec3::Forward * speed;
Vec3 newPosition = position + velocity * deltaTime;

// Matrix transformations
Mat4 world = Mat4::CreateTranslation(position) *
             Mat4::CreateRotationY(rotation) *
             Mat4::CreateScale(scale);
```

## Integration Examples

The API modules are designed to work seamlessly together:

```cpp
// Complete integration example
class AdvancedGame : public Pyramid::Game {
private:
    std::unique_ptr<RenderSystem> m_renderSystem;
    std::unique_ptr<SceneManager> m_sceneManager;
    Camera m_camera;
    
public:
    void onCreate() override {
        // Initialize all systems
        m_renderSystem = std::make_unique<RenderSystem>();
        m_renderSystem->Initialize(GetGraphicsDevice());
        
        m_sceneManager = SceneUtils::CreateSceneManager();
        m_camera = Camera(Math::Radians(60.0f), 16.0f/9.0f, 0.1f, 1000.0f);
        
        // Setup scene
        auto scene = m_sceneManager->CreateScene("MainLevel");
        m_sceneManager->SetActiveScene(scene);
    }
    
    void onUpdate(float deltaTime) override {
        // Update scene
        m_sceneManager->Update(deltaTime);
        
        // Update camera
        UpdateCamera(deltaTime);
    }
    
    void onRender() override {
        // Render with all systems
        m_renderSystem->BeginFrame();
        m_renderSystem->Render(*m_sceneManager->GetActiveScene(), m_camera);
        m_renderSystem->EndFrame();
    }
};
```

## Performance Considerations

### SIMD Optimization
All math operations automatically use SIMD when available:
```cpp
// Automatically uses SSE/AVX when supported
Vec3 result = Vec3::Cross(a, b);
Mat4 mvp = projection * view * model;
```

### Memory Management
Smart pointer usage throughout the API:
```cpp
auto texture = ITexture2D::Create("texture.png");
auto shader = device->CreateShader();
// Automatic cleanup when objects go out of scope
```

### Efficient Rendering
Command buffer system for optimal performance:
```cpp
renderSystem->BeginFrame();  // Start command recording
// Commands are batched automatically
renderSystem->EndFrame();   // Submit all commands
```

## Error Handling

Comprehensive error handling across all modules:
```cpp
if (!device->Initialize()) {
    PYRAMID_LOG_ERROR("Graphics device initialization failed");
    return false;
}

auto texture = ITexture2D::Create("missing.png");
if (!texture) {
    PYRAMID_LOG_WARN("Using default texture fallback");
    texture = GetDefaultTexture();
}
```

## Next Steps

1. **Start with Core**: Begin with the [Game](Core/Game.md) class documentation
2. **Learn Graphics**: Progress to [Graphics Device](Graphics/GraphicsDevice.md) and [Render System](Graphics/RenderSystem.md)
3. **Master Math**: Study the [Math Library](Math/MathLibrary.md) for performance optimization
4. **Explore Advanced Features**: Scene management, spatial partitioning, and optimization techniques

## Support and Community

- **GitHub Issues**: Report bugs and request features
- **Discussions**: Ask questions and share knowledge
- **Examples**: See the [Examples](../Examples/) directory for complete working code
- **Contributing**: Read the [Contributing Guide](../Contributing.md) to get involved

The Pyramid Engine API is designed to be powerful yet approachable. Start with the basics and gradually explore the advanced features as your projects grow in complexity.
