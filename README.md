# Pyramid Engine

Pyramid Engine is a Windows-first, general-purpose C++17 game-engine project built around a Win32/WGL platform layer and an OpenGL renderer. A future Ruqoom RTS is the first proving game, not a constraint on the engine architecture.

**Current development version:** `0.6.0-pre-alpha`

The project is intended for engine development and experimentation. It is not yet a stable SDK or production-ready game engine.

## Status

| Area | Current state |
|---|---|
| Platform | Windows 10/11 x64 only |
| Graphics | OpenGL 3.3 core or newer |
| Toolchain | MSYS2 UCRT64, MinGW-w64 GCC, Ninja, and CMake 3.23+ |
| Optional compiler | Clang targeting the same MinGW-w64/UCRT runtime |
| Renderer | Forward, shadow, and deferred passes; several advanced paths remain partial |
| Scene | Stable-ID entities, hierarchical transforms, optional mesh-renderer/light components, scene serialization, and octree queries |
| Foundation | Standalone `Pyramid::Foundation` types, colors, assertions, and logging |
| Math | Standalone `Pyramid::Math` vectors, matrices, quaternions, geometry helpers, and SIMD utilities |
| Input | Standalone `Pyramid::Input` physical state and named action mapping |
| Images | Standalone `Pyramid::Image` library with TGA/BMP subsets, custom non-interlaced PNG, and owned baseline/progressive JPEG decoding |
| Models | Standalone `Pyramid::Model` OBJ/MTL parser with bounded diagnostics, triangulation, indexing, generated normals, and normalized dependencies |
| Text | Standalone `Pyramid::Text` UTF-8 decoding, wrapping/alignment, glyph-run generation, and deterministic embedded ASCII debug atlas |
| UI | Standalone hybrid `Pyramid::UI` retained state/immediate facade with layout, scrolling, collapsing sections, input capture, clipping, widgets, and renderer-independent draw lists |
| Tests | 46 CTest targets, including standalone foundation, math, input, image, model, text, and UI coverage plus focused engine/reference-game tests |
| CI | GCC and Clang, Debug and Release, package install, and external-consumer validation |

## Implemented

- Application lifecycle and frame loop through `Pyramid::Game`.
- Real Win32 keyboard and mouse input with held/pressed/released states, pointer movement, wheel deltas, and focus-safe reset behavior.
- Engine-generic named input actions with button/1D/2D values, prioritized contexts, control consumption, chords, and runtime rebinding.
- Reusable camera-controller framework with free-fly, target-orbit, and optional XZ-ground-plane strategy controllers. Controllers consume configurable named actions and never own physical bindings.
- A game-side RTS reference interaction layer with focus-safe edge scrolling, ray-based selection, and ground-plane command requests; RTS semantics remain outside `Pyramid::Engine`.
- Win32 window creation, resize-event delivery, resize-safe viewport updates, visibility, positioning, and WGL context management.
- OpenGL device, a game-owned `ResourceRegistry`, generation-checked typed resource handles, versioned resource manifests and entity-scene serialization, engine-owned mesh and material resources, stable resource identifiers, content-deduplicating mesh, shader-program, texture, and material caches, buffers, vertex arrays, shaders, textures, resize-safe framebuffers, and state caching.
- Forward, cascaded-shadow, deferred-geometry, and deferred-lighting passes.
- Perspective and orthographic cameras with normalized world-space frusta and point/sphere/AABB visibility tests.
- Bounds-aware point, sphere, box, ray, nearest-object, and K-nearest scene queries with octree/linear parity.
- An authoritative entity/component scene: every entity has a stable ID, name, visibility, and hierarchical transform; mesh-renderer and light components are optional. Renderer/light proxies are derived from entities for rendering, culling, and spatial queries.
- OpenGL driver debug callbacks and centralized error diagnostics in Debug builds.
- Foundation types/logging, math primitives, physical/action input, image decoding, CPU model importing, debug text, and UI layout/interaction are independently testable owned libraries.
- Dependency-free OBJ/MTL import supports positive/negative indices, polygon triangulation, material groups, common MTL properties, generated smooth or hard-edge normals, vertex deduplication, bounds, file/memory input, and bounded malformed-input diagnostics.
- `ModelResourceImporter` transactionally publishes imported meshes, diffuse textures, and immutable materials through the existing caches using a configurable shader/material profile; repeated content is reused and failed imports roll back only resources introduced by that operation.
- Hybrid `Pyramid::UI` contexts reconcile immediate widget calls into retained state, generate renderer-independent clipped draw batches, and reserve handled controls before gameplay action evaluation.
- `UIRenderer` uploads batched quads through the graphics device, renders the embedded `Pyramid::Text` atlas, supports registered image textures, explicitly restores the final DPI-scaled surface viewport after off-screen passes, applies scissor clipping, and restores the engine render-state baseline.
- `BasicGame` includes an F1 runtime overlay with collapsible frame/render/resource/input diagnostics, animation/camera controls, and a scrollable severity-colored runtime log.
- Installable CMake packages export `Pyramid::Foundation`, `Pyramid::Math`, `Pyramid::Input`, `Pyramid::Image`, `Pyramid::Model`, `Pyramid::Text`, `Pyramid::UI`, and `Pyramid::Engine`.

## Important limitations

- DirectX and Vulkan enum values are reserved; only OpenGL is implemented.
- Linux and macOS builds are rejected explicitly.
- Compute dispatch is recorded by the command buffer but is not executed by OpenGL.
- `SceneSerializer` version 2 persists stable entities, hierarchy, transforms, mesh-renderer components, light components, and the primary light. Cameras, environment settings, gameplay components, and editor metadata are not serialized yet; the legacy `SceneManager` JSON/XML/Binary methods remain unsupported.
- Occlusion culling remains a placeholder and is disabled by default.
- `ITexture2D::CreateDepthTarget` fails explicitly; use the framebuffer API for depth attachments.
- Audio and physics modules are not part of the current source tree. Text editing/input, runtime font import, bidirectional/Arabic shaping, advanced retained screen authoring, raw relative mouse mode, collision-aware cameras, camera blending, and persisted user bindings are not implemented yet. The reference edge-scrolling/selection/command layer is example support rather than an installed engine API. Model import currently supports OBJ/MTL and diffuse `map_Kd` material publication only; additional material maps, model formats, asset packaging, and scene-hierarchy instantiation remain later steps.

See [Roadmap and known issues](docs/ROADMAP.md) before building new systems on top of the engine.

## Toolchain setup

Pyramid does **not** require Visual Studio or the Microsoft C++ compiler. The supported default is MSYS2 UCRT64 with MinGW-w64 GCC and Ninja.

1. Install MSYS2 to its default location: `C:\msys64`.
2. From PowerShell in the repository, install the toolchain:

```powershell
./scripts/bootstrap-msys2.ps1 -Compiler gcc
```

3. Either open **MSYS2 UCRT64** or use the PowerShell build wrapper below.

## Build from PowerShell

```powershell
./scripts/build-mingw.ps1 -Compiler gcc -Configuration Debug
```

That configures, builds, and runs all tests. A Release build is:

```powershell
./scripts/build-mingw.ps1 -Compiler gcc -Configuration Release
```

Optional Clang validation:

```powershell
./scripts/bootstrap-msys2.ps1 -Compiler both
./scripts/build-mingw.ps1 -Compiler clang -Configuration Debug
```

## Build from MSYS2 UCRT64

```bash
cmake --preset gcc-debug-tests
cmake --build --preset build-gcc-debug-tests
ctest --preset test-gcc-debug
```

Release validation:

```bash
cmake --preset gcc-release-tests
cmake --build --preset build-gcc-release-tests
ctest --preset test-gcc-release
```

### Graphical smoke test

Run this from PowerShell after a successful build:

```powershell
./scripts/run-smoke.ps1 -BuildDir build/gcc-debug-tests -DurationSeconds 5
```

This catches early process failure. It does not replace visual inspection or render-image regression testing.

## Minimal application

```cpp
#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Input/InputActions.hpp>

class MyGame final : public Pyramid::Game
{
protected:
    void onCreate() override
    {
        Game::onCreate();
        if (!IsInitialized() || !GetResourceRegistry())
        {
            quit();
            return;
        }

        auto* gameplay = GetInputActions().CreateContext("gameplay");
        if (!gameplay ||
            !gameplay->AddAction("Quit", Pyramid::InputActionType::Button) ||
            !gameplay->AddBinding(
                "Quit",
                Pyramid::InputBinding::KeyBinding(Pyramid::Key::Escape)))
        {
            quit();
        }
    }

    void onUpdate(float deltaTime) override
    {
        Game::onUpdate(deltaTime);
        if (GetInputActions().WasActionPressed("gameplay", "Quit"))
            quit();
    }

    void onRender() override
    {
        auto* device = GetGraphicsDevice();
        if (!device)
            return;

        device->Clear(Pyramid::Color(0.08f, 0.10f, 0.14f, 1.0f));
        device->Present(true);
    }

    void onWindowResize(const Pyramid::WindowResizeEvent& event) override
    {
        Game::onWindowResize(event);
        if (!event.HasRenderableArea())
            return;

        // The engine updates the default viewport automatically. Register a
        // camera with SetActiveCamera() and a RenderSystem with SetRenderSystem()
        // to synchronize their window-sized resources. Standalone framebuffers
        // can call OpenGLFramebuffer::Resize() from this hook.
    }
};

int main()
{
    MyGame game;
    game.run();
}
```

A derived `onCreate()` must call `Game::onCreate()` before creating GPU resources.

## Install and consume

```bash
cmake --preset gcc-release-tests
cmake --build --preset build-gcc-release-tests
cmake --install build/gcc-release-tests --prefix install
```

External engine project:

```cmake
find_package(PyramidEngine CONFIG REQUIRED)
add_executable(MyGame main.cpp)
target_link_libraries(MyGame PRIVATE Pyramid::Engine)
```

Owned libraries can be consumed without the engine or OpenGL package:

```cmake
find_package(PyramidFoundation CONFIG REQUIRED)
find_package(PyramidMath CONFIG REQUIRED)
find_package(PyramidInput CONFIG REQUIRED)
add_executable(MyTool main.cpp)
target_link_libraries(MyTool PRIVATE Pyramid::Foundation Pyramid::Math Pyramid::Input)
```

Standalone image and model tooling use the same package model:

```cmake
find_package(PyramidImage CONFIG REQUIRED)
find_package(PyramidModel CONFIG REQUIRED)
find_package(PyramidUI CONFIG REQUIRED)
add_executable(MyAssetTool main.cpp)
target_link_libraries(MyAssetTool PRIVATE Pyramid::Image Pyramid::Model Pyramid::UI)
```

The CI workflow validates engine, foundation/math/input, image, model, and UI package-consumer paths with GCC and Clang.

## Documentation

- [Documentation index](docs/README.md)
- [Building and testing](docs/BUILDING.md)
- [Architecture](docs/Architecture.md)
- [API overview](docs/API.md)
- [Examples](docs/EXAMPLES.md)
- [Development guide](docs/DEVELOPMENT.md)
- [Roadmap and known issues](docs/ROADMAP.md)
- [Changelog](CHANGELOG.md)

## Repository layout

```text
Libraries/
  PyramidFoundation/  Primitive types, colors, assertions, and logging
  PyramidMath/        Vectors, matrices, quaternions, and SIMD utilities
  PyramidInput/       Physical input state and named action mapping
  PyramidImage/       Owned image parsing, PNG/DEFLATE, and JPEG decoding
  PyramidModel/       Dependency-free OBJ/MTL parsing and imported-model data
Engine/
  Core/               Application lifecycle
  Graphics/           OpenGL resources, renderer, camera, scenes, and octree
  Platform/           Win32/WGL window and context implementation
Examples/          BasicGame, BasicRenderingExample, and game-side RTS reference support
Tests/             Focused tests and external package consumers
CMake/        Package configuration template
scripts/      MSYS2 setup, builds, clean configure, and smoke tests
docs/         Maintained documentation
vendor/glad/  Bundled OpenGL/WGL loader
```

## License

Pyramid Engine is licensed under the [MIT License](LICENSE).
