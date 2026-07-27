# Building and testing

## Supported environment

Pyramid Engine currently supports native 64-bit Windows builds with an open-source MinGW toolchain:

- Windows 10 or 11, x64;
- MSYS2 using the **UCRT64** environment;
- MinGW-w64 GCC as the default compiler;
- optional Clang targeting the same MinGW-w64/UCRT runtime;
- Ninja;
- CMake 3.23 or newer;
- OpenGL 3.3 core or newer.

Visual Studio, MSVC, and the Visual Studio Build Tools are not required.

A normal configure on Linux or macOS fails immediately. `PYRAMID_ALLOW_UNSUPPORTED_HOST_CONFIGURE` is only for metadata validation; it does not make the Win32/WGL engine runnable on another platform.

## Install MSYS2 and the compiler

Install MSYS2 to `C:\msys64`, then run from PowerShell:

```powershell
./scripts/bootstrap-msys2.ps1 -Compiler gcc
```

To install both GCC and Clang:

```powershell
./scripts/bootstrap-msys2.ps1 -Compiler both
```

The script installs the UCRT64 MinGW-w64 toolchain, CMake, and Ninja. It does not install Visual Studio or any codec middleware.

After setup, use either:

- the **MSYS2 UCRT64** terminal; or
- `scripts/build-mingw.ps1` from ordinary PowerShell.

Do not build from the plain **MSYS** shell. Its `/usr` compiler targets the MSYS POSIX runtime rather than native Windows.

## PowerShell workflow

Default GCC Debug build with tests:

```powershell
./scripts/build-mingw.ps1 -Compiler gcc -Configuration Debug
```

GCC Release:

```powershell
./scripts/build-mingw.ps1 -Compiler gcc -Configuration Release
```

Clang Debug:

```powershell
./scripts/build-mingw.ps1 -Compiler clang -Configuration Debug
```

## Presets

| Configure preset | Build directory | Purpose |
|---|---|---|
| `gcc-debug` | `build/gcc-debug` | GCC Debug engine and examples |
| `gcc-debug-tests` | `build/gcc-debug-tests` | GCC Debug engine, examples, and tests |
| `gcc-release-tests` | `build/gcc-release-tests` | GCC Release engine, examples, and tests |
| `clang-debug-tests` | `build/clang-debug-tests` | Clang Debug engine, examples, and tests |
| `clang-release-tests` | `build/clang-release-tests` | Clang Release engine, examples, and tests |

### GCC Debug with tests

Run in **MSYS2 UCRT64**:

```bash
cmake --preset gcc-debug-tests
cmake --build --preset build-gcc-debug-tests
ctest --preset test-gcc-debug
```

### GCC Release with tests

```bash
cmake --preset gcc-release-tests
cmake --build --preset build-gcc-release-tests
ctest --preset test-gcc-release
```

### Clang validation

```bash
cmake --preset clang-debug-tests
cmake --build --preset build-clang-debug-tests
ctest --preset test-clang-debug
```

Clean configuration helper from PowerShell:

```powershell
./scripts/configure-clean.ps1 -Preset gcc-debug-tests
```

## CMake options

| Option | Default | Purpose |
|---|---:|---|
| `PYRAMID_BUILD_EXAMPLES` | `ON` | Build both graphical examples |
| `PYRAMID_BUILD_TESTS` | `OFF` | Enable CTest and all maintained tests |
| `PYRAMID_WARNINGS_AS_ERRORS` | `OFF` | Promote GCC/Clang warnings to errors |
| `PYRAMID_ALLOW_UNSUPPORTED_HOST_CONFIGURE` | `OFF` | Configure-only validation outside Windows |

Manual GCC configuration from UCRT64:

```bash
cmake -S . -B build/manual -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=gcc \
  -DCMAKE_CXX_COMPILER=g++ \
  -DPYRAMID_BUILD_EXAMPLES=ON \
  -DPYRAMID_BUILD_TESTS=ON
cmake --build build/manual --parallel
ctest --test-dir build/manual --output-on-failure
```

## Tests

CTest registers 35 executables: seven standalone image tests plus API, platform, input, graphics, resource, spatial, scene, and reference-game tests. List the exact graph with:

```powershell
ctest --test-dir build/gcc-debug-tests -N
```

The standalone image suite covers PNG/DEFLATE plus baseline, progressive, grayscale, subsampled, restart-marker, malformed, and truncated JPEG inputs. The platform input contract is covered by `Platform.InputState`; generic actions, contexts, consumption, chords, and rebinding are covered by `Input.ActionMapping`; resize delivery is covered by `Platform.WindowResizeEvents`.

Key scene tests are:

- `Graphics.EntityScene`: stable entity IDs, hierarchy, inherited visibility, mesh-renderer/light components, generated proxies, and recursive destruction;
- `Graphics.SceneSerialization`: deterministic version-2 entity/component persistence, manifest references, hierarchy validation, and missing/stale resource diagnostics;
- `Graphics.CameraFrustum`, `Graphics.OctreeUpdates`, `Graphics.OctreeQueries`, `Graphics.NearestQueries`, `Graphics.OctreeConfiguration`, and `Graphics.OctreeCompaction`: world-bound visibility and spatial behavior.

Resource tests cover mesh, shader, texture, and material resources/caches plus registry ownership, typed handles, and manifests. `API.PublicApiLinkage` turns missing public definitions into link failures. Tests that do not require a graphics context use fake devices/backends; renderer changes still require the graphical smoke test and visual inspection.

## Graphical smoke test

After building, run from PowerShell:

```powershell
./scripts/run-smoke.ps1 -BuildDir build/gcc-debug-tests -DurationSeconds 5
```

The script starts `BasicGame` and `BasicRenderingExample`, then fails if either exits abnormally during the requested interval. Visually inspect both windows after renderer changes. Verify the orbit and strategy camera controls after input or camera-controller changes.

Typical GCC Debug outputs:

```text
build/gcc-debug-tests/bin/BasicGame.exe
build/gcc-debug-tests/bin/BasicRenderingExample.exe
build/gcc-debug-tests/lib/libPyramidEngined.a
```

## Installable package

Build and install:

```bash
cmake --preset gcc-release-tests
cmake --build --preset build-gcc-release-tests
cmake --install build/gcc-release-tests --prefix install
```

The installation contains:

- engine and image-library public headers;
- GLAD headers;
- `PyramidEngine`, `PyramidImage`, and `glad` libraries;
- independent `PyramidEngineConfig.cmake` and `PyramidImageConfig.cmake` package metadata;
- exported `Pyramid::Engine`, `Pyramid::Image`, and `Pyramid::glad` targets.

Engine consumer:

```cmake
find_package(PyramidEngine CONFIG REQUIRED)
target_link_libraries(MyTarget PRIVATE Pyramid::Engine)
```

Standalone image consumer:

```cmake
find_package(PyramidImage CONFIG REQUIRED)
target_link_libraries(MyImageTool PRIVATE Pyramid::Image)
```

`Tests/Consumer` and `Tests/ImageConsumer` are the reference external consumers and are built independently by CI after installation. `PyramidImage` can also be installed alone with `--component PyramidImage`.

## CI

`.github/workflows/windows-ci.yml` uses MSYS2 UCRT64 and validates four combinations:

- GCC Debug;
- GCC Release;
- Clang Debug;
- Clang Release.

Each combination configures, builds, runs CTest, installs both packages, builds independent engine and image `find_package` consumers, and runs both consumers. No Visual Studio installation is used by the workflow.

## Troubleshooting

### `gcc`, `g++`, `cmake`, or `ninja` is not found

Open **MSYS2 UCRT64**, not the plain MSYS shell, or use `scripts/build-mingw.ps1` from PowerShell. Rerun `scripts/bootstrap-msys2.ps1` if the packages are missing.

### CMake still reports `Visual Studio 17 2022`

You are using an old `CMakePresets.json` or an old build directory. Pull/copy the updated files and remove the old build directory:

```powershell
Remove-Item build -Recurse -Force
```

Then use `gcc-debug-tests`, not `vs2022-debug-tests`.

### OpenGL context creation fails

Update the GPU driver and confirm OpenGL 3.3 core support. The window layer attempts versions from 4.6 down to 3.3 and rejects legacy fallback contexts.

### Shader compilation fails

The bundled engine shaders and examples use GLSL 3.30. Inspect the shader log and the reported OpenGL/GLSL versions.

### Executable cannot locate engine shaders

Development builds resolve checked-in shaders using the compile-time source root. Copying only an executable away from the repository can break this lookup.

### Build files are locked

Close running examples and any terminal/debugger using the output files, then rerun `scripts/configure-clean.ps1`.
