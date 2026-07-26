# Examples

The repository contains two Windows graphical applications plus one non-installed RTS reference support library. Both applications use GLSL 3.30 and require an OpenGL 3.3-or-newer core context.

## BasicGame

Location: `Examples/BasicGame`

Demonstrates:

- deriving from `Pyramid::Game`;
- base lifecycle initialization;
- the `Game`-owned `ResourceRegistry` and generation-checked mesh/material handles;
- authoritative scene entities with `TransformComponent`, `MeshRendererComponent`, and `LightComponent`;
- stable shader-program compilation and reuse through the registry shader cache;
- engine-owned indexed mesh creation with validated layout, topology, draw count, and local bounds;
- content-derived mesh-cache reuse: the cube and floor request identical geometry but perform one GPU upload;
- texture loading;
- frame update by mutating entity transforms and rendering through generated scene proxies;
- engine-generic named actions with game-owned physical bindings;
- the reusable `RTSCameraController`: WASD pans, Q/E or middle-drag orbits, the wheel zooms, Shift boosts movement, and `R` restores the captured home pose;
- `Pyramid::RTSReference` game-side interaction: moving the pointer to a window edge scrolls, left click selects the demo cube through a selectable predicate, and right click emits a ground-plane command for the selected entity;
- `Escape` quits and `Space` toggles cube animation;
- automatic default-viewport updates and active-camera projection resizing;
- platform-neutral resize-event logging through `Game::onWindowResize()`;
- logging configuration.

Build:

```powershell
cmake --preset gcc-debug
cmake --build --preset build-gcc-debug
```

Run:

```powershell
./build/gcc-debug/bin/BasicGame.exe
```

Selection and command requests are logged. The reference helper lives under `Examples/RTSReference`; it is deliberately not exported by the installed `Pyramid::Engine` package.

## BasicRenderingExample

Location: `Examples/BasicRendering`

This is the lower-level reference rendering path. It demonstrates:

- one game-owned `ResourceRegistry` instead of four manually ordered cache owners;
- typed resource handles suitable for scene-facing references without long-lived mesh/material ownership;
- inline GLSL 3.30 shaders compiled through a stable shader-cache asset ID;
- an engine-owned cube mesh with position, normal, texture-coordinate, and color attributes;
- caller-defined stable mesh and shader asset identifiers resolved through their graphics-device-bound caches;
- scene and material uniform buffers;
- perspective camera setup registered through `Game::SetActiveCamera()`;
- topology-aware indexed rendering and basic lighting;
- the optional `RTSCameraController` consuming a reference action profile: `Escape` quits, `R`/`1` resets, WASD/arrows move across the XZ plane, `Q`/`E` rotate, right-drag orbits/tilts, Page Up/Down and the wheel zoom, and Shift boosts movement. The controller API and input module contain no game-specific physical bindings.

Run:

```powershell
./build/gcc-debug/bin/BasicRenderingExample.exe
```

## Smoke validation

```powershell
./scripts/run-smoke.ps1 -BuildDir build/gcc-debug -DurationSeconds 5
```

A successful timed process run proves that startup did not immediately fail. After graphics changes, also verify:

- both windows open;
- shader compilation reports no errors;
- geometry is visible;
- resize, maximize, minimize, and restore messages appear in the log;
- the cube keeps the correct proportions after resizing;
- minimizing suspends rendering until a renderable client area is restored;
- resizing does not crash;
- named keyboard and mouse actions respond without repeated one-frame presses;
- BasicGame edge scrolling waits for a valid pointer sample, left-click selection follows the visible cube, and right-click commands log a ground-plane target;
- switching focus does not leave keys or mouse buttons stuck;
- closing exits cleanly;
- the OpenGL debug/error log remains clean.

The project does not yet include automated pixel-comparison tests.
