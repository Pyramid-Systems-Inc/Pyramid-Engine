# Repository Guidelines

## Project Structure & Module Organization
`Engine/` contains the main `PyramidEngine` library. Active modules typically follow `include/` + `source/` layout (for example `Engine/Core`, `Engine/Graphics`, `Engine/Math`, `Engine/Platform`, `Engine/Utils`).  
`Engine/Utils/test/` holds standalone C++ test executables for PNG/JPEG and utility components.  
`Examples/BasicGame` and `Examples/BasicRendering` provide runnable sample apps.  
`docs/` contains architecture, API, build, and contribution docs.  
`vendor/glad/` is the bundled OpenGL loader dependency.  
`build/` is generated output and should stay uncommitted.

## Build, Test, and Development Commands
Use CMake from repo root:

```powershell
cmake -B build -S . -DPYRAMID_BUILD_EXAMPLES=ON
cmake --build build --config Debug
```

Build with tests enabled:

```powershell
cmake -B build -S . -DPYRAMID_BUILD_TESTS=ON -DPYRAMID_BUILD_EXAMPLES=ON
cmake --build build --config Debug --target PyramidEngine BasicGame BasicRenderingExample TestPNGComponents
```

Run binaries directly from the build tree (path varies by generator/config), for example:

```powershell
.\build\bin\Debug\BasicGame*.exe
.\build\bin\Debug\TestPNGComponents*.exe
```

## Coding Style & Naming Conventions
Target C++17. Follow existing style: 4-space indentation, braces on new lines, and small focused translation units.  
Naming pattern in current codebase:
- Types/functions: `PascalCase` (for example `GraphicsDevice`, `Initialize`)
- Local variables/parameters: `camelCase` (for example `deltaTime`)
- Member fields: `m_` prefix (for example `m_graphicsDevice`)

Prefer RAII and smart pointers; use engine logging/assert macros (`PYRAMID_LOG_*`, `PYRAMID_ASSERT`) for diagnostics and invariants.

## Testing Guidelines
Current tests are executable-based (not fully wired into `ctest`). Add new tests under `Engine/Utils/test/` as `Test<Feature>.cpp`.  
Each test binary should return non-zero on failure and print clear pass/fail output.  
For graphics or runtime behavior changes, also validate with `BasicGame` and `BasicRenderingExample`.

## Commit & Pull Request Guidelines
Recent history favors short imperative commit subjects like `Add ...`, `Fix ...`, `Refactor ...` (without strict conventional prefixes). Keep subject lines concise and explain non-trivial rationale in the body.  
PRs should include: change summary, affected modules, exact local build/test commands run, linked issue(s), and screenshots/video for visible rendering changes.  
Update relevant docs in `docs/` when APIs, behavior, or build options change.
