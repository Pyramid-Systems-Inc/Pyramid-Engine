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
- a retained main menu starts the reference runtime; `Escape` opens/closes the modal pause menu, `F10` quits, `Space` toggles cube animation during gameplay, and `F1` toggles the runtime debug overlay;
- responsive retained main-menu/HUD/pause screens plus collapsible diagnostics, wrapped help text, runtime controls, and a scrollable severity-colored in-memory log console;
- the Ruqoom-owned `Pyramid Sans` TrueType source, deterministic `.pfont` runtime asset, kerning-aware `Pyramid::Text` layout, automatic copied font assets, and embedded-font fallback when the processed file is unavailable;
- explicit final-surface viewport restoration after fixed-resolution shadow/off-screen passes;
- UI pointer capture that blocks both named actions and raw RTS edge scrolling/selection/commands;
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


Compile another TrueType source into Pyramid's runtime format:

```powershell
.\build\gcc-debug-tests\bin\PyramidFontCompiler.exe `
    .\Fonts\MyFont.ttf `
    .\Fonts\MyFont-24.pfont `
    24 512 U+00E9 U+03A9 U+2713 U+0600-U+06FF
```

The compiler always includes Basic Latin and accepts additional single codepoints or inclusive ranges in `U+XXXX[-U+YYYY]` form. The compiler is an owned development tool; packaged games load `.pfont` rather than depending on a font middleware DLL.

Selection and command requests are logged and appear in the runtime log panel. The reference helper lives under `Examples/RTSReference`; it is deliberately not exported by the installed `Pyramid::Engine` package.

## BasicRenderingExample

Location: `Examples/BasicRendering`

This is the lower-level reference rendering path. It demonstrates:

- one game-owned `ResourceRegistry` instead of four manually ordered cache owners;
- typed resource handles suitable for scene-facing references without long-lived mesh/material ownership;
- inline GLSL 3.30 shaders compiled through a stable shader-cache asset ID;
- a Pyramid-owned OBJ/MTL parser loading the bundled pyramid reference asset beside the executable;
- generated hard-edge normals, polygon triangulation, vertex colors, and material-slot metadata;
- transactional publication of imported meshes, the MTL `map_Kd` image, and immutable material properties through the registry caches;
- a caller-defined shader/material profile controlling uniform names, sRGB texture handling, sampler state, fallback behavior, and stable resource identifiers;
- scene uniform buffers;
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
- geometry is visible and initially framed from above rather than clipped by a stale shadow-map viewport;
- the F1 overlay shows text/controls across the full panel and its log area scrolls without off-screen widgets receiving clicks;
- resize, maximize, minimize, and restore messages appear in the log;
- the imported pyramid keeps the correct proportions after resizing;
- minimizing suspends rendering until a renderable client area is restored;
- resizing does not crash;
- named keyboard and mouse actions respond without repeated one-frame presses;
- BasicGame edge scrolling waits for a valid pointer sample, left-click selection follows the visible cube, and right-click commands log a ground-plane target;
- switching focus does not leave keys or mouse buttons stuck;
- closing exits cleanly;
- the OpenGL debug/error log remains clean.

The project does not yet include automated pixel-comparison tests.
