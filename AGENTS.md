# Repository guidelines

## Scope

- `Engine/` builds the C++17 `PyramidEngine` library.
- Active modules are Core, Graphics, Math, Platform, and Utils.
- `Examples/BasicGame` and `Examples/BasicRendering` are the graphical references.
- `Tests/PublicApiLinkage.cpp` protects selected public symbols. Focused tests cover platform input state, resize behavior, camera/frustum logic, framebuffers, texture loading, spatial updates/queries/configuration/compaction, render bounds, resources/caches, registry handles/manifests, the authoritative entity scene, and scene serialization. `Tests/EntitySceneTests.cpp` protects stable entity IDs, cycle-safe hierarchy transforms, inherited visibility, component attachment, generated renderer/light proxies, and recursive destruction. `Tests/SceneSerializationTests.cpp` protects deterministic version-2 entity/component round trips, exact manifest references, hierarchy validation, and missing/stale resource diagnostics.
- `Tests/Consumer` validates the installed CMake package.
- `vendor/glad` is a bundled public dependency. libjpeg-turbo is an external open-source dependency resolved through CMake `FindJPEG`.
- The supported Windows toolchain is MSYS2 UCRT64 with MinGW-w64 GCC; Clang is also validated. Visual Studio is not required.
- Real Win32 keyboard/mouse polling is implemented. Action mapping, audio, physics, editor, scripting, DirectX, Vulkan, Linux, and macOS are not yet supported. The first product target is a Windows RTS vertical slice; Linux follows it, a full editor is planned, and Baa is the long-term scripting and rewrite target.

## Build and test

```powershell
cmake --preset gcc-debug-tests
cmake --build --preset build-gcc-debug-tests
ctest --preset test-gcc-debug
./scripts/run-smoke.ps1 -BuildDir build/gcc-debug-tests -DurationSeconds 5
```

Release validation:

```powershell
cmake --preset gcc-release-tests
cmake --build --preset build-gcc-release-tests
ctest --preset test-gcc-release
```

## Style

- C++17, four spaces, braces on new lines.
- Types/public methods use `PascalCase`; locals/parameters use `camelCase`; fields use `m_`.
- Prefer RAII, explicit ownership, and `PYRAMID_LOG_*` diagnostics. Geometry passed to scenes must use `Mesh`; do not reintroduce raw vertex-array fields on `RenderObject`. Shared reusable geometry should be acquired through the game-owned `ResourceRegistry::Meshes()` cache; do not create parallel uploads for byte-identical mesh specifications. Shared shader programs should be acquired through `ResourceRegistry::Shaders()`; do not mutate cached `ShaderProgram` instances directly or compile identical stage source repeatedly. Shared sampled textures should use `ResourceRegistry::Textures()`; cached `TextureResource` instances are immutable, color space is part of identity, and file changes must be published through transactional reload. Scene renderables should acquire immutable `Material` resources through `ResourceRegistry::Materials()` rather than ad-hoc shader/texture fields; per-draw matrices belong in command-buffer uniforms, not material identity. Platform input must remain backend-neutral at the public API boundary: native messages feed `InputState`, games poll it through `Game::GetInput()`, and focus loss must release held controls. Long-lived scene and serialized references should use typed registry handles. Handles must stay non-owning, every alias bind/remap/removal must advance its generation, direct cache mutation must invalidate existing handles, and stale handles must resolve to null rather than a replacement resource.
- Scene authoring must use `Entity` plus components; do not reintroduce `SceneNode` or make `RenderObject` transforms authoritative. Renderer/light proxies are generated from the scene. Stable entity IDs and hierarchy invariants must remain serialization-safe.
- Do not add required interface methods with silent no-op defaults.
- Do not expose source-tree absolute paths through installed target interfaces.

## Public APIs

Every public declaration must be implemented, removed, or documented as an explicit failure. Add linkage-sensitive symbols to `Tests/PublicApiLinkage.cpp`.

Do not describe placeholder algorithms as complete. Frustum culling is implemented; occlusion culling is not. Current examples and engine shaders target GLSL 3.30; the runtime requires OpenGL 3.3 core or newer.

## Tests

Tests must fail visibly, use valid fixtures, avoid false-success skips, clean temporary files, and print actionable context. Renderer changes require visual inspection because process smoke testing is not pixel validation.

## Documentation

Update the maintained compact set instead of adding overlapping guides or status files. Planned work belongs in `docs/ROADMAP.md`; historical changes belong in `CHANGELOG.md`.

## Pull requests

Include affected modules, ownership/API decisions, exact validation commands, visible rendering evidence, known limitations, and documentation/changelog updates.
