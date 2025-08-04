# Pyramid Graphics Device API Reference

## Overview

The Pyramid Graphics Device provides a unified abstraction layer for multiple graphics APIs, enabling developers to write graphics code once and run it across different platforms and graphics backends. Currently supporting OpenGL 3.3-4.6 with plans for DirectX, Vulkan, and Metal support.

## Key Features

- **Multi-API Support**: Unified interface across different graphics APIs
- **Resource Management**: Comprehensive buffer, texture, and shader management
- **State Management**: Efficient graphics state changes with minimal overhead
- **Command Buffer System**: Efficient GPU command batching and submission
- **Modern Features**: Support for modern graphics features like UBOs, instanced rendering
- **Performance Focused**: Optimized for minimal API call overhead

## Architecture

The graphics system follows a layered architecture:

```
Application Layer
├── Game Logic
├── Scene Management
└── Resource Management

Graphics Abstraction Layer
├── IGraphicsDevice (Interface)
├── GraphicsDevice (Implementation)
└── Resource Factories

Platform Implementation Layer
├── OpenGLDevice (Current)
├── DirectXDevice (Planned)
├── VulkanDevice (Planned)
└── MetalDevice (Planned)
```

## Quick Start

```cpp
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Camera.hpp>

using namespace Pyramid;

// Create graphics device (usually done by Game class)
auto graphicsDevice = IGraphicsDevice::Create(GraphicsAPI::OpenGL, window);

// Create resources
auto vertexBuffer = graphicsDevice->CreateVertexBuffer();
auto indexBuffer = graphicsDevice->CreateIndexBuffer();
auto shader = graphicsDevice->CreateShader();
auto texture = graphicsDevice->CreateTexture2D("texture.png");

// Set up vertex buffer
f32 vertices[] = {
    // Position         // UV coords
   -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
    0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
    0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
   -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
};
vertexBuffer->setData(vertices, sizeof(vertices));

// Main render loop
void Render() {
    graphicsDevice->Clear(Color(0.1f, 0.1f, 0.1f, 1.0f));
    
    // Set up rendering state
    shader->bind();
    vertexBuffer->bind();
    indexBuffer->bind();
    texture->bind(0);
    
    // Draw
    graphicsDevice->DrawIndexed(6);
    
    graphicsDevice->Present(true);
}
```

## IGraphicsDevice Interface

The `IGraphicsDevice` interface provides the core abstraction for all graphics operations.

### Core Methods

#### Device Management
```cpp
virtual bool Initialize() = 0;
virtual void Shutdown() = 0;
```

#### Rendering Operations
```cpp
virtual void Clear(const Color &color) = 0;
virtual void Present(bool vsync) = 0;
virtual void DrawIndexed(u32 count) = 0;
virtual void DrawIndexedInstanced(u32 indexCount, u32 instanceCount) = 0;
virtual void DrawArraysInstanced(u32 vertexCount, u32 instanceCount, u32 firstVertex = 0) = 0;
```

#### Viewport and Scissor
```cpp
virtual void SetViewport(u32 x, u32 y, u32 width, u32 height) = 0;
```

### Resource Creation

#### Buffer Management
```cpp
virtual std::shared_ptr<IVertexBuffer> CreateVertexBuffer() = 0;
virtual std::shared_ptr<IIndexBuffer> CreateIndexBuffer() = 0;
virtual std::shared_ptr<IVertexArray> CreateVertexArray() = 0;
virtual std::shared_ptr<IUniformBuffer> CreateUniformBuffer(size_t size, BufferUsage usage) = 0;
virtual std::shared_ptr<IInstanceBuffer> CreateInstanceBuffer() = 0;
virtual std::shared_ptr<IShaderStorageBuffer> CreateShaderStorageBuffer() = 0;
```

#### Shader Management
```cpp
virtual std::shared_ptr<IShader> CreateShader() = 0;
```

#### Texture Management
```cpp
virtual std::shared_ptr<ITexture2D> CreateTexture2D(const TextureSpecification &specification, const void *data = nullptr) = 0;
virtual std::shared_ptr<ITexture2D> CreateTexture2D(const std::string &filepath, bool srgb = false, bool generateMips = true) = 0;
```

### State Management

#### Blending
```cpp
virtual void EnableBlend(bool enable) = 0;
virtual void SetBlendFunc(u32 sfactor, u32 dfactor) = 0;
```

#### Depth Testing
```cpp
virtual void EnableDepthTest(bool enable) = 0;
virtual void SetDepthFunc(u32 func) = 0;
```

#### Face Culling
```cpp
virtual void EnableCullFace(bool enable) = 0;
virtual void SetCullFace(u32 mode) = 0;
```

#### Clear Color
```cpp
virtual void SetClearColor(f32 r, f32 g, f32 b, f32 a) = 0;
```

### Performance Monitoring
```cpp
virtual u32 GetStateChangeCount() const = 0;
virtual void ResetStateChangeCount() = 0;
```

## Resource Types

### Vertex Buffers (IVertexBuffer)

Vertex buffers store vertex data for rendering operations.

#### Usage Example
```cpp
// Create vertex buffer
auto vertexBuffer = graphicsDevice->CreateVertexBuffer();

// Define vertex data
struct Vertex {
    Vec3 position;
    Vec2 texCoord;
    Vec3 normal;
};

std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
};

// Set buffer data
vertexBuffer->setData(vertices.data(), vertices.size() * sizeof(Vertex));

// Set buffer layout
BufferLayout layout = {
    {ShaderDataType::Float3, "a_Position"},
    {ShaderDataType::Float2, "a_TexCoord"},
    {ShaderDataType::Float3, "a_Normal"}
};
vertexBuffer->setLayout(layout);

// Bind for rendering
vertexBuffer->bind();
```

### Index Buffers (IIndexBuffer)

Index buffers store indices for indexed rendering operations.

#### Usage Example
```cpp
// Create index buffer
auto indexBuffer = graphicsDevice->CreateIndexBuffer();

// Define index data
std::vector<u32> indices = {
    0, 1, 2,  // First triangle
    0, 2, 3   // Second triangle
};

// Set buffer data
indexBuffer->setData(indices.data(), indices.size() * sizeof(u32));

// Bind and draw
indexBuffer->bind();
graphicsDevice->DrawIndexed(indices.size());
```

### Vertex Arrays (IVertexArray)

Vertex arrays encapsulate vertex buffer and index buffer configurations.

#### Usage Example
```cpp
// Create vertex array
auto vertexArray = graphicsDevice->CreateVertexArray();

// Set up vertex buffers
auto vertexBuffer = graphicsDevice->CreateVertexBuffer();
auto indexBuffer = graphicsDevice->CreateIndexBuffer();

// Configure vertex array
vertexArray->addVertexBuffer(vertexBuffer);
vertexArray->setIndexBuffer(indexBuffer);

// Bind for rendering (binds all associated buffers)
vertexArray->bind();
graphicsDevice->DrawIndexed(indexBuffer->getCount());
```

### Shaders (IShader)

Shaders provide programmable graphics pipeline stages.

#### Usage Example
```cpp
// Create shader
auto shader = graphicsDevice->CreateShader();

// Set shader sources
const char* vertexSource = R"(
    #version 330 core
    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec2 a_TexCoord;
    
    uniform mat4 u_ViewProjection;
    uniform mat4 u_WorldMatrix;
    
    out vec2 v_TexCoord;
    
    void main() {
        gl_Position = u_ViewProjection * u_WorldMatrix * vec4(a_Position, 1.0);
        v_TexCoord = a_TexCoord;
    }
)";

const char* fragmentSource = R"(
    #version 330 core
    in vec2 v_TexCoord;
    
    uniform sampler2D u_Texture;
    uniform vec4 u_Color;
    
    out vec4 o_Color;
    
    void main() {
        vec4 texColor = texture(u_Texture, v_TexCoord);
        o_Color = texColor * u_Color;
    }
)";

shader->setVertexSource(vertexSource);
shader->setFragmentSource(fragmentSource);

// Compile shader
if (!shader->compile()) {
    PYRAMID_LOG_ERROR("Shader compilation failed: ", shader->getErrorMessage());
    return;
}

// Set uniforms
shader->bind();
shader->setUniform("u_ViewProjection", viewProjectionMatrix);
shader->setUniform("u_WorldMatrix", worldMatrix);
shader->setUniform("u_Color", Vec4(1.0f, 1.0f, 1.0f, 1.0f));
shader->setUniform("u_Texture", 0);  // Texture unit 0
```

### Textures (ITexture2D)

Textures store image data for use in shaders.

#### Usage Example
```cpp
// Create texture from file
auto texture = graphicsDevice->CreateTexture2D("textures/brick.png", false, true);

// Create texture from data
TextureSpecification spec;
spec.width = 512;
spec.height = 512;
spec.format = TextureFormat::RGBA8;
spec.wrapS = TextureWrap::Repeat;
spec.wrapT = TextureWrap::Repeat;
spec.minFilter = TextureFilter::LinearMipmapLinear;
spec.magFilter = TextureFilter::Linear;

std::vector<u8> pixelData(512 * 512 * 4, 255);  // White texture
auto texture = graphicsDevice->CreateTexture2D(spec, pixelData.data());

// Use texture in rendering
texture->bind(0);  // Bind to texture unit 0
shader->setUniform("u_Texture", 0);
```

### Uniform Buffers (IUniformBuffer)

Uniform buffers provide efficient way to pass uniform data to shaders.

#### Usage Example
```cpp
// Define uniform data structure
struct CameraUniforms {
    Mat4 viewProjection;
    Mat4 view;
    Mat4 projection;
    Vec3 position;
    f32 padding;
};

// Create uniform buffer
auto uniformBuffer = graphicsDevice->CreateUniformBuffer(sizeof(CameraUniforms), BufferUsage::Dynamic);

// Update uniform data
CameraUniforms uniforms;
uniforms.viewProjection = camera.GetViewProjectionMatrix();
uniforms.view = camera.GetViewMatrix();
uniforms.projection = camera.GetProjectionMatrix();
uniforms.position = camera.GetPosition();

uniformBuffer->setData(&uniforms, sizeof(CameraUniforms));

// Bind to shader binding point
uniformBuffer->bind(0);

// Access in shader
/*
#version 330 core
layout(std140, binding = 0) uniform CameraBlock {
    mat4 viewProjection;
    mat4 view;
    mat4 projection;
    vec3 position;
};
*/
```

## Camera System

The Camera class provides advanced camera functionality with projection and view matrix calculations.

### Camera Types
```cpp
enum class ProjectionType {
    Perspective,
    Orthographic
};
```

### Usage Example
```cpp
// Create perspective camera
Camera camera(Math::Radians(60.0f), 16.0f/9.0f, 0.1f, 1000.0f);

// Set camera transform
camera.SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
camera.LookAt(Math::Vec3::Zero);

// Camera movement
camera.MoveForward(1.0f);      // Move forward
camera.MoveRight(0.5f);        // Move right
camera.MoveUp(-0.2f);          // Move down
camera.Rotate(0.1f, 0.2f);     // Rotate camera

// Get matrices for rendering
const Math::Mat4& viewMatrix = camera.GetViewMatrix();
const Math::Mat4& projectionMatrix = camera.GetProjectionMatrix();
const Math::Mat4& viewProjectionMatrix = camera.GetViewProjectionMatrix();

// Frustum culling
Math::Vec3 objectPosition(10.0f, 5.0f, 0.0f);
f32 objectRadius = 2.0f;
bool isVisible = camera.IsSphereVisible(objectPosition, objectRadius);

// Screen/world coordinate conversion
f32 screenX, screenY;
if (camera.WorldToScreen(worldPos, screenX, screenY)) {
    // Object is in front of camera
}

Math::Vec3 rayOrigin, rayDirection;
camera.ScreenToWorldRay(0.5f, 0.5f, rayOrigin, rayDirection);  // Center of screen
```

## Scene Management

The scene management system provides efficient organization and rendering of scene objects.

### SceneManager

#### Usage Example
```cpp
#include <Pyramid/Graphics/Scene/SceneManager.hpp>

using namespace Pyramid::SceneManagement;

// Create scene manager
auto sceneManager = SceneUtils::CreateSceneManager();

// Create and configure scene
auto scene = sceneManager->CreateScene("MainScene");
sceneManager->SetActiveScene(scene);

// Configure spatial partitioning
sceneManager->SetOctreeDepth(8);
sceneManager->SetOctreeSize(Math::Vec3(1000.0f, 1000.0f, 1000.0f));
sceneManager->RebuildSpatialPartition();

// Spatial queries
Math::Vec3 playerPos = GetPlayerPosition();
auto nearbyObjects = sceneManager->GetObjectsInRadius(playerPos, 10.0f);
auto nearestObject = sceneManager->GetNearestObject(playerPos);
auto visibleObjects = sceneManager->GetVisibleObjects(camera);

// Performance monitoring
const SceneStats& stats = sceneManager->GetStats();
PYRAMID_LOG_INFO("Scene objects: ", stats.totalObjects, ", visible: ", stats.visibleObjects);
```

## Rendering Pipeline

The rendering pipeline follows a structured approach for efficient rendering:

### 1. Begin Frame
```cpp
graphicsDevice->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
graphicsDevice->Clear(Color(0.1f, 0.1f, 0.1f, 1.0f));
```

### 2. Set Up State
```cpp
// Enable depth testing and blending
graphicsDevice->EnableDepthTest(true);
graphicsDevice->SetDepthFunc(GL_LESS);
graphicsDevice->EnableBlend(true);
graphicsDevice->SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// Set viewport
graphicsDevice->SetViewport(0, 0, windowWidth, windowHeight);
```

### 3. Update Global State
```cpp
// Update camera matrices
shader->bind();
shader->setUniform("u_ViewProjection", camera.GetViewProjectionMatrix());
shader->setUniform("u_CameraPosition", camera.GetPosition());
```

### 4. Render Objects
```cpp
for (auto& renderObject : visibleObjects) {
    // Bind object resources
    renderObject->shader->bind();
    renderObject->vertexArray->bind();
    renderObject->texture->bind(0);
    
    // Set object-specific uniforms
    renderObject->shader->setUniform("u_WorldMatrix", renderObject->worldMatrix);
    renderObject->shader->setUniform("u_Color", renderObject->color);
    
    // Draw
    graphicsDevice->DrawIndexed(renderObject->indexBuffer->getCount());
}
```

### 5. End Frame
```cpp
graphicsDevice->Present(true);
```

## Performance Optimization

### State Management
```cpp
// Monitor state changes
u32 stateChanges = graphicsDevice->GetStateChangeCount();
PYRAMID_LOG_INFO("State changes this frame: ", stateChanges);
graphicsDevice->ResetStateChangeCount();

// Minimize state changes by sorting objects
std::sort(renderObjects.begin(), renderObjects.end(), 
    [](const auto& a, const auto& b) {
        // Sort by shader, then by texture to minimize state changes
        if (a->shader != b->shader) return a->shader < b->shader;
        return a->texture < b->texture;
    });
```

### Batch Rendering
```cpp
// Use instanced rendering for similar objects
graphicsDevice->DrawIndexedInstanced(indexCount, instanceCount);

// Use uniform buffers for bulk data transfer
uniformBuffer->setData(data, dataSize);
```

### Resource Management
```cpp
// Reuse resources when possible
static auto vertexBuffer = graphicsDevice->CreateVertexBuffer();
static auto shader = graphicsDevice->CreateShader();

// Clean up unused resources
if (!resource->isUsed()) {
    resource.reset();
}
```

## Error Handling

The graphics system provides comprehensive error handling:

```cpp
// Check device initialization
if (!graphicsDevice->Initialize()) {
    PYRAMID_LOG_ERROR("Failed to initialize graphics device");
    return false;
}

// Check shader compilation
if (!shader->compile()) {
    PYRAMID_LOG_ERROR("Shader compilation failed: ", shader->getErrorMessage());
    return false;
}

// Check resource creation
auto texture = graphicsDevice->CreateTexture2D("texture.png");
if (!texture) {
    PYRAMID_LOG_ERROR("Failed to create texture");
    return false;
}

// Handle graphics API errors
GLenum error;
while ((error = glGetError()) != GL_NO_ERROR) {
    PYRAMID_LOG_ERROR("OpenGL error: ", error);
}
```

## Best Practices

### Performance
1. **Minimize State Changes**: Sort objects by shader and texture to reduce state changes
2. **Use Batch Rendering**: Combine similar objects into single draw calls when possible
3. **Optimize Buffer Usage**: Use appropriate buffer usage patterns (Static, Dynamic, Stream)
4. **Leverage Spatial Partitioning**: Use scene management for efficient culling
5. **Profile and Monitor**: Use performance monitoring to identify bottlenecks

### Resource Management
1. **Reuse Resources**: Share shaders, textures, and buffers across objects
2. **Clean Up**: Properly release resources when no longer needed
3. **Memory Alignment**: Ensure proper alignment for optimal performance
4. **Asynchronous Loading**: Consider loading resources asynchronously

### Rendering Quality
1. **Proper State Management**: Ensure correct render state for each object
2. **Depth Testing**: Use appropriate depth testing for 3D scenes
3. **Alpha Blending**: Configure blending correctly for transparent objects
4. **Mipmapping**: Generate mipmaps for better texture quality and performance

## Integration with Math Library

The graphics system integrates seamlessly with the math library:

```cpp
// Math types in graphics operations
Math::Mat4 worldMatrix = Math::Transform::CreateTRS(position, rotation, scale);
Math::Mat4 mvp = camera.GetViewProjectionMatrix() * worldMatrix;

// Math operations for rendering
Math::Vec3 lightDir = (lightPosition - position).Normalized();
f32 distance = Math::Vec3::Distance(camera.GetPosition(), objectPosition);

// Frustum culling with math operations
bool isVisible = camera.IsSphereVisible(objectPosition, objectRadius);
```

## Conclusion

The Pyramid Graphics Device provides a comprehensive, high-performance graphics abstraction layer that enables developers to create modern graphics applications with ease. With support for multiple graphics APIs, efficient resource management, and seamless integration with the math library, it offers both flexibility and performance for a wide range of graphics applications.