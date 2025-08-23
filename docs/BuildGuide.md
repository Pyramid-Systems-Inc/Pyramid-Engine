# Pyramid Engine Build Guide

## System Requirements

### Windows Development

#### Minimum Requirements
- **Operating System**: Windows 10 (64-bit) or later
- **Processor**: x64 CPU with SSE2 support
- **Memory**: 8 GB RAM minimum, 16 GB recommended
- **Storage**: 5 GB available space (including build artifacts)
- **Graphics**: OpenGL 3.3+ compatible graphics card

#### Recommended Requirements
- **Operating System**: Windows 11 (64-bit)
- **Processor**: Modern x64 CPU with AVX support for SIMD optimizations
- **Memory**: 32 GB RAM for large projects and fast compilation
- **Storage**: SSD with 10+ GB available space
- **Graphics**: Modern graphics card with OpenGL 4.6+ support

### Development Tools

#### Required Tools

##### Visual Studio 2022
- **Edition**: Community, Professional, or Enterprise
- **Required Workloads**:
  - Desktop development with C++
- **Required Components**:
  - MSVC v143 - VS 2022 C++ x64/x86 build tools (latest version)
  - Windows 10/11 SDK (latest version)
  - C++ CMake tools for Visual Studio
  - Git for Windows (if not already installed)

##### CMake
- **Version**: 3.16.0 or later (3.20+ recommended)
- **Installation**: Download from [cmake.org](https://cmake.org/download/)
- **Configuration**: Add to system PATH during installation

##### Git
- **Version**: Latest stable version
- **Installation**: Download from [git-scm.com](https://git-scm.com/)
- **Configuration**: Configure user name and email

#### Optional Tools

##### Visual Studio Code
- **Purpose**: Lightweight editor for documentation and shader editing
- **Extensions**:
  - C/C++ Extension Pack
  - CMake Tools
  - GitLens

##### Ninja Build System
- **Purpose**: Faster builds (alternative to MSBuild)
- **Installation**: Via Visual Studio installer or standalone
- **Usage**: `cmake -G Ninja` for generation

##### Vcpkg (Planned)
- **Purpose**: Package management for future dependencies
- **Status**: Will be integrated in future versions

## Installation Guide

### Step 1: Install Visual Studio 2022

1. **Download Visual Studio 2022**
   - Visit [visualstudio.microsoft.com](https://visualstudio.microsoft.com/)
   - Download Visual Studio 2022 Community (free) or your preferred edition

2. **Run the Visual Studio Installer**
   - Launch the downloaded installer
   - Select "Desktop development with C++" workload
   - Ensure the following individual components are selected:
     - MSVC v143 - VS 2022 C++ x64/x86 build tools (latest)
     - Windows 10/11 SDK (10.0.19041.0 or later)
     - C++ CMake tools for Visual Studio
     - Git for Windows (if not separately installed)

3. **Complete Installation**
   - Click Install and wait for completion
   - Restart computer if prompted

### Step 2: Install CMake

1. **Download CMake**
   - Visit [cmake.org/download/](https://cmake.org/download/)
   - Download the Windows x64 Installer

2. **Install CMake**
   - Run the installer
   - **Important**: Select "Add CMake to the system PATH for all users"
   - Complete the installation

3. **Verify Installation**
   ```cmd
   cmake --version
   ```
   Should output CMake version 3.16.0 or later

### Step 3: Configure Git (if needed)

1. **Set User Information**
   ```bash
   git config --global user.name "Your Name"
   git config --global user.email "your.email@example.com"
   ```

2. **Configure Line Endings (Windows)**
   ```bash
   git config --global core.autocrlf true
   ```

### Step 4: Clone the Repository

1. **Open Command Prompt or PowerShell**
   ```cmd
   cd C:\Development  # or your preferred directory
   git clone https://github.com/yourusername/Pyramid-Engine.git
   cd Pyramid-Engine
   ```

2. **Verify Repository Contents**
   ```cmd
   dir
   ```
   Should show directories like `Engine/`, `Examples/`, `docs/`, etc.

## Building the Engine

### Method 1: Visual Studio (Recommended)

#### Using Visual Studio 2022 IDE

1. **Open Visual Studio 2022**

2. **Open Folder**
   - Select "Open a local folder"
   - Navigate to the cloned Pyramid-Engine directory
   - Click "Select Folder"

3. **CMake Configuration**
   - Visual Studio will automatically detect CMakeLists.txt
   - Wait for CMake generation to complete (watch the Output window)
   - If prompted, select "x64-Debug" configuration

4. **Build the Engine**
   - Wait for IntelliSense indexing to complete
   - Select Build → Build All (Ctrl+Shift+B)
   - Monitor the Output window for build progress

5. **Build Success**
   - Look for "Build succeeded" in the Output window
   - Build artifacts will be in `build/` directory

#### Using Visual Studio Developer Command Prompt

1. **Open Developer Command Prompt**
   - Start Menu → Visual Studio 2022 → Developer Command Prompt for VS 2022

2. **Navigate to Repository**
   ```cmd
   cd C:\Development\Pyramid-Engine
   ```

3. **Generate Build Files**
   ```cmd
   cmake -B build -S . -G "Visual Studio 17 2022" -A x64
   ```

4. **Build the Engine**
   ```cmd
   cmake --build build --config Debug
   ```

   Or for Release build:
   ```cmd
   cmake --build build --config Release
   ```

### Method 2: Command Line with Ninja (Advanced)

1. **Open Developer Command Prompt**

2. **Generate with Ninja**
   ```cmd
   cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
   ```

3. **Build with Ninja**
   ```cmd
   cmake --build build
   ```

### Build Configurations

#### Debug Configuration
- **Purpose**: Development and debugging
- **Features**: Debug symbols, no optimization, full logging
- **Usage**: Default for development
- **Command**: `cmake --build build --config Debug`

#### Release Configuration
- **Purpose**: Production builds and performance testing
- **Features**: Full optimization, minimal logging, no debug symbols
- **Usage**: Final builds and benchmarking
- **Command**: `cmake --build build --config Release`

#### RelWithDebInfo Configuration
- **Purpose**: Performance profiling and release debugging
- **Features**: Optimization + debug symbols, balanced logging
- **Usage**: Performance analysis and release testing
- **Command**: `cmake --build build --config RelWithDebInfo`

#### MinSizeRel Configuration
- **Purpose**: Minimal size builds
- **Features**: Size optimization, minimal features
- **Usage**: Distribution builds where size matters
- **Command**: `cmake --build build --config MinSizeRel`

## Build Options

### CMake Configuration Options

Configure build options using CMake variables:

```cmd
cmake -B build -S . ^
    -DPYRAMID_BUILD_EXAMPLES=ON ^
    -DPYRAMID_BUILD_TESTS=OFF ^
    -DPYRAMID_BUILD_TOOLS=OFF ^
    -DCMAKE_BUILD_TYPE=Debug
```

#### Available Options

| Option | Default | Description |
|--------|---------|-------------|
| `PYRAMID_BUILD_EXAMPLES` | `ON` | Build example projects |
| `PYRAMID_BUILD_TESTS` | `OFF` | Build unit tests |
| `PYRAMID_BUILD_TOOLS` | `OFF` | Build development tools |
| `PYRAMID_ENABLE_SIMD` | `ON` | Enable SIMD optimizations |
| `PYRAMID_ENABLE_LOGGING` | `ON` | Enable logging system |
| `PYRAMID_ENABLE_PROFILING` | `OFF` | Enable profiling hooks |

### Environment Variables

Set environment variables to customize the build:

```cmd
set PYRAMID_SHADER_PATH=C:\Development\Pyramid-Engine\Assets\Shaders
set PYRAMID_ASSET_PATH=C:\Development\Pyramid-Engine\Assets
cmake -B build -S .
```

## Build Output

### Directory Structure

After a successful build, you'll find:

```
Pyramid-Engine/
├── build/
│   ├── bin/
│   │   ├── Debug/
│   │   │   ├── BasicGame.exe
│   │   │   ├── BasicRendering.exe
│   │   │   └── *.pdb files
│   │   └── Release/
│   ├── lib/
│   │   ├── Debug/
│   │   │   ├── PyramidEngined.lib
│   │   │   └── glad.lib
│   │   └── Release/
│   └── CMakeFiles/
└── ... (source files)
```

### Build Artifacts

#### Libraries
- `PyramidEngined.lib` - Debug build of the engine
- `PyramidEngine.lib` - Release build of the engine
- `glad.lib` - OpenGL loading library

#### Executables
- `BasicGame.exe` - Basic game example
- `BasicRendering.exe` - Rendering showcase example

#### Debug Files
- `*.pdb` - Debug symbol files (Debug configuration only)
- `*.ilk` - Incremental linker files

## Running Examples

### Basic Game Example

1. **Navigate to Build Directory**
   ```cmd
   cd build\bin\Debug
   ```

2. **Run Basic Game**
   ```cmd
   BasicGame.exe
   ```

3. **Expected Output**
   - Window opens with 3D scene
   - Colored cube rotating with camera movement
   - Console output showing engine initialization

### Basic Rendering Example

1. **Run Basic Rendering**
   ```cmd
   BasicRendering.exe
   ```

2. **Expected Output**
   - Window with rotating cube
   - Lighting effects and material animations
   - Performance statistics in console

## Troubleshooting

### Common Build Issues

#### CMake Generation Failed

**Problem**: CMake cannot find Visual Studio or tools
```
CMake Error: CMAKE_C_COMPILER not set
```

**Solution**:
1. Ensure Visual Studio 2022 is properly installed with C++ workload
2. Use Developer Command Prompt for VS 2022
3. Verify CMake is in PATH: `cmake --version`

#### Missing Windows SDK

**Problem**: Cannot find Windows SDK
```
Error: Windows SDK version 10.0 not found
```

**Solution**:
1. Open Visual Studio Installer
2. Modify Visual Studio 2022 installation
3. Select latest Windows 10/11 SDK
4. Install and restart

#### GLAD Compilation Error

**Problem**: OpenGL loader compilation fails
```
Error: Cannot find OpenGL headers
```

**Solution**:
1. Ensure graphics drivers are up to date
2. Verify OpenGL support: `OpenGL Extensions Viewer`
3. Check Windows SDK installation

#### Missing Debug Symbols

**Problem**: Cannot debug the engine
```
Warning: Cannot find or open PDB file
```

**Solution**:
1. Build Debug configuration: `cmake --build build --config Debug`
2. Ensure PDB files are in same directory as executables
3. Check Visual Studio debugger settings

### Runtime Issues

#### Application Failed to Start

**Problem**: Executable crashes on startup
```
The application was unable to start correctly (0xc000007b)
```

**Solution**:
1. Install Microsoft Visual C++ Redistributable 2022
2. Ensure all DLL dependencies are available
3. Run from Developer Command Prompt

#### OpenGL Context Creation Failed

**Problem**: Graphics initialization fails
```
Error: Failed to create OpenGL context
```

**Solution**:
1. Update graphics drivers to latest version
2. Verify OpenGL 3.3+ support in graphics card specifications
3. Run `dxdiag` to check graphics capabilities

#### Low Performance

**Problem**: Poor framerate or stuttering
```
Warning: Frame time spike: 50.0 ms
```

**Solution**:
1. Build Release configuration for performance testing
2. Ensure VSync is properly configured
3. Check GPU utilization and thermal throttling
4. Verify no background applications are interfering

### Performance Optimization

#### Build Performance

**Improve Build Times**:
1. Use Ninja generator: `cmake -G Ninja`
2. Enable parallel compilation: `cmake --build build -j 8`
3. Use SSD for source and build directories
4. Increase RAM allocation for Visual Studio

**Incremental Builds**:
1. Use Visual Studio's incremental build features
2. Avoid cleaning unless necessary
3. Build specific targets: `cmake --build build --target BasicGame`

#### Runtime Performance

**Debug vs Release Performance**:
- Debug builds are significantly slower due to optimizations being disabled
- Always use Release builds for performance testing
- RelWithDebInfo provides a good balance for profiling

**SIMD Optimizations**:
- Ensure SIMD is enabled: `-DPYRAMID_ENABLE_SIMD=ON`
- Check CPU feature detection in logs
- Verify compiler optimizations are enabled

## Advanced Configuration

### Custom Build Configurations

Create custom build configurations for specific needs:

```cmake
# Custom configuration for profiling
set(CMAKE_CXX_FLAGS_PROFILING "-O2 -g -DPYRAMID_ENABLE_PROFILING=1")
set(CMAKE_C_FLAGS_PROFILING "-O2 -g -DPYRAMID_ENABLE_PROFILING=1")
```

### Cross-Compilation (Planned)

Future support for cross-compilation to other platforms:

```cmd
# Example for future Linux cross-compilation
cmake -B build-linux -S . ^
    -DCMAKE_TOOLCHAIN_FILE=cmake/Linux-Cross.cmake ^
    -DPYRAMID_TARGET_PLATFORM=Linux
```

### Package Management Integration

Future integration with package managers:

```cmd
# Future vcpkg integration
cmake -B build -S . ^
    -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

## Continuous Integration

### GitHub Actions (Planned)

The repository will include GitHub Actions for automated building:

```yaml
# .github/workflows/build.yml (example)
name: Build
on: [push, pull_request]
jobs:
  build:
    runs-on: windows-latest
    steps:
    - uses: actions/checkout@v3
    - name: Setup CMake
      uses: jwlawson/actions-setup-cmake@v1.12
    - name: Configure
      run: cmake -B build -S .
    - name: Build
      run: cmake --build build --config Release
```

## Next Steps

After successfully building the engine:

1. **Run Examples**: Try the example projects to understand the engine
2. **Read Documentation**: Explore the API reference and architecture docs
3. **Create Your First Game**: Follow the Getting Started guide
4. **Join the Community**: Participate in discussions and contribute

## Getting Help

If you encounter issues during building:

1. **Check Documentation**: Review this guide and the troubleshooting section
2. **Search Issues**: Look through GitHub Issues for similar problems
3. **Ask for Help**: Create a new issue with:
   - Your operating system and version
   - Visual Studio version and components
   - CMake version
   - Full error messages and logs
   - Steps to reproduce the problem

## Contributing to the Build System

If you'd like to improve the build system:

1. **Understand CMake**: Familiarize yourself with CMake best practices
2. **Test Changes**: Ensure changes work across different configurations
3. **Documentation**: Update this guide when making changes
4. **Pull Requests**: Submit improvements via GitHub pull requests

For more information about contributing, see [Contributing.md](Contributing.md).
