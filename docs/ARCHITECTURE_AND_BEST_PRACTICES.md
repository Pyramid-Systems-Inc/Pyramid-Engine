# Pyramid Engine - Architecture and Best Practices

> **Version:** 0.6.0 | **Architecture:** Component-based with OpenGL 4.6 rendering backend

## Navigation

- Previous: [Implementation Roadmap](IMPLEMENTATION_ROADMAP.md)
- See also: [Critical Issues & Blockers](CRITICAL_ISSUES_AND_BLOCKERS.md)
- Back to: [Master Roadmap](../PYRAMID_ENGINE_MASTER_ROADMAP.md)

---

## Engine Architecture Overview

### High-Level Component Structure

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                        │
├─────────────────────────────────────────────────────────────┤
│  Core Engine  │  Scene Management  │  Rendering System      │
├───────────────┼────────────────────┼────────────────────────┤
│  Graphics API │  Resource Mgmt     │  Platform Abstraction  │
├───────────────┴────────────────────┴────────────────────────┤
│                    OpenGL 3.3-4.6 Backend                    │
└─────────────────────────────────────────────────────────────┘
```

### Namespace Organization

```cpp
Pyramid::Core::     // Core engine systems (Window, Game, etc.)
Pyramid::Graphics:: // Rendering, shaders, graphics API abstraction
Pyramid::Math::     // SIMD-optimized mathematical operations
Pyramid::Util::     // Utilities (Image loading, logging, etc.)
Pyramid::Scene::    // Scene management, transforms, cameras
Pyramid::Input::    // Input handling and event processing
Pyramid::Audio::    // Audio system and 3D spatial audio
Pyramid::Physics::  // Physics integration and collision detection
```

### Component Diagram

- **Graphics Device:** API-agnostic interface with OpenGL implementation
- **State Manager:** Singleton caching all OpenGL state to minimize redundant calls
- **Resource System:** RAII-based buffer, texture, shader, framebuffer management
- **Render System:** Command buffer architecture with multi-pass rendering framework
- **Scene Graph:** Hierarchical transforms with spatial partitioning (Octree)
- **Camera System:** Multiple projections with frustum culling

---

## Completed Systems ✅

### Math Library - SIMD-Optimized (100% Complete)

- **Runtime CPU Detection:** Automatic SSE/AVX feature detection with graceful fallbacks
- **Comprehensive Types:** Vec2, Vec3, Vec4, Mat3, Mat4, Quaternion classes
- **Batch Operations:** SIMD-optimized array processing for vectors/matrices
- **Geometric Queries:** Ray-sphere intersection, frustum culling, spatial transforms
- **Performance:** Fast inverse square root, trigonometric approximations
- **Quality:** Thread-safe, cache-friendly, extensively tested

### Image Loading Library - Zero Dependencies (100% Complete)

- **PNG Support:** Complete DEFLATE decompression (RFC 1951), all filter types, custom ZLIB with Adler-32
- **JPEG Support:** Baseline DCT, Huffman decoder, dequantization, IDCT, YCbCr→RGB conversion
- **BMP Support:** Windows Bitmap with padding and BGR conversion
- **TGA Support:** Uncompressed RGB/RGBA with orientation handling
- **Architecture:** Self-contained, no stb_image dependency

### Enhanced Rendering System (85% Complete)

- **Command Buffer:** Efficient GPU command submission and batching
- **Render Pass Framework:** Forward, deferred, shadow, post-processing stages
- **PBR Material System:** Metallic-roughness workflow
- **Camera System:** Perspective/orthographic with frustum culling
- **Scene Graph:** Hierarchical transforms with lazy updates

### Scene Management Core (100% Complete)

- **SceneManager:** Comprehensive lifecycle management
- **Octree Spatial Partitioning:** O(log n) queries vs O(n) brute-force
- **AABB Implementation:** Intersection testing and collision detection
- **Performance Monitoring:** Real-time statistics and profiling
- **Memory Management:** Smart pointer integration with reference counting

### Production Logging System (100% Complete)

- **Thread Safety:** Multi-threaded logging with synchronization
- **Multiple Outputs:** Console, file rotation, structured logging
- **Performance:** Minimal overhead in production builds
- **Source Location:** File, line, function tracking

---

## OpenGL 4.6 Implementation

### Project Structure

```
Engine/Graphics/
├── include/Pyramid/Graphics/
│   ├── GraphicsDevice.hpp          # Core abstraction interface
│   ├── Buffer/                      # VBO, IBO, VAO, UBO, SSBO
│   ├── Shader/                      # Shader system
│   ├── Renderer/                    # RenderSystem, RenderPasses
│   ├── Scene/                       # SceneManager, Octree
│   └── OpenGL/                      # OpenGL implementations
│       ├── OpenGLDevice.hpp
│       ├── OpenGLStateManager.hpp
│       └── Buffer/, Shader/
```

### Context Creation Strategy

**Target:** OpenGL 4.6 Core Profile with fallback chain:
4.6 → 4.5 → 4.4 → 4.3 → 4.2 → 4.1 → 4.0 → 3.3

**Method:** Two-stage initialization via [`Win32OpenGLWindow.cpp:192`](../Engine/Platform/source/Windows/Win32OpenGLWindow.cpp:192)

1. Temporary legacy context to load WGL extensions
2. Modern context with WGL_ARB_create_context_profile
3. Forward-compatible bit set, deprecated features disabled

### OpenGL Feature Utilization

**OpenGL 3.3 (Baseline):**

- ✅ Core Profile (no fixed-function pipeline)
- ✅ Vertex Array Objects (VAO)
- ✅ Uniform Buffer Objects (UBO)
- ✅ Instanced Rendering
- ✅ Multiple Render Targets (MRT)
- ✅ Framebuffer Objects (FBO)

**OpenGL 4.0:**

- ✅ Tessellation Shaders (control + evaluation)
- ✅ Geometry Shaders

**OpenGL 4.3:**

- ✅ Compute Shaders with dispatch
- ✅ Shader Storage Buffers (SSBO)
- ✅ Multi-Draw Indirect

**OpenGL 4.5:**

- ⚠️ Direct State Access (DSA) - mentioned but not fully utilized

**OpenGL 4.6:**

- ✅ SPIR-V shaders (capability logged)
- ✅ Anisotropic filtering (standard support)

### State Manager Design

**Pattern:** Singleton with comprehensive caching via [`OpenGLStateManager`](../Engine/Graphics/include/Pyramid/Graphics/OpenGL/OpenGLStateManager.hpp:14)

**Cached State Categories:**

- Shader program binding
- VAO, VBO, IBO bindings
- UBO, SSBO bindings
- 32 texture units × multiple targets
- Framebuffer (draw/read)
- Viewport, scissor
- Blend, depth, stencil, culling states
- Clear colors and values

**Performance Benefits:**

- Redundant call elimination
- State change counting/tracking
- Can invalidate and reset
- Initializes from current GL state

**Current Issues:**

- ❌ Shaders bypass via direct [`glUseProgram()`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:24)
- ❌ Framebuffer cache inconsistency (GL_FRAMEBUFFER vs GL_DRAW_FRAMEBUFFER)
- ❌ Some device operations bypass state manager
- ⚠️ Missing polygon mode, color mask, multisample tracking

---

## Core Systems

### Shader System

**Architecture:** [`OpenGLShader`](../Engine/Graphics/include/Pyramid/Graphics/OpenGL/Shader/OpenGLShader.hpp:13)

**Multi-Stage Support:**

- Vertex + Fragment (basic pipeline)
- [`CompileWithGeometry()`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:299)
- [`CompileWithTessellation()`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:355)
- [`CompileCompute()`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:522)
- [`CompileAdvanced()`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:424) - full pipeline

**Uniform Management:**

- Location caching via hash map
- Type-safe setters (int, float, vec2/3/4, mat3/4)
- Auto-bind on uniform set (causes redundant calls)
- ⚠️ Missing: Array uniforms, math type integration, batching

**Block Binding:**

- UBO via [`SetUniformBlockBinding()`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:277)
- SSBO via [`SetShaderStorageBlockBinding()`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:589)
- ✅ Good error handling with block index validation
- ❌ Missing: Block index caching, centralized binding point registry

**Compute Shader Support:**

- [`DispatchCompute()`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:557) with automatic memory barrier
- ❌ Missing: Work group size validation, indirect dispatch

**Missing Features:**

- ❌ Shader reflection (auto-discovery of uniforms/attributes)
- ❌ Shader includes/shared code
- ❌ Shader variants/macros
- ❌ Binary shader caching
- ❌ Hot-reload capability

### Resource Management

**RAII Compliance:** ✅ **EXCELLENT**

All resources follow proper acquisition/release:

- Constructors generate GL objects
- Destructors delete GL objects with zero-checks
- Exception-safe cleanup paths
- Smart pointer integration throughout

**Examples:**

- [`OpenGLVertexBuffer`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLVertexBuffer.cpp:5)
- [`OpenGLShader`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:11)
- [`OpenGLFramebuffer`](../Engine/Graphics/source/OpenGL/OpenGLFramebuffer.cpp:24) - outstanding with full attachment cleanup

**Critical Gaps:**

- ❌ **Move Semantics Missing:** No move constructors/assignment operators
- ❌ Copy should be explicitly deleted (currently implicit)
- ❌ Cannot efficiently store in STL containers
- ❌ No resource handle system with generation counters

**Resource Lifecycle:**

```
Application
    ↓
IGraphicsDevice::CreateXXX()
    ↓
OpenGLDevice::CreateXXX()
    ↓
std::make_shared<OpenGLXXX>()
    ↓
Constructor: glGenXXX()
    ↓
Returns std::shared_ptr
```

### Buffer System

**Buffer Hierarchy:**

```
IVertexBuffer / IIndexBuffer / IVertexArray
    ↓ implements
OpenGLVertexBuffer / OpenGLIndexBuffer / OpenGLVertexArray
```

**Quality Assessment:**

| Buffer Type | SetData | UpdateData | Map/Unmap | Orphaning | Grade |
|-------------|---------|------------|-----------|-----------|-------|
| VertexBuffer | ✅ | ❌ | ❌ | ❌ | **D** |
| IndexBuffer | ✅ | ❌ | ❌ | ❌ | **D** |
| UniformBuffer | ✅ | ✅ | ✅ | ✅ | **A+** |
| SSBO | ✅ | ✅ | ✅ | ❌ | **B+** |

**UniformBuffer Excellence:**

- [`Initialize()`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLUniformBuffer.cpp:68) with usage hints
- [`UpdateData()`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLUniformBuffer.cpp:113) with bounds checking
- [`Map()`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLUniformBuffer.cpp:162)/[`Unmap()`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLUniformBuffer.cpp:192) support
- [`SetPersistentMapping()`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLUniformBuffer.cpp:359) for GL 4.4+
- [`Resize()`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLUniformBuffer.cpp:285) with data preservation
- [`InvalidateData()`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLUniformBuffer.cpp:346) for orphaning
- [`UBOStateManager`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLUniformBuffer.cpp:11) prevents redundant binds

**VBO/IBO Need Improvement:**

- Hardcoded GL_STATIC_DRAW (no usage hints)
- No partial updates
- No mapping support
- No orphaning strategy
- Should match UBO quality

### Rendering Pipeline

**Command Buffer Architecture:**

Recording/playback separation via [`CommandBuffer`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderSystem.hpp:71)

**Command Types:**

- SetRenderTarget, SetShader, SetTexture, SetUniformBuffer
- DrawIndexed, Dispatch, ClearTarget

**Current State:**

- ✅ Recording API functional
- ⚠️ Execution has placeholder TODOs at [`Execute()`](../Engine/Graphics/source/Renderer/RenderSystem.cpp:116)
- ❌ Missing device binding APIs (BindShader, BindTexture, BindUBO, BindVAO)
- ❌ Resource ID→pointer resolution not implemented

**Render Pass Framework:**

Base class: [`RenderPass`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderSystem.hpp:147)

Declared passes (implementations pending):

- [`ForwardRenderPass`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderPasses.hpp:14)
- [`DeferredGeometryPass`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderPasses.hpp:36)
- [`DeferredLightingPass`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderPasses.hpp:60)
- [`ShadowMapPass`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderPasses.hpp:82)
- [`TransparentPass`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderPasses.hpp:114)
- [`PostProcessPass`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderPasses.hpp:140)
- [`UIRenderPass`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderPasses.hpp:191)
- [`DebugRenderPass`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderPasses.hpp:216)

**Integration Points:**

- Scene queries: [`GetVisibleObjects()`](../Engine/Graphics/source/Scene.cpp:224), [`GetVisibleLights()`](../Engine/Graphics/source/Scene.cpp:243)
- SceneManager octree (not yet integrated with RenderSystem)
- Material binding (struct exists, GPU hookup missing)

---

## Design Patterns in Use

### 1. Abstract Factory Pattern

**Implementation:** [`IGraphicsDevice::Create()`](../Engine/Graphics/include/Pyramid/Graphics/GraphicsDevice.hpp:250)

- API-agnostic device creation
- Supports future DirectX/Vulkan backends
- Factory pattern for textures, shaders, buffers

### 2. Singleton Pattern

**Implementation:** [`OpenGLStateManager::GetInstance()`](../Engine/Graphics/include/Pyramid/Graphics/OpenGL/OpenGLStateManager.hpp:17)

- Single source of truth for GL state
- Thread-local if needed for multi-context
- **Caution:** Global state, testing complexity

### 3. Command Pattern

**Implementation:** [`CommandBuffer`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderSystem.hpp:71)

- Recording → Execution separation
- Deferred rendering support
- Batch optimization opportunities

### 4. Template Method Pattern

**Implementation:** [`RenderPass`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderSystem.hpp:147)

- Begin/Execute/End lifecycle
- Derived classes override behavior
- Framework controls flow

### 5. RAII (Resource Acquisition Is Initialization)

**Implementation:** All resource classes

- Constructors acquire GL objects
- Destructors release GL objects
- Smart pointers throughout
- Exception-safe cleanup

### 6. Observer Pattern (Partial)

**Implementation:** Transform dirty flags in [`SceneNode`](../Engine/Graphics/source/Scene.cpp:119)

- Lazy matrix updates
- Parent→child propagation
- Could extend to full observer for events

---

## System Integration Strategy

### Unified Initialization Sequence

**Integration Points:** Window ↔ Graphics Device ↔ Input System ↔ GUI System

**Startup Flow:**

1. **Window Creation:** Platform-specific window with desired API support
2. **Graphics Device:** API-specific device creation with window context
3. **Input System:** Window-integrated input event processing
4. **GUI System:** Graphics device-dependent UI rendering
5. **Resource Manager:** Shared texture, shader, and buffer management

### Cross-System Communication

**Event System Integration:**

- Unified event dispatcher for type-safe routing
- **Priority Chain:** GUI → Game Logic → System Events
- Window events (resize, focus, DPI) propagate to all systems
- Performance events (frame timing, memory, API state)

**State Management:**

- Graphics state caching minimizes redundant API calls
- Memory pool integration for shared resources
- Configuration management unified across systems

### Error Handling and Recovery

**Graceful Degradation:**

- API fallback system: OpenGL 4.6 → 4.5 → ... → 3.3
- Device lost recovery: Save state, release resources, attempt restore
- Resource recovery: Automatic restoration after device recovery
- Performance monitoring: Real-time detection of issues

---

## Core Architectural Principles

### 1. Performance First

- SIMD-optimized math operations
- State change minimization via caching
- Spatial partitioning (Octree)
- Frustum culling
- Command batching

### 2. Type Safety

- Strong typing throughout
- Enum classes over integers
- Smart pointers over raw pointers
- Const-correctness enforced

### 3. Platform Abstraction

- Clean interfaces (IGraphicsDevice, IWindow)
- Platform-specific implementations isolated
- Ready for Linux, macOS expansion

### 4. Modern C++ Standards

- C++17 minimum
- Smart pointers (`std::shared_ptr`, `std::unique_ptr`)
- Move semantics (needs completion)
- Structured bindings, `std::optional`, `std::variant`

### 5. Modular Design

- Clean separation of concerns
- Well-defined interfaces
- Minimal coupling between subsystems

### 6. Self-Contained

- Custom image loaders (no stb_image)
- Custom DEFLATE/JPEG decoders
- Only external dependency: GLAD (GL loader)

---

## Best Practices Assessment

### Currently Following ✅

- [x] **Core Profile:** No deprecated OpenGL features
- [x] **VAOs for all geometry:** Modern vertex specification
- [x] **UBOs for shared uniforms:** Camera, lighting data
- [x] **Command batching:** Via CommandBuffer system
- [x] **State caching:** OpenGLStateManager reduces redundant calls
- [x] **RAII cleanup:** All resources cleaned in destructors
- [x] **Shader error handling:** Comprehensive info logs
- [x] **FBO completeness checks:** Validation before use
- [x] **Instanced rendering:** Supported via divisors
- [x] **Smart pointers:** `shared_ptr`/`unique_ptr` throughout

### Not Following (Should Implement) ❌

- [ ] **Direct State Access (DSA):** GL 4.5+ for cleaner code
  - Replace `glBindBuffer` + `glBufferData` → `glNamedBufferData`
  - Replace `glBindTexture` + `glTexImage2D` → `glTextureStorage2D`
  - Replace `glBindFramebuffer` + `glFramebufferTexture2D` → `glNamedFramebufferTexture`

- [ ] **Centralized GL error checking:** Currently ad-hoc
  - Implement `GL_CALL` macro for debug builds
  - Enable `glDebugMessageCallback` for automatic reporting

- [ ] **Integer vertex attributes:** Using `glVertexAttribPointer` instead of `glVertexAttribIPointer`
  - **CRITICAL BUG** at [`OpenGLVertexArray.cpp:83`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLVertexArray.cpp:83)

- [ ] **Pixel storage control:** Missing `GL_UNPACK_ALIGNMENT`/`GL_PACK_ALIGNMENT`
  - Set before texture uploads/downloads

- [ ] **Immutable texture storage:** Using mutable `glTexImage2D`
  - Use `glTexStorage2D` for better driver optimization

- [ ] **Sampler objects:** Texture state coupled with data
  - Use `glGenSamplers` to separate sampling state

- [ ] **Shader introspection:** No reflection system
  - Query active uniforms, attributes, blocks
  - Enable auto-binding and validation

- [ ] **Synchronization primitives:** No fences or barriers
  - Use `glFenceSync` for CPU-GPU sync
  - Use `glMemoryBarrier` for compute/SSBO

- [ ] **Debug labels on all resources:** Only FBO has them
  - Use `glObjectLabel` for buffers, textures, shaders

- [ ] **Query hardware limits:** Some hardcoded (32 texture units)
  - Query `GL_MAX_*` values at initialization

### Partially Following (Needs Improvement) ⚠️

- [~] **State manager usage:** Shaders and some device calls bypass it
  - Route ALL state changes through StateManager
  - Fix shader [`glUseProgram()`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:24) direct calls

- [~] **Buffer usage hints:** Only UBO/SSBO; VBO/IBO hardcoded
  - Add usage parameter to all buffer types
  - Map to `GL_STATIC_DRAW`/`GL_DYNAMIC_DRAW`/`GL_STREAM_DRAW`

- [~] **Resource tracking:** No central registry
  - Implement resource lifetime tracking
  - Enable leak detection in debug builds

- [~] **Framebuffer state caching:** Inconsistent target handling
  - Fix `GL_FRAMEBUFFER` vs `GL_DRAW_FRAMEBUFFER`/`GL_READ_FRAMEBUFFER`

---

## Recommended Architectural Improvements

### 1. Resource Handle System

Replace raw `u32` IDs with type-safe handles:

```cpp
template<typename T>
struct Handle {
    u32 index;
    u32 generation;  // Detect use-after-free
    
    bool IsValid() const { return index != INVALID_INDEX; }
};

using ShaderHandle = Handle<class ShaderTag>;
using TextureHandle = Handle<class TextureTag>;
using BufferHandle = Handle<class BufferTag>;
```

**Benefits:** Type safety, generation-based validation, prevents stale handle usage

### 2. Modern C++20 Features

**Concepts for template constraints:**

```cpp
template<typename T>
concept GraphicsResource = requires(T t) {
    { t.GetID() } -> std::convertible_to<u32>;
    { t.Bind() } -> std::same_as<void>;
};
```

**Ranges for cleaner code:**

```cpp
auto visibleObjects = scene->GetAllObjects() 
    | std::views::filter([&](auto& obj) { return camera->IsVisible(obj); });
```

### 3. Context-Based Resource Creation

**Problem:** Singleton StateManager, global state

**Better:** Context-based state manager

```cpp
class RenderContext {
    OpenGLStateManager stateManager;
    // ... other context state ...
};

thread_local RenderContext* g_currentContext = nullptr;
```

**Benefits:** Thread-safe, testable, multi-context support

### 4. Enhanced Error Handling

**Current:** Sporadic `glGetError()` checks

**Improved:**

```cpp
#ifdef PYRAMID_DEBUG
    #define GL_CALL(x) do { x; CheckGLError(#x, __FILE__, __LINE__); } while(0)
#else
    #define GL_CALL(x) x
#endif

void InitializeGLDebug() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugCallback, nullptr);
}
```

### 5. Shader Reflection System

**Auto-discovery of shader resources:**

```cpp
class ShaderReflection {
    struct UniformInfo { std::string name; GLenum type; GLint location; };
    struct AttributeInfo { std::string name; GLenum type; GLint location; };
    struct UniformBlockInfo { std::string name; GLuint index; GLint size; };
    
    static ShaderReflection Reflect(GLuint program);
};
```

**Benefits:** Eliminate manual uniform management, enable validation, support hot-reload

### 6. Centralized Binding Point Registry

```cpp
namespace BindingPoints {
    // UBO Binding Points
    constexpr u32 CAMERA_UBO = 0;
    constexpr u32 MATERIAL_UBO = 1;
    constexpr u32 LIGHTS_UBO = 2;
    
    // SSBO Binding Points
    constexpr u32 PARTICLES_SSBO = 0;
    constexpr u32 INSTANCE_DATA_SSBO = 1;
    
    // Texture Units
    constexpr u32 ALBEDO_TEXTURE = 0;
    constexpr u32 NORMAL_TEXTURE = 1;
    // ...
}
```

---

## Modern C++ Practices to Adopt

### 1. std::span for Array Views

**Instead of:**

```cpp
void UpdateData(const void* data, u32 size);
```

**Use:**

```cpp
void UpdateData(std::span<const std::byte> data);
```

### 2. Constexpr Where Possible

```cpp
constexpr u32 CalculateStride() const { /* ... */ }
constexpr bool IsInteger() const { return type >= Int && type <= Int4; }
```

### 3. Move Semantics (CRITICAL)

**Add to all resource classes:**

```cpp
OpenGLVertexBuffer(OpenGLVertexBuffer&& other) noexcept
    : m_bufferId(other.m_bufferId), m_size(other.m_size) {
    other.m_bufferId = 0;
    other.m_size = 0;
}

OpenGLVertexBuffer& operator=(OpenGLVertexBuffer&& other) noexcept {
    if (this != &other) {
        if (m_bufferId) glDeleteBuffers(1, &m_bufferId);
        m_bufferId = other.m_bufferId;
        m_size = other.m_size;
        other.m_bufferId = 0;
        other.m_size = 0;
    }
    return *this;
}
```

### 4. Explicit Copy Deletion

```cpp
ClassName(const ClassName&) = delete;
ClassName& operator=(const ClassName&) = delete;
```

### 5. Smart Pointer Strategy

- `std::unique_ptr` for exclusive ownership
- `std::shared_ptr` for shared resources (textures, shaders)
- `std::weak_ptr` for parent-child relationships (scene graph)
- Avoid raw pointers except for non-owning references

### 6. Template Metaprogramming

**Type traits for buffer layouts:**

```cpp
template<typename T>
struct is_vertex_type : std::false_type {};

template<> struct is_vertex_type<Vec3> : std::true_type {};
template<> struct is_vertex_type<Vec4> : std::true_type {};
```

---

## Performance Considerations

### Current Optimizations

1. **State Change Minimization:** OpenGLStateManager caches all state
2. **Uniform Location Caching:** Hash map in OpenGLShader
3. **Command Batching:** CommandBuffer pre-allocation (1024 commands)
4. **Frustum Culling:** Camera-based visibility tests
5. **Spatial Partitioning:** Octree for O(log n) queries
6. **Transform Caching:** Dirty flags avoid redundant matrix calculations
7. **SIMD Math:** Runtime CPU feature detection (SSE/AVX)
8. **Persistent Mapping:** GL 4.4+ for UBOs (zero-copy updates)

### Optimization Opportunities

1. **Render Queue Sorting:** Sort by shader → material → mesh
2. **Material Batching:** Minimize texture/shader switches
3. **Buffer Sub-allocation:** Ring buffer allocator for dynamic data
4. **Texture Atlasing:** Combine small textures
5. **Resource Pooling:** Reuse buffers/textures
6. **GPU-Driven Rendering:** Multi-draw indirect for large scenes
7. **Async Resource Loading:** Separate thread for I/O
8. **Shader Binary Caching:** Reduce startup compilation time

---

## Code Quality Standards

### Naming Conventions

- **Classes:** PascalCase (`OpenGLDevice`, `SceneManager`)
- **Functions/Methods:** PascalCase (`Initialize()`, `GetVisibleObjects()`)
- **Variables:** camelCase (`m_bufferId`, `vertexCount`)
- **Constants:** UPPER_SNAKE_CASE (`MAX_TEXTURE_UNITS`)
- **Namespaces:** PascalCase (`Pyramid::Graphics`)

### Documentation Requirements

- Public API fully documented with comments
- Complex algorithms explained
- File headers with purpose/responsibility
- Example usage for non-trivial classes

### Error Handling

- Check return values from GL calls (improve coverage)
- Log errors with severity levels (ERROR, WARN, INFO)
- Graceful degradation where possible
- Exception-safe RAII cleanup

### Code Organization

- Interface (`.hpp`) separate from implementation (`.cpp`)
- One class per file (generally)
- Platform-specific code isolated
- Clean dependency graph (no circular dependencies)

### Testing Standards

- Unit tests for math library (complete)
- Integration tests for rendering scenarios (needed)
- Visual regression tests for rendering (needed)
- Performance benchmarks (needed)

---

**Last Updated:** 2025-11-01  
**Document Version:** 1.0  
**Status:** Consolidated from 3 source documents

**See Also:**

- [Critical Issues & Blockers](CRITICAL_ISSUES_AND_BLOCKERS.md) - Issues preventing basic rendering
- [Implementation Roadmap](IMPLEMENTATION_ROADMAP.md) - Phased development plan
- [Master Roadmap](../PYRAMID_ENGINE_MASTER_ROADMAP.md) - Complete project roadmap
