# Pyramid Game Engine

A modern, high-performance game engine designed for flexibility and performance. Built with C++17 and featuring SIMD-optimized math, advanced rendering, and comprehensive development tools.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Platform](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3%2B-green.svg)](https://www.opengl.org/)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](#building)

## ✨ Key Features

### 🚀 **High-Performance Math Library**
- **SIMD-Accelerated**: SSE/AVX optimized vector and matrix operations with runtime CPU detection
- **Complete 3D Math**: Vec2, Vec3, Vec4, Mat3, Mat4, Quaternion with full operator support
- **Cache-Friendly**: 16-byte aligned structures optimized for modern CPUs
- **Batch Processing**: Efficient processing of arrays for optimal SIMD utilization

### 🎨 **Advanced Graphics System**
- **Modern OpenGL**: 3.3-4.6 support with clean abstraction layer
  - **Command Buffer System**: Efficient GPU command submission and batching
- **Render Passes**: Organized rendering stages (forward, deferred, post-processing)
- **PBR Materials**: Physically-based rendering with metallic-roughness workflow
- **Advanced Camera**: Perspective/orthographic projections with frustum culling

### 🌍 **Intelligent Scene Management**
  - **Octree Spatial Partitioning**: O(log n) object queries with configurable depth
- **Multiple Query Types**: Point, sphere, box, ray, and frustum-based object lookup
- **Performance Monitoring**: Real-time statistics and optimization feedback
- **Memory Efficient**: Smart pointer-based RAII resource management

### 🖼️ **Zero-Dependency Image Loading**
- **Complete Format Support**: TGA, BMP, PNG (custom DEFLATE), JPEG (baseline DCT)
- **Self-Contained**: No external dependencies, production-ready implementations
- **Optimized Loading**: Direct GPU texture creation with efficient memory usage

### 📝 **Production Logging System**
- **Thread-Safe**: Mutex-protected concurrent logging operations
- **Multiple Outputs**: Console and file logging with independent level control
- **File Rotation**: Automatic log rotation with size limits and file count management
- **Structured Data**: Key-value pair logging for analytics and debugging

### 🏗️ **Modern C++ Architecture**
- **C++17 Standard**: Modern language features with RAII principles
- **Cross-Platform Ready**: Clean platform abstraction layer
- **Extensible Design**: Plugin-ready architecture for future expansion

## 📋 Requirements

### System Requirements
- **OS**: Windows 10/11 (64-bit)
- **CPU**: x64 with SSE2 support (AVX recommended for optimal SIMD performance)
- **GPU**: OpenGL 3.3+ compatible graphics card
- **RAM**: 8GB minimum, 16GB recommended

### Development Tools
- **Visual Studio 2022** (Community/Professional/Enterprise)
- **CMake 3.16+** (3.20+ recommended)
- **Git** for version control

## 🚀 Quick Start

### 1️⃣ Clone and Setup
```bash
git clone https://github.com/yourusername/Pyramid-Engine.git
cd Pyramid-Engine
```

### 2️⃣ Build (Command Line)
```bash
# Generate build files
cmake -B build -S . -DPYRAMID_BUILD_EXAMPLES=ON

# Build the engine
cmake --build build --config Debug
```

### 3️⃣ Build (Visual Studio)
1. Open Visual Studio 2022
2. Select **"Open a local folder"**
3. Choose the `Pyramid-Engine` directory
4. Wait for CMake configuration
5. Build → Build All (Ctrl+Shift+B)

### 4️⃣ Run Examples
```bash
cd build\bin\Debug
BasicGame.exe          # Basic game with 3D scene
BasicRendering.exe     # Advanced rendering showcase
```

> 💡 **Need help?** See our [**Build Guide**](docs/BuildGuide.md) for detailed setup instructions.

## 💻 Your First Game

Create a simple game in just a few lines:

```cpp
#include <Pyramid/Core/Game.hpp>

class MyGame : public Pyramid::Game {
public:
    void onCreate() override {
        // Initialize your game
        PYRAMID_LOG_INFO("Game starting up!");
    }
    
    void onUpdate(float deltaTime) override {
        // Update game logic
    }
    
    void onRender() override {
        // Render your scene
        auto* device = GetGraphicsDevice();
        device->Clear(Pyramid::Color(0.2f, 0.3f, 0.4f, 1.0f));
    }
};

int main() {
    MyGame game;
    game.run();
    return 0;
}
```

### 🔧 Advanced Features

#### SIMD-Optimized Math
```cpp
#include <Pyramid/Math/Math.hpp>

using namespace Pyramid::Math;

// Automatic SIMD acceleration
Vec3 position(1.0f, 2.0f, 3.0f);
Vec3 velocity = Vec3::Forward * speed;
Vec3 newPosition = position + velocity * deltaTime;

// 3x faster than standard math libraries
Mat4 mvp = projection * view * model;
```

#### Intelligent Scene Management
```cpp
#include <Pyramid/Graphics/Scene/SceneManager.hpp>

// Octree spatial partitioning for massive worlds
auto sceneManager = SceneUtils::CreateSceneManager();
sceneManager->EnableSpatialPartitioning(true);

// Lightning-fast spatial queries
auto nearbyEnemies = sceneManager->GetObjectsInRadius(playerPos, 10.0f);
auto visibleObjects = sceneManager->GetVisibleObjects(camera);
```

#### Production-Ready Logging
```cpp
#include <Pyramid/Util/Log.hpp>

// Thread-safe, high-performance logging
PYRAMID_LOG_INFO("Player scored: ", score, " points!");
PYRAMID_LOG_ERROR("Failed to load texture: ", filename);

// Structured logging for analytics
PYRAMID_LOG_STRUCTURED(LogLevel::Info, "Level completed", {
    {"level", "forest_1"}, {"time", "120.5"}, {"score", "1500"}
});
```

## 📚 Documentation

| **Getting Started** | **API Reference** | **Advanced** |
|:---|:---|:---|
| [🚀 Getting Started](docs/GettingStarted.md) | [📖 Complete API Reference](docs/API_Reference/) | [🏗️ Architecture Guide](docs/Architecture.md) |
| [🔨 Build Guide](docs/BuildGuide.md) | [🎮 Core Game Engine](docs/API_Reference/Core/Game.md) | [🤝 Contributing Guide](docs/Contributing.md) |
| [🎯 Examples & Tutorials](docs/Examples/) | [🎨 Graphics System](docs/API_Reference/Graphics/) | [📋 Project Roadmap](PYRAMID_ENGINE_MASTER_ROADMAP.md) |
| | [🧮 Math Library](docs/API_Reference/Math/MathLibrary.md) | [📝 Changelog](CHANGELOG.md) |
| | [🔧 Utilities](docs/API_Reference/Utils/) | |

### Quick Navigation
- **New to game engines?** → Start with [Getting Started](docs/GettingStarted.md)
- **Ready to build?** → Follow the [Build Guide](docs/BuildGuide.md)  
- **Want to contribute?** → Read [Contributing Guidelines](docs/Contributing.md)
- **Need API docs?** → Browse [API Reference](docs/API_Reference/)
- **Looking for examples?** → Check [Examples & Tutorials](docs/Examples/)

## 🏗️ Project Structure

```text
Pyramid-Engine/
├── 🎮 Engine/                    # Core engine library
│   ├── Core/                    # Game loop and foundation
│   ├── Graphics/                # Rendering and visual systems
│   │   ├── Scene/              # Scene management with octrees
│   │   ├── Renderer/           # Command buffers and render passes
│   │   └── OpenGL/             # OpenGL 3.3-4.6 implementation
│   ├── Math/                   # SIMD-optimized mathematics
│   ├── Platform/               # Cross-platform abstractions
│   ├── Utils/                  # Logging and image loading
│   ├── Audio/                  # 3D spatial audio (planned)
│   ├── Physics/                # Rigid body dynamics (planned)
│   └── Input/                  # Multi-device input (planned)
├── 🎯 Examples/                  # Learning examples
│   ├── BasicGame/              # Your first game
│   └── BasicRendering/         # Advanced graphics showcase
├── 📚 docs/                      # Comprehensive documentation
├── 🛠️ vendor/                    # Third-party dependencies
└── 🔧 CMake build system         # Multi-platform builds
```

## 🚦 Development Status

| Module | Status | Documentation | Examples |
|:-------|:------:|:-------------:|:--------:|
| 🎮 **Core Engine** | ✅ **Complete** | ✅ | ✅ |
| 🎨 **Graphics System** | ✅ **Complete** | ✅ | ✅ |
| 🧮 **Math Library** | ✅ **Complete** | ✅ | ✅ |
| 🖼️ **Image Loading** | ✅ **Complete** | ✅ | ✅ |
| 📝 **Logging System** | ✅ **Complete** | ✅ | ✅ |
| 🔧 **Platform Layer** | ✅ **Complete** | ✅ | ✅ |
| 🎵 **Audio System** | 🔄 **In Progress** | ✅ | ⏳ |
| ⚡ **Physics Engine** | 🔄 **In Progress** | ✅ | ⏳ |
| 🎮 **Input System** | 🔄 **In Progress** | ✅ | ⏳ |

## 🤝 Contributing

We welcome contributions! Whether you're fixing bugs, adding features, or improving documentation, your help makes Pyramid Engine better for everyone.

### Ways to Contribute
- 🐛 **Report bugs** and suggest features via [GitHub Issues](https://github.com/yourusername/Pyramid-Engine/issues)
- 💻 **Submit code** improvements and new features
- 📚 **Improve documentation** and write tutorials
- 🎮 **Create examples** to help other developers learn

**Ready to contribute?** Read our [Contributing Guide](docs/Contributing.md) for detailed instructions.

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

```
MIT License - Free for commercial and non-commercial use
✅ Use in commercial projects    ✅ Modify and redistribute
✅ Private use                   ✅ Include copyright notice
```

## 🙏 Acknowledgments

- **GLAD** - OpenGL function loading
- **Microsoft** - Visual Studio and Windows API
- **CMake** - Cross-platform build system
- **Community contributors** - Bug reports, features, and feedback

---

<div align="center">

**⭐ Star this repo if you find it useful! ⭐**

[Report Bug](https://github.com/yourusername/Pyramid-Engine/issues) · [Request Feature](https://github.com/yourusername/Pyramid-Engine/issues) · [Documentation](docs/) · [Examples](docs/Examples/)

**Built with ❤️ for the game development community**

</div>
