# Getting Started with Pyramid Game Engine

## Prerequisites

- Windows 10 or later
- Visual Studio 2022
- CMake 3.16.0 or later
- Graphics card supporting:
  - OpenGL 3.3 or later (primary)
  - DirectX 9.0c or later (planned)

## Building the Engine

1. Clone the repository:
```bash
git clone https://github.com/yourusername/Pyramid.git
cd Pyramid
```

2. Generate project files:
```bash
cmake -B build -S .
```

3. Build the engine:
```bash
cmake --build build --config Debug
```

## Creating Your First Game

1. Create a new class inheriting from `Pyramid::Game`:

```cpp
#include <Pyramid/Core/Game.hpp>

class MyGame : public Pyramid::Game {
public:
    MyGame() = default;

    void onCreate() override {
        // Initialize your game resources
    }

    void onUpdate() override {
        // Add your game logic here
    }

    void onQuit() override {
        // Clean up your resources
    }
};
```

2. Create the main entry point (Windows):

```cpp
#include <windows.h>

int WINAPI WinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow)
{
    try
    {
        MyGame game;
        game.run();
        return 0;
    }
    catch (const std::exception& e)
    {
        MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}
```

## Window Management

The engine provides a robust window management system:

### Basic Window Management
```cpp
// Window is automatically created and managed by Game
// You can customize window creation by overriding:
void onCreate() override {
    // Set window title
    setWindowTitle("My Game");
    
    // Set window size
    setWindowSize(1280, 720);
}
```

### Window Events
```cpp
// Handle window events in your game class
void onWindowEvent(const WindowEvent& event) override {
    switch (event.type) {
    case WindowEvent::Resize:
        // Handle window resize
        break;
    case WindowEvent::Focus:
        // Handle window focus change
        break;
    case WindowEvent::Close:
        // Handle window close request
        break;
    }
}
```

## Graphics

The engine provides a flexible graphics abstraction layer:

### Graphics Device
```cpp
// Graphics device is automatically created and managed by Game
// You can access it in your game class:
void onCreate() override {
    auto* device = getGraphicsDevice();
    
    // Create graphics resources
    auto vertexBuffer = device->createVertexBuffer();
    auto shader = device->createShader();
}
```

### Shaders
```cpp
// Create and use shaders
void onCreate() override {
    auto* device = getGraphicsDevice();
    
    // Create shader
    auto shader = device->createShader();
    shader->setVertexSource(vertexShaderSource);
    shader->setFragmentSource(fragmentShaderSource);
    shader->compile();
    
    // Use shader
    shader->bind();
}
```

### Vertex Buffers
```cpp
// Create and use vertex buffers
void onCreate() override {
    auto* device = getGraphicsDevice();
    
    // Create vertex buffer
    auto vertexBuffer = device->createVertexBuffer();
    vertexBuffer->setData(vertices, sizeof(vertices));
    
    // Use vertex buffer
    vertexBuffer->bind();
}
```

## Scene Management

The engine features a production-ready scene management system with spatial partitioning:

### Basic Scene Management
```cpp
#include <Pyramid/Graphics/Scene/SceneManager.hpp>

void onCreate() override {
    using namespace Pyramid::SceneManagement;

    // Create scene manager
    m_sceneManager = SceneUtils::CreateSceneManager();

    // Create and set active scene
    auto mainScene = m_sceneManager->CreateScene("MainLevel");
    m_sceneManager->SetActiveScene(mainScene);

    // Configure spatial partitioning
    m_sceneManager->SetOctreeDepth(8);  // 8 levels deep
    m_sceneManager->SetOctreeSize(Math::Vec3(1000.0f, 1000.0f, 1000.0f));
}
```

### Spatial Queries
```cpp
void onUpdate(float deltaTime) override {
    // Find objects near player
    Math::Vec3 playerPos = getPlayerPosition();
    auto nearbyObjects = m_sceneManager->GetObjectsInRadius(playerPos, 10.0f);

    // Find nearest interactive object
    auto nearest = m_sceneManager->GetNearestObject(playerPos);
    if (nearest) {
        // Handle interaction
    }

    // Get visible objects for rendering
    auto visibleObjects = m_sceneManager->GetVisibleObjects(camera);

    // Update scene
    m_sceneManager->Update(deltaTime);
}
```

### Performance Monitoring
```cpp
void logPerformanceStats() {
    const auto& stats = m_sceneManager->GetStats();
    PYRAMID_LOG_INFO("Scene Performance:");
    PYRAMID_LOG_INFO("  Total Objects: ", stats.totalObjects);
    PYRAMID_LOG_INFO("  Visible Objects: ", stats.visibleObjects);
    PYRAMID_LOG_INFO("  Query Time: ", stats.lastQueryTime, " ms");
}
```

## Project Structure

A typical Pyramid game project structure looks like this:

```
MyGame/
├── include/
│   └── MyGame.hpp
├── source/
│   ├── MyGame.cpp
│   └── Main.cpp
└── CMakeLists.txt
```

Example CMakeLists.txt:
```cmake
add_executable(MyGame WIN32
    source/Main.cpp
    source/MyGame.cpp
    include/MyGame.hpp
)

target_link_libraries(MyGame PRIVATE PyramidEngine)

target_include_directories(MyGame
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

## Next Steps

1. Check out the example projects in the `Examples/` directory
2. Read the API documentation
3. Join our community:
   - GitHub Discussions
   - Discord Server
   - Forums

## Getting Help

If you need help:
1. Check the documentation
2. Look at the example projects
3. Ask in our community channels
4. File an issue on GitHub
