# Pyramid Engine - Implementation Roadmap

> **Timeline:** Q3 2025 - Q4 2026 | **Current Focus:** Phase 0-1 (OpenGL Core & Forward Rendering)

## Navigation

- **Previous:** [Critical Issues & Blockers](CRITICAL_ISSUES_AND_BLOCKERS.md)
- **See also:** [Architecture & Best Practices](ARCHITECTURE_AND_BEST_PRACTICES.md) _(coming soon)_

---

## Overview

This roadmap outlines the **6-8 week path to production-ready rendering** followed by extended engine development through Q4 2026. The critical path (Phases 0-4) focuses on unblocking basic rendering and building a robust forward rendering pipeline. Subsequent phases add scene management, input, GUI, physics, and audio systems.

**Phased Approach:**

- **Phase 0-4** (6-8 weeks): Core rendering from triangle to production-quality forward/deferred pipeline
- **Phase 5-7** (Q4 2025 - Q1 2026): Scene, input, and GUI systems
- **Phase 8-10** (Q2-Q4 2026): Physics, audio, asset pipeline

**Current Status:** Advanced Math Library ✅ | Image Loading ✅ | Scene Core ✅ | **Rendering Pipeline 🔴 BLOCKED**

---

## Phase 0: Critical Fixes ⚡

**Duration:** 3-5 days | **Effort:** 22 hours  
**Goal:** Render a basic colored triangle through RenderSystem  
**Deliverable:** Working command buffer execution with indexed geometry rendering

### Tasks (Priority Order)

1. **Fix Integer Vertex Attributes** (2h)
   - File: [`OpenGLVertexArray.cpp:80`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLVertexArray.cpp:80)
   - Use `glVertexAttribIPointer` for integer types
   - Handle Mat3/Mat4 column decomposition

2. **Fix State Manager Cache** (2h)
   - File: [`OpenGLStateManager.cpp:173`](../Engine/Graphics/source/OpenGL/OpenGLStateManager.cpp:173)
   - Handle `GL_FRAMEBUFFER` aliasing for draw/read targets

3. **Integrate Shader with State Manager** (4h)
   - File: [`OpenGLShader.cpp:22`](../Engine/Graphics/source/OpenGL/Shader/OpenGLShader.cpp:22)
   - Route `glUseProgram()` through state manager
   - Remove redundant per-uniform `Bind()` calls

4. **Add Forward to RenderPassType** (15min)
   - File: [`RenderSystem.hpp:26`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderSystem.hpp:26)

5. **Add Device Binding APIs** (6h)
   - Implement `BindShader()`, `BindVertexArray()`, `BindTexture()`, `BindUniformBuffer()`
   - Files: [`GraphicsDevice.hpp`](../Engine/Graphics/include/Pyramid/Graphics/GraphicsDevice.hpp), [`OpenGLDevice.cpp`](../Engine/Graphics/source/OpenGL/OpenGLDevice.cpp)

6. **Implement Command Buffer Execution** (8h)
   - File: [`RenderSystem.cpp:116`](../Engine/Graphics/source/Renderer/RenderSystem.cpp:116)
   - Implement SetRenderTarget, SetShader, SetTexture, SetUniformBuffer
   - Add resource resolution (ID → pointer mapping)

### Success Criteria

✅ Triangle renders with correct vertex colors  
✅ State manager eliminates redundant GL calls  
✅ Command buffer executes all binding commands  

---

## Phase 1: Basic Rendering 🎨

**Duration:** 1-2 weeks | **Effort:** ~35 hours  
**Goal:** Render textured meshes with basic diffuse lighting  
**Deliverable:** Lit, textured 3D model with depth testing and camera control

### Tasks

1. **Fix Texture/Depth Issues** (5h)
   - Texture unpack alignment: [`OpenGLTexture.cpp:147`](../Engine/Graphics/source/OpenGL/OpenGLTexture.cpp:147)
   - Mipmap filter consistency: [`OpenGLTexture.cpp:172`](../Engine/Graphics/source/OpenGL/OpenGLTexture.cpp:172)
   - Depth clear mask safety: [`OpenGLDevice.cpp:76`](../Engine/Graphics/source/OpenGL/OpenGLDevice.cpp:76)

2. **Implement UpdateGlobalUniforms** (4h)
   - File: [`RenderSystem.cpp:343`](../Engine/Graphics/source/Renderer/RenderSystem.cpp:343)
   - Map camera UBO and write view/projection matrices
   - Map lighting UBO and write light data

3. **Create ForwardRenderPass** (16h)
   - Create implementation file `ForwardRenderPass.cpp`
   - Begin: Set render target, clear color/depth
   - Execute: Pull visible objects, bind VAO/shader/textures, record draws
   - End: Cleanup and transition
   - Implement `RenderPassFactory::CreateForwardPass()`

4. **Implement SetupDefaultRenderPasses** (2h)
   - File: [`RenderSystem.cpp:336`](../Engine/Graphics/source/Renderer/RenderSystem.cpp:336)
   - Create and configure forward pass

5. **Create Test Scene** (4h)
   - Textured cube mesh with UVs
   - Basic lighting shader (Phong or PBR)
   - Directional light
   - Camera with movement controls

6. **Fix Multisample FBO** (4h)
   - File: [`OpenGLFramebuffer.cpp:621`](../Engine/Graphics/source/OpenGL/OpenGLFramebuffer.cpp:621)
   - Use `GL_TEXTURE_2D_MULTISAMPLE` for MSAA attachments

### Success Criteria

✅ Textured cube with correct UVs and mipmaps  
✅ Depth testing prevents Z-fighting  
✅ Basic diffuse lighting visible  
✅ Camera movement updates view smoothly  
✅ Multiple objects render with different materials  

---

## Phase 2: Forward Pipeline 💡

**Duration:** 2-3 weeks | **Effort:** ~79 hours  
**Goal:** Production-quality forward renderer with shadows and materials  
**Deliverable:** Complex multi-light scene with cascaded shadow maps

### Tasks

1. **Shader Reflection System** (8h)
   - Auto-discover uniforms via `glGetActiveUniform()`
   - Auto-discover UBO/SSBO blocks
   - Cache block indices, populate uniform cache

2. **Binding Point Registry** (4h)
   - Define standard binding points for UBOs/SSBOs/Textures
   - Runtime validation, hardware limit queries

3. **Material System** (16h)
   - Material struct → shader uniform mapping
   - Default textures for missing slots
   - Material sorting for batching

4. **VBO/IBO Usage Hints** (3h)
   - Add `BufferUsage` parameter to `SetData()`
   - Map Static/Dynamic/Stream to GL hints

5. **Shadow Map Pass** (20h)
   - Depth-only FBO creation
   - Cascade split calculation
   - Render shadow casters from light's view
   - Store cascade matrices in UBO
   - Sample shadow map in forward shader

6. **Post-Process Pass** (16h)
   - Fullscreen quad mesh
   - Tone mapping shader (Reinhard, ACES)
   - Gamma correction
   - FXAA implementation

7. **GL Debug Output** (4h)
   - Initialize `glDebugMessageCallback`
   - Create `GL_CALL` macro for debug builds

8. **Framebuffer Improvements** (8h)
   - Apply per-attachment sampler state
   - Remove auto-viewport on bind
   - Fix multisample attachment target

### Success Criteria

✅ Shadows render with cascaded splits  
✅ HDR rendering with tone mapping  
✅ Gamma-correct output  
✅ Multiple materials batched efficiently  
✅ GL errors caught and logged in debug builds  
✅ 100+ objects at 60 FPS  

---

## Phase 3: Advanced Features 🚀

**Duration:** 2-3 weeks | **Effort:** ~92 hours  
**Goal:** Deferred rendering and advanced post-processing  
**Deliverable:** High-quality rendering with bloom, SSAO, transparent objects

### Tasks

1. **Deferred Geometry Pass** (16h)
   - G-Buffer FBO with 4 MRTs (Albedo+Metallic, Normal+Roughness, Motion+AO, Emissive+Flags)
   - Pack material properties efficiently
   - Render scene to G-Buffer

2. **Deferred Lighting Pass** (16h)
   - Fullscreen quad rendering
   - Unpack G-Buffer in shader
   - Accumulate lighting (directional, point, spot)
   - Integrate shadow maps

3. **Transparent Pass** (12h)
   - Sort objects back-to-front
   - Configure blending modes
   - Optional: Order-independent transparency

4. **Enhanced Post-Processing** (16h)
   - Bloom: Downsample → blur → upsample
   - SSAO: Screen-space ambient occlusion
   - Optional: SSR, TAA

5. **UI Render Pass** (12h)
   - Immediate-mode or retained batching
   - Font atlas rendering
   - Scissor/clipping support

6. **Debug Render Pass** (8h)
   - Line rendering (bounding boxes, gizmos)
   - G-Buffer visualization modes
   - Overdraw heatmap

7. **Shader Hot-Reload** (12h)
   - File watching system
   - Recompile on change
   - Preserve uniform state across reload

### Success Criteria

✅ Deferred rendering matches forward quality  
✅ Bloom adds glow to bright areas  
✅ SSAO enhances depth perception  
✅ Transparent objects blend correctly  
✅ UI renders on top without depth conflicts  
✅ Shaders reload without engine restart  

---

## Phase 4: Production Polish ✨

**Duration:** 1-2 weeks | **Effort:** ~79 hours  
**Goal:** Optimization, profiling, and developer tools  
**Deliverable:** Production-ready engine with zero resource leaks

### Tasks

1. **Add Move Semantics** (6h)
   - Implement for all buffer types, shaders, textures, FBOs
   - Enable efficient container storage

2. **Resource Tracking System** (16h)
   - Central resource registry
   - Debug labels for all GL objects (`glObjectLabel`)
   - Leak detection in debug builds
   - Memory usage tracking

3. **Enhanced Buffer System** (12h)
   - Add `UpdateData()` to VBO/IBO
   - Implement `Map()`/`Unmap()`
   - Orphaning strategy for dynamic buffers
   - Persistent mapping (GL 4.4+)

4. **Binary Shader Caching** (12h)
   - Save compiled shaders to disk
   - Load from cache on startup
   - Hash-based invalidation

5. **SSBO Binding Cache** (3h)
   - Prevent redundant SSBO binds
   - Match UBO state manager quality

6. **Extend State Manager** (6h)
   - Track polygon mode, color mask, multisample
   - Ensure all state goes through manager

7. **Performance Profiling** (8h)
   - GPU timer queries (`glQueryCounter`)
   - Per-pass timing breakdown
   - Performance HUD overlay
   - RenderDoc integration helpers

8. **Render Queue Optimization** (12h)
   - Sort by shader → material → mesh
   - Minimize state changes
   - Measure and validate improvements

9. **Comprehensive Testing** (16h)
   - Unit tests for critical components
   - Integration tests (triangle, textured quad, FBO)
   - Visual regression tests
   - Performance benchmarks

### Success Criteria

✅ Zero resource leaks in 1-hour stress test  
✅ Shader compilation 10x faster with cache  
✅ State changes reduced by 50% vs naïve  
✅ All resources have debug labels  
✅ Per-pass GPU timing available  
✅ Performance matches industry standards  

---

## Quick Wins (Parallel Work) ⚡

High-impact improvements that can be done independently:

| Task | Effort | Impact | File |
|------|--------|--------|------|
| 1. Add Forward enum value | 15min | 🟢 High | [`RenderSystem.hpp:26`](../Engine/Graphics/include/Pyramid/Graphics/Renderer/RenderSystem.hpp:26) |
| 2. Enable GL Debug Output | 2h | 🟢 High | [`OpenGLDevice.cpp:30`](../Engine/Graphics/source/OpenGL/OpenGLDevice.cpp:30) |
| 3. Fix Texture Unpack Alignment | 1h | 🟢 High | [`OpenGLTexture.cpp:147`](../Engine/Graphics/source/OpenGL/OpenGLTexture.cpp:147) |
| 4. Remove FBO Auto-Viewport | 1h | 🟢 Medium | [`OpenGLFramebuffer.cpp:130`](../Engine/Graphics/source/OpenGL/OpenGLFramebuffer.cpp:130) |
| 5. Add Explicit Copy Delete | 30min | 🟢 Medium | All resource classes |
| 6. Query Max Texture Units | 1h | 🟢 Medium | [`OpenGLStateManager.cpp:17`](../Engine/Graphics/source/OpenGL/OpenGLStateManager.cpp:17) |
| 7. Fix Depth Clear Mask | 2h | 🟢 High | [`OpenGLDevice.cpp:76`](../Engine/Graphics/source/OpenGL/OpenGLDevice.cpp:76) |
| 8. Fix UBO Resize Logging | 30min | 🟡 Low | [`OpenGLUniformBuffer.cpp:285`](../Engine/Graphics/source/OpenGL/Buffer/OpenGLUniformBuffer.cpp:285) |

---

## Future Phases (Q4 2025 - Q4 2026)

### Phase 5: Scene Management System (Q4 2025)

**Duration:** 1 month | **35 specific tasks**

- Transform System: Quaternion rotation, dirty-flag optimization, hierarchy
- Scene Graph: Traversal algorithms, node management, serialization
- Camera System: Perspective/orthographic, frustum culling, controls
- Integration: MVP matrix calculation, multi-camera support

### Phase 6: Input System Enhancement (Q4 2025)

**Duration:** 3-4 weeks

- **Keyboard**: State management, key repeat, modifiers, Unicode
- **Mouse**: Button tracking, movement delta, wheel, cursor control
- **Input Events**: Callbacks, queuing, filtering, priority handling
- **Mapping**: Customizable bindings, action mapping, profiles
- **Gamepad**: Multiple controllers, dead zones, vibration, hot-plugging

### Phase 7: Production-Ready GUI System (Q1 2026)

**Duration:** 6 weeks | **Hybrid IMGUI approach**

- **Core IMGUI** (Week 1-2): GUIContext, rendering pipeline, font/text system
- **Essential Widgets** (Week 3-4): Text fields, buttons, checkboxes, windows, tabs
- **Advanced Features** (Week 5-6): Dropdown menus, styling, themes, batching

**Target:** 60+ FPS with complex UI, 20+ widget types, runtime theme switching

### Phase 8: Physics Integration (Q2 2026)

**Duration:** 3-4 weeks

- Physics engine selection (Bullet Physics vs custom)
- Collision detection with basic shapes
- Rigid body dynamics (mass, velocity, forces)
- Physics debug rendering and ray casting
- Integration with Scene Management

### Phase 9: Audio System Enhancement (Q2-Q3 2026)

**Duration:** 3-4 weeks

- 3D spatial audio with distance attenuation
- Audio streaming for large files
- Effects processing (reverb, echo, filters)
- Music management and crossfading
- Integration with scene for positional audio

### Phase 10: Asset Pipeline (Q3-Q4 2026)

**Duration:** 4-6 weeks

- Asset importing (models, animations, textures)
- Resource management with caching
- Hot reloading for development
- Asset optimization and compression
- Batch processing tools

---

## Technical Debt Register

| ID | Area | Issue | Impact | Effort | Priority |
|----|------|-------|--------|--------|----------|
| TD-01 | Texture | Factory methods commented out | Cannot use texture factory | 4h | High |
| TD-02 | Scene | Geometry needs device access | Scene can't create GPU resources | 6h | High |
| TD-03 | Commands | DrawInstanced enum never used | Dead code | 1h | Low |
| TD-04 | Stats | RenderStats never updated | No performance metrics | 2h | Medium |
| TD-05 | State | GL calls bypass state manager | Breaks caching | 12h | High |
| TD-06 | Reflection | No shader introspection | Manual uniform management | 8h | High |
| TD-07 | Threading | No thread safety | Can't multi-thread rendering | 40h | Low |
| TD-08 | Sync | No GPU sync primitives | Potential race conditions | 8h | Medium |
| TD-09 | Resources | No central tracking | Can't detect leaks | 16h | Medium |
| TD-10 | DSA | Not using Direct State Access | More verbose code | 24h | Low |
| TD-11 | Buffers | VBO/IBO lack update methods | Inefficient for dynamic data | 12h | High |
| TD-12 | Validation | No vertex layout validation | Errors caught late | 4h | Medium |
| TD-13 | Errors | No centralized GL error checking | Silent failures | 4h | High |
| TD-14 | Labels | Debug labels only on FBO | Hard to debug in RenderDoc | 6h | Medium |
| TD-15 | Binding | No binding point registry | Potential conflicts | 4h | High |

**Total Debt:** ~155 hours | **High Priority Items:** 9/15

---

## Timeline Summary

### Q3 2025 (Current - September)

- ✅ Math Library (Complete)
- ✅ Image Loading (Complete)
- ✅ Scene Management Core (Complete)
- 🔄 **Phase 0-1**: OpenGL enhancement, forward rendering (6 weeks)

### Q4 2025 (October - December)

- **Phase 2-4**: Shadows, deferred, production polish (6 weeks)
- **Phase 5-6**: Scene management, input system (2 months)

### Q1 2026 (January - March)

- **Phase 7**: Production-ready GUI system (6 weeks)
- Physics integration begins

### Q2 2026 (April - June)

- **Phase 8**: Physics integration complete
- **Phase 9**: Audio system enhancement
- Asset pipeline begins

### Q3-Q4 2026 (July - December)

- **Phase 10**: Asset pipeline completion
- Advanced graphics features (ray tracing, compute)
- Multi-API support (DirectX 12, Vulkan)
- Platform expansion planning

---

## Navigation

- **Previous:** [Critical Issues & Blockers](CRITICAL_ISSUES_AND_BLOCKERS.md)
- **See also:** [Architecture & Best Practices](ARCHITECTURE_AND_BEST_PRACTICES.md) _(coming soon)_

---

**Document Status:** Consolidated implementation roadmap  
**Last Updated:** 2025-10-31  
**Total Phases:** 10 (Phase 0-4: Core, Phase 5-10: Extended)  
**Critical Path:** 6-8 weeks to production-ready rendering
