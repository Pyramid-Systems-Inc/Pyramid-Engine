# Render System API Reference

## Overview

The Pyramid Engine's Render System provides a high-level rendering abstraction that manages the complete rendering pipeline. It implements modern rendering techniques including command buffers, render passes, and efficient resource management.

## Classes

### RenderSystem

The main rendering system class that orchestrates all rendering operations.

#### Constructor
```cpp
RenderSystem();
```

#### Public Methods

##### Initialize
```cpp
void Initialize(IGraphicsDevice* device);
```
Initializes the render system with the specified graphics device.

**Parameters:**
- `device`: Pointer to the graphics device to use for rendering

##### BeginFrame
```cpp
void BeginFrame();
```
Begins a new rendering frame. Must be called before any rendering operations.

##### EndFrame
```cpp
void EndFrame();
```
Ends the current rendering frame and presents the results to the screen.

##### Render
```cpp
void Render(const Scene& scene, const Camera& camera);
```
Renders a scene from the specified camera's perspective.

**Parameters:**
- `scene`: The scene to render
- `camera`: The camera to render from

##### SetRenderPass
```cpp
void SetRenderPass(RenderPassType type);
```
Sets the current render pass for subsequent rendering operations.

**Parameters:**
- `type`: The type of render pass (Forward, Deferred, Shadow, etc.)

#### Protected Methods

##### InitializeCommandBuffers
```cpp
void InitializeCommandBuffers();
```
Initializes the command buffer system for efficient GPU command submission.

##### InitializeRenderPasses
```cpp
void InitializeRenderPasses();
```
Sets up the various render passes used by the system.

## Render Passes

The render system supports multiple render pass types:

### RenderPassType Enumeration

```cpp
enum class RenderPassType {
    Forward,        // Forward rendering pass
    Deferred,       // Deferred rendering pass
    Shadow,         // Shadow mapping pass
    PostProcess,    // Post-processing pass
    UI              // User interface rendering pass
};
```

### Forward Rendering Pass
- Traditional immediate-mode rendering
- Suitable for transparent objects and simple scenes
- Lower memory usage but higher fill rate requirements

### Deferred Rendering Pass
- Geometry buffer (G-buffer) based rendering
- Efficient for complex lighting scenarios
- Higher memory usage but consistent performance

### Shadow Mapping Pass
- Generates shadow maps for dynamic lighting
- Supports multiple shadow map resolutions
- Cascade shadow mapping for directional lights

### Post-Processing Pass
- Screen-space effects and tone mapping
- Bloom, SSAO, and other effects
- Frame buffer manipulation

## Usage Examples

### Basic Rendering Setup
```cpp
#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>

// Initialize render system
auto renderSystem = std::make_unique<RenderSystem>();
renderSystem->Initialize(graphicsDevice);

// Setup camera
Camera camera(Math::Radians(60.0f), 16.0f/9.0f, 0.1f, 1000.0f);
camera.SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
camera.LookAt(Math::Vec3::Zero);

// Create scene
auto scene = SceneUtils::CreateTestScene();

// Render loop
while (running) {
    renderSystem->BeginFrame();
    renderSystem->Render(*scene, camera);
    renderSystem->EndFrame();
}
```

### Multi-Pass Rendering
```cpp
// Shadow pass
renderSystem->BeginFrame();
renderSystem->SetRenderPass(RenderPassType::Shadow);
renderSystem->Render(*scene, shadowCamera);

// Main rendering pass
renderSystem->SetRenderPass(RenderPassType::Forward);
renderSystem->Render(*scene, mainCamera);

// Post-processing pass
renderSystem->SetRenderPass(RenderPassType::PostProcess);
renderSystem->ApplyPostProcessing();

renderSystem->EndFrame();
```

### Command Buffer Usage
```cpp
// The render system automatically manages command buffers
// for efficient GPU command submission
renderSystem->BeginFrame();  // Starts command recording
// ... rendering operations are batched into command buffers
renderSystem->EndFrame();    // Submits all recorded commands
```

## Performance Considerations

### Command Buffer Batching
- Commands are automatically batched for optimal GPU utilization
- State changes are minimized through intelligent sorting
- Draw calls are merged when possible

### Memory Management
- Efficient resource pooling for temporary rendering resources
- Automatic garbage collection of unused resources
- Smart caching of frequently used render states

### GPU Synchronization
- Minimal CPU-GPU synchronization points
- Efficient use of GPU memory bandwidth
- Optimized for modern graphics APIs

## Integration with Graphics System

The render system seamlessly integrates with other graphics components:

```cpp
// Works with the camera system
Camera camera = scene.GetActiveCamera();
renderSystem->Render(*scene, camera);

// Integrates with the material system
Material material = object.GetMaterial();
renderSystem->ApplyMaterial(material);

// Uses the scene management system
auto visibleObjects = scene.GetVisibleObjects(camera);
renderSystem->RenderObjects(visibleObjects);
```

## Error Handling

The render system provides comprehensive error checking:

```cpp
if (!renderSystem->Initialize(device)) {
    PYRAMID_LOG_ERROR("Failed to initialize render system");
    return false;
}

// Automatic validation of render state
renderSystem->ValidateRenderState();  // Debug builds only
```

## Future Extensions

The render system is designed for extensibility:

- **Custom Render Passes**: Add application-specific rendering passes
- **Plugin Architecture**: Load rendering plugins at runtime
- **Multi-API Support**: Abstract rendering across different graphics APIs
- **Compute Shaders**: Integration with compute-based rendering techniques

## See Also

- [Graphics Device API](GraphicsDevice.md) - Low-level graphics operations
- [Camera API](Camera.md) - Camera system documentation
- [Scene Management](SceneManager.md) - Scene organization and culling
- [Shader System](../Shaders/ShaderSystem.md) - Shader management and compilation
