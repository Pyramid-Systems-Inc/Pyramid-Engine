# Development guide

## Workflow

1. Create a focused branch.
2. Configure a test preset.
3. Build and run CTest before changing behavior.
4. Make one coherent subsystem change.
5. Add or update tests, including linkage coverage for public APIs.
6. Run the graphical smoke test and visually inspect renderer changes.
7. Update existing documentation and the changelog.
8. Record exact validation commands in the pull request.

Recommended local validation:

```powershell
cmake --preset gcc-debug-tests
cmake --build --preset build-gcc-debug-tests
ctest --preset test-gcc-debug
./scripts/run-smoke.ps1 -BuildDir build/gcc-debug-tests -DurationSeconds 5
```

## Conventions

- C++17 with extensions disabled.
- Four-space indentation and braces on new lines.
- `PascalCase` for types and most public methods.
- `camelCase` for locals and parameters.
- `m_` prefix for fields.
- RAII and smart pointers for ownership.
- `PYRAMID_LOG_*` and assertion macros for diagnostics.
- No required interface operation may silently default to a no-op.
- Platform-specific APIs belong behind platform implementation headers. Runtime files deployed beside executables must use the shared executable-relative path resolver; do not duplicate `GetModuleFileNameW` logic or rely on the process current working directory.

The primary toolchain enables GCC/Clang `-Wall -Wextra -Wpedantic`. Warnings are not errors by default because existing warnings still need cleanup. New code should not add warnings.

## Public API discipline

For every public method, choose one behavior:

1. implement it;
2. remove it before release;
3. fail explicitly with a documented capability limitation.

Do not leave declarations without definitions. Add the symbol to `Tests/PublicApiLinkage.cpp` when a factory or subsystem method is especially vulnerable to linker regressions.

## Tests

Owned-library tests live beside their implementations under `Libraries/<Library>/test`; engine integration tests remain under `Tests/`. A test must:

- return non-zero on failure;
- avoid hidden skips that report success;
- use standards-valid embedded fixtures;
- clean up temporary files;
- print enough context to diagnose a failure.

`Foundation.Core`, `Math.Core`, `Platform.InputState`, `Platform.RuntimePaths`, `Input.ActionMapping`, `Font.TrueTypePipeline`, `Font.ReferenceTypography`, `Text.International`, and the `Model.*` parser tests validate the extracted owned libraries directly. `Graphics.ModelMaterialResourceImport` validates the graphics-side OBJ/MTL-to-mesh/texture/material transaction, including reuse, conflicts, fallback policy, malformed dependencies, and rollback. `API.PublicApiLinkage` verifies selected exported engine symbols. The focused suite covers OpenGL diagnostics, window/viewport/framebuffer behavior, image and texture loading, camera frusta, octree update/query/configuration/compaction behavior, resource types and caches, registry handles/manifests, the authoritative entity scene, and versioned entity-scene serialization. `Graphics.EntityScene` verifies stable IDs, hierarchy composition, inherited visibility, component attachment, renderer/light proxy synchronization, recursive destruction, and cache invalidation. `Graphics.SceneSerialization` verifies version-2 entity/component round trips, hierarchy validation, deterministic manifest-key references, light and mesh-renderer persistence, and missing/stale resource diagnostics. JPEG tests must use standards-valid encoded fixtures and cover malformed/truncated data, progressive scans, subsampling, restart markers, and bounded allocation behavior. Windows CI validates GCC and Clang in Debug and Release, installation, and independent engine, foundation/math/input, image, model, font, and UI `find_package` consumers.

Renderer changes require manual visual validation until image-regression tests exist.

Mesh changes must preserve immutable layout/count/topology/bounds/identifier metadata, keep raw GPU buffers out of `RenderObject`, reject invalid index and vertex ranges before upload, and keep indexed/non-indexed command submission equivalent. Cache changes must deduplicate by exact content, preserve deterministic identifiers, reject one alias mapped to different resident content, retain strong ownership until explicit eviction/collection, and never invalidate external resource owners during cache removal or reload. Texture identity must include decoded pixels, sampler/mip state, and explicit color space; file reload must publish only after a complete replacement exists. Material identity must include referenced shader/texture content IDs, slots, typed uniform values, and fixed state while excluding debug names; dynamic per-draw values must stay outside the material resource. Material-cache replacement must validate or resolve a complete immutable replacement before publishing a stable alias, preserve old external owners, and reject replacement through content-derived identifiers. Application code should use the `Game`-owned `ResourceRegistry`; registry maintenance and teardown must always process materials before textures, shaders, and meshes, and the registry must die before the graphics device/context. Handle changes must preserve three invariants: handles never own resources, any alias bind/remap/removal advances its non-zero generation, and direct cache operations invalidate registry-issued handles exactly like registry wrappers. Replacement APIs must return the newly issued generation, while stale or forged handles resolve to null. Manifest changes must preserve deterministic ordering, exact 128-bit ID values, transactional parsing, explicit version rejection, and separate missing-asset versus stale-generation diagnostics.

Entity-scene changes must preserve these invariants: entity ID zero is invalid; live IDs are unique; every entity owns one transform; hierarchy links use entity IDs; no cycles or duplicate direct children are allowed; parent/local edits invalidate the complete descendant subtree; reparenting preserves local TRS; inherited visibility is deterministic; and `RenderObject`/`Light` remain derived proxies rather than a second authoring model. Add or update `Graphics.EntityScene` and `Graphics.SceneSerialization` before changing those semantics.

Frustum planes must remain normalized and inward-facing in the order left, right, bottom, top, near, far. Scene visibility must test transformed object bounds rather than center points. An octree object may descend only when its complete AABB fits inside one child; spanning objects stay in the parent. Spatial synchronization must compare complete world AABBs, tolerate insignificant floating-point drift, and deduplicate scene snapshots. Configuration changes must validate first, rebuild into temporary ownership, preserve every tracked object, and swap live state only after successful construction. Compaction must run bottom-up, promote every surviving descendant before releasing children, never alter the tracked-object map, and remain a no-op when the structure is already minimal. Spatial queries must test complete world AABBs, keep root-retained objects queryable outside configured bounds, return unique results, and preserve identical semantics when the octree is disabled.

Debug builds request an OpenGL debug context and attach a synchronous driver callback when supported. Driver warnings and errors are routed through `PYRAMID_LOG_*`; notification-level messages are suppressed. Release builds do not enable the callback.

Window resize callbacks run synchronously on the game thread while `ProcessMessages()` dispatches native events. Before the virtual hook runs, `Game` updates the default viewport, synchronizes the camera registered through `SetActiveCamera()`, resizes the `RenderSystem` registered through `SetRenderSystem()`, and marks zero-sized/minimized surfaces non-renderable. Keep custom handlers lightweight, reject `!event.HasRenderableArea()`, resize only standalone targets there, and avoid retaining references to the callback argument.

## CMake and package changes

After changing targets, include directories, or installation:

```bash
cmake --preset gcc-release-tests
cmake --build --preset build-gcc-release-tests
ctest --preset test-gcc-release
cmake --install build/gcc-release-tests --prefix install
cmake -S Tests/Consumer -B build/consumer -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=g++ \
  -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build build/consumer --parallel
```

Keep build-tree paths out of installed target interfaces. Public headers should be installed as files, not exposed as absolute `INTERFACE_SOURCES`. `Pyramid::Foundation`, `Pyramid::Math`, `Pyramid::Input`, `Pyramid::Image`, `Pyramid::Model`, `Pyramid::Font`, `Pyramid::Text`, and `Pyramid::UI` are exported as relocatable packages; future owned libraries must follow that pattern or an explicitly documented package boundary. GLAD is the only approved bundled third-party runtime library; do not add codec or middleware package dependencies to bootstrap, CI, or package configuration by default.

## Documentation

Maintain this fixed set:

```text
README.md
docs/README.md
docs/BUILDING.md
docs/Architecture.md
docs/API.md
docs/EXAMPLES.md
docs/DEVELOPMENT.md
docs/ROADMAP.md
CHANGELOG.md
```

Do not add separate blocker reports, status snapshots, duplicated setup guides, or speculative API references.

## Pull requests

Include:

- purpose and affected modules;
- public API or ownership changes;
- exact Debug/Release/test commands;
- screenshots or video for visual changes;
- known limitations and follow-up work;
- documentation and changelog updates.

## Input changes

Keep native virtual-key codes and Win32 message types inside the platform implementation. `InputState` is the platform-neutral physical snapshot; `InputActionSystem` is the engine-generic semantic layer above it. Do not hard-code RTS, editor, UI, vehicle, or other product-specific action names into the input module. Every physical transition rule belongs in `Tests/InputStateTests.cpp`; action aggregation, priority, consumption, chords, and rebinding belong in `Tests/InputActionTests.cpp`. Camera controllers must feed the same action layer instead of adding gameplay concepts to `Window`. Keep their action references configurable, keep physical defaults in examples/games, and use separate delta actions for mouse motion versus rate actions for held digital or analog axes. Controller behavior belongs in `Tests/CameraControllerTests.cpp`. Product semantics such as selectable filters, command destinations, ownership, units, and screen-edge behavior must stay above the engine in game/reference code. The maintained example implementation is `Examples/RTSReference`, with focused coverage in `Tests/RTSInteractionTests.cpp`.

## Release hygiene

Before tagging:

1. synchronize CMake version, status, tag, and changelog;
2. pass clean GCC and Clang Debug/Release CI;
3. run all registered tests;
4. manually run and inspect both examples;
5. install into an empty prefix and build both external consumers;
6. verify every documented public method is implemented or explicitly unsupported;
7. review the roadmap and remove completed P0 items.


Font changes must keep source parsing and rasterization inside `Pyramid::Font`, enforce byte/table/glyph/mapping/depth/atlas limits, reject cyclic compound glyphs, preserve deterministic `.pfont` output, and retain the embedded debug atlas as a no-filesystem fallback. Reference-font changes must regenerate through `scripts/regenerate-reference-fonts.py --check`, retain owned source outlines, preserve contextual joining-edge tests, and validate antialiased 48-pixel atlas output plus linear UI sampling. Do not add FreeType, HarfBuzz, ICU, stb, platform font APIs, or other font middleware. Parser, rasterizer, kerning, processed-asset corruption, package-consumer, and UI-atlas integration tests are required. International-text changes belong in `Pyramid::Text`: preserve logical Unicode-scalar offsets at grapheme boundaries, deterministic fallback precedence, logical/visual cluster maps, cluster-safe wrapping/editing, and renderer-neutral shaping/bidi/hit-testing behavior. Add focused grapheme, Arabic, mixed-direction, fallback, caret, selection, and line-break tests, and document any conformance subset rather than claiming complete Unicode or OpenType support.

Model importer changes must keep parsing renderer-independent, enforce bounded input/count limits, preserve deterministic primitive/material ordering, reject malformed and missing declared dependencies explicitly, and extend the standalone parser plus graphics-publication tests. Graphics import changes must take shader/uniform/sampler policy from a caller-owned profile, preserve source material slots, validate the full profile before cache mutation, reuse exact content, reject stable-ID conflicts, and roll back newly introduced mesh, texture, and material aliases/content without disturbing pre-existing resources.
