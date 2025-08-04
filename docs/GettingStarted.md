# Getting Started with Pyramid Game Engine

## Prerequisites

### System Requirements
- **Operating System**: Windows 10 or later (64-bit)
- **Processor**: x86-64 CPU with SSE2 support
- **Memory**: 8 GB RAM minimum, 16 GB recommended
- **Graphics**: Graphics card supporting OpenGL 3.3 or later
- **Storage**: 2 GB available space

### Development Tools
- **Visual Studio 2022**: Community Edition or higher
  - C++ CMake tools for Visual Studio
  - Windows 10/11 SDK
- **CMake**: Version 3.16.0 or later
- **Git**: For version control

### Optional Tools
- **Visual Studio Code**: With C++ and CMake extensions
- **GitKraken**: Git GUI client (optional)
- **Notepad++**: Advanced text editor (optional)

## Installation

### Step 1: Install Visual Studio 2022
1. Download Visual Studio 2022 Community from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/)
2. Run the installer and select the "Desktop development with C++" workload
3. Ensure the following components are selected:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - Windows 10/11 SDK
   - C++ CMake tools for Visual Studio

### Step 2: Install CMake
1. Download CMake from [cmake.org](https://cmake.org/)
2. Run the installer and follow the setup wizard
3. Ensure CMake is added to your system PATH

### Step 3: Clone the Repository
1. Open Command Prompt or PowerShell
2. Navigate to your desired development directory
3. Clone the repository:
```bash
git clone https://github.com/yourusername/Pyramid.git
cd Pyramid
```

## Building the Engine

### Option 1: Using Visual Studio (Recommended)
1. Open Visual Studio 2022
2. Select "Open a local folder"
3. Navigate to and select the Pyramid directory
4. Visual Studio will automatically detect the CMakeLists.txt and configure the project
5. Wait for CMake generation to complete
6. In the toolbar, select the desired configuration (Debug, Release, RelWithDebInfo)
7. Build the solution by pressing Ctrl+Shift+B or selecting Build → Build Solution

### Option 2: Using Command Line
1. Open Command Prompt or PowerShell
2. Navigate to the Pyramid directory
3. Generate build files:
```bash
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
```
4. Build the engine:
```bash
cmake --build build --config Debug
```

### Build Configurations
- **Debug**: Includes debug symbols, no optimizations, full logging
- **Release**: Optimized for performance, minimal logging
- **RelWithDebInfo**: Optimized with debug symbols, useful for profiling

### Build Output
After successful build, you will find:
- **Static Library**: `build/lib/PyramidEngine.lib`
- **Dynamic Library**: `build/bin/Debug/PyramidEngine.dll` (Debug configuration)
- **Example Executables**: `build/bin/Debug/Examples/BasicGame.exe`

## Creating Your First Game

### Basic Game Structure

#### Step 1: Create the Game Class
Create a new class inheriting from `Pyramid::Game`:

```cpp
// MyGame.hpp
#pragma once
#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Math/Math.hpp>

class MyGame : public Pyramid::Game {
public:
    MyGame();
    ~MyGame() = default;

protected:
    void onCreate() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;

private:
    // Graphics resources
    std::shared_ptr<Pyramid::IShader> m_shader;
    std::shared_ptr<Pyramid::IVertexBuffer> m_vertexBuffer;
    std::shared_ptr<Pyramid::IIndexBuffer> m_indexBuffer;
    std::shared_ptr<Pyramid::IVertexArray> m_vertexArray;
    
    // Camera
    Pyramid::Camera m_camera;
    
    // Timing
    f32 m_time = 0.0f;
};
```

#### Step 2: Implement the Game Class

```cpp
// MyGame.cpp
#include "MyGame.hpp"

MyGame::MyGame() : Pyramid::Game() {
    // Constructor - window properties are set automatically
}

void MyGame::onCreate() {
    auto* device = GetGraphicsDevice();
    
    // Create camera
    m_camera = Pyramid::Camera(
        Pyramid::Math::Radians(60.0f),  // FOV
        1280.0f / 720.0f,             // Aspect ratio
        0.1f,                         // Near plane
        1000.0f                       // Far plane
    );
    m_camera.SetPosition(Pyramid::Math::Vec3(0.0f, 0.0f, 5.0f));
    m_camera.LookAt(Pyramid::Math::Vec3::Zero);
    
    // Create shader
    m_shader = device->CreateShader();
    const char* vertexSource = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Color;
        
        uniform mat4 u_ViewProjection;
        uniform mat4 u_WorldMatrix;
        uniform float u_Time;
        
        out vec3 v_Color;
        
        void main() {
            gl_Position = u_ViewProjection * u_WorldMatrix * vec4(a_Position, 1.0);
            v_Color = a_Color;
        }
    )";
    
    const char* fragmentSource = R"(
        #version 330 core
        in vec3 v_Color;
        
        uniform float u_Time;
        
        out vec4 o_Color;
        
        void main() {
            // Animate color based on time
            vec3 animatedColor = v_Color * (0.5f + 0.5f * sin(u_Time));
            o_Color = vec4(animatedColor, 1.0f);
        }
    )";
    
    m_shader->setVertexSource(vertexSource);
    m_shader->setFragmentSource(fragmentSource);
    
    if (!m_shader->compile()) {
        PYRAMID_LOG_ERROR("Shader compilation failed: ", m_shader->getErrorMessage());
        return;
    }
    
    // Create vertex buffer
    struct Vertex {
        Pyramid::Math::Vec3 position;
        Pyramid::Math::Vec3 color;
    };
    
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Bottom-left - Red
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // Bottom-right - Green
        {{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Top - Blue
    };
    
    m_vertexBuffer = device->CreateVertexBuffer();
    m_vertexBuffer->setData(vertices.data(), vertices.size() * sizeof(Vertex));
    
    // Set up vertex layout
    Pyramid::BufferLayout layout = {
        {Pyramid::ShaderDataType::Float3, "a_Position"},
        {Pyramid::ShaderDataType::Float3, "a_Color"}
    };
    m_vertexBuffer->setLayout(layout);
    
    // Create index buffer
    std::vector<u32> indices = {0, 1, 2};  // Triangle indices
    m_indexBuffer = device->CreateIndexBuffer();
    m_indexBuffer->setData(indices.data(), indices.size() * sizeof(u32));
    
    // Create vertex array
    m_vertexArray = device->CreateVertexArray();
    m_vertexArray->addVertexBuffer(m_vertexBuffer);
    m_vertexArray->setIndexBuffer(m_indexBuffer);
    
    PYRAMID_LOG_INFO("Game created successfully!");
}

void MyGame::onUpdate(float deltaTime) {
    // Update timing
    m_time += deltaTime;
    
    // Rotate camera around the triangle
    f32 radius = 5.0f;
    f32 camX = sin(m_time * 0.5f) * radius;
    f32 camZ = cos(m_time * 0.5f) * radius;
    
    m_camera.SetPosition(Pyramid::Math::Vec3(camX, 0.0f, camZ));
    m_camera.LookAt(Pyramid::Math::Vec3::Zero);
}

void MyGame::onRender() {
    auto* device = GetGraphicsDevice();
    
    // Clear the screen
    device->Clear(Pyramid::Color(0.1f, 0.1f, 0.1f, 1.0f));
    
    // Set up rendering state
    device->EnableDepthTest(false);
    
    // Bind shader and set uniforms
    m_shader->bind();
    m_shader->setUniform("u_ViewProjection", m_camera.GetViewProjectionMatrix());
    m_shader->setUniform("u_WorldMatrix", Pyramid::Math::Mat4::Identity);
    m_shader->setUniform("u_Time", m_time);
    
    // Bind vertex array and draw
    m_vertexArray->bind();
    device->DrawIndexed(3);
}

```

#### Step 3: Create the Main Entry Point

```cpp
// Main.cpp
#include <windows.h>
#include "MyGame.hpp"

int WINAPI WinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow)
{
    try {
        MyGame game;
        game.run();
        return 0;
    }
    catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}
```

#### Step 4: Create CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16.0)
project(MyGame CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Pyramid Engine
find_package(PyramidEngine REQUIRED)

# Add executable
add_executable(MyGame WIN32
    source/Main.cpp
    source/MyGame.cpp
    include/MyGame.hpp
)

# Link with Pyramid Engine
target_link_libraries(MyGame PRIVATE PyramidEngine)

# Include directories
target_include_directories(MyGame
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Set output directory
set_target_properties(MyGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
)
```

### Project Structure

```
MyGame/
├── CMakeLists.txt
├── include/
│   └── MyGame.hpp
├── source/
│   ├── Main.cpp
│   └── MyGame.cpp
└── assets/
    ├── textures/
    ├── shaders/
    └── models/
```

## Hello Triangle Example

Here's a complete minimal example that renders a colored triangle:

```cpp
#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Math/Math.hpp>

class HelloTriangle : public Pyramid::Game {
public:
    void onCreate() override {
        auto* device = GetGraphicsDevice();
        
        // Create simple shader
        m_shader = device->CreateShader();
        m_shader->setVertexSource(R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Color;
            
            uniform mat4 u_ViewProjection;
            
            out vec3 v_Color;
            
            void main() {
                gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
                v_Color = a_Color;
            }
        )");
        
        m_shader->setFragmentSource(R"(
            #version 330 core
            in vec3 v_Color;
            out vec4 o_Color;
            
            void main() {
                o_Color = vec4(v_Color, 1.0);
            }
        )");
        
        m_shader->compile();
        
        // Triangle vertices with colors
        float vertices[] = {
            // Position         // Color
           -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // Bottom-left - Red
            0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // Bottom-right - Green
            0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // Top - Blue
        };
        
        m_vertexBuffer = device->CreateVertexBuffer();
        m_vertexBuffer->setData(vertices, sizeof(vertices));
        
        // Set up vertex layout
        Pyramid::BufferLayout layout = {
            {Pyramid::ShaderDataType::Float3, "a_Position"},
            {Pyramid::ShaderDataType::Float3, "a_Color"}
        };
        m_vertexBuffer->setLayout(layout);
        
        // Create vertex array
        m_vertexArray = device->CreateVertexArray();
        m_vertexArray->addVertexBuffer(m_vertexBuffer);
        
        // Set up camera
        m_camera = Pyramid::Camera(
            Pyramid::Math::Radians(60.0f),
            1280.0f / 720.0f,  // Fixed aspect ratio - window size methods don't exist
            0.1f, 100.0f
        );
        m_camera.SetPosition(Pyramid::Math::Vec3(0.0f, 0.0f, 3.0f));
    }
    
    void onRender() override {
        auto* device = GetGraphicsDevice();
        
        device->Clear(Pyramid::Color(0.2f, 0.2f, 0.2f, 1.0f));
        
        m_shader->bind();
        m_shader->setUniform("u_ViewProjection", m_camera.GetViewProjectionMatrix());
        
        m_vertexArray->bind();
        device->DrawArrays(3);
    }
    
private:
    std::shared_ptr<Pyramid::IShader> m_shader;
    std::shared_ptr<Pyramid::IVertexBuffer> m_vertexBuffer;
    std::shared_ptr<Pyramid::IVertexArray> m_vertexArray;
    Pyramid::Camera m_camera;
};

int main() {
    HelloTriangle game;
    game.run();
    return 0;
}
```

## Building and Running Your Game

### Using Visual Studio
1. Create a new directory for your game project
2. Copy the project structure shown above
3. Add your game project to the Pyramid solution:
   - Right-click the Solution in Solution Explorer
   - Add → Existing Project
   - Select your game's CMakeLists.txt
4. Set your game as the startup project
5. Build and run (F5)

### Using Command Line
1. Navigate to your game directory
2. Create a build directory:
```bash
mkdir build
cd build
```
3. Generate build files:
```bash
cmake .. -G "Visual Studio 17 2022" -A x64
```
4. Build:
```bash
cmake --build . --config Debug
```
5. Run:
```bash
./bin/Debug/MyGame.exe
```

## Common Issues and Solutions

### Build Errors
**Problem**: CMake cannot find Pyramid Engine
```
CMake Error at CMakeLists.txt:10 (find_package):
  Could not find a configuration file for "PyramidEngine"
```
**Solution**: Ensure the Pyramid Engine is built and installed, or specify the path:
```cmake
set(PyramidEngine_DIR "path/to/pyramid/build")
```

**Problem**: Linker errors unresolved external symbols
**Solution**: Ensure you're linking against the correct library configuration (Debug/Release) and that all dependencies are properly installed.

### Runtime Errors
**Problem**: Window creation fails
**Solution**: Ensure your graphics drivers are up to date and that your system supports OpenGL 3.3 or later.

**Problem**: Shader compilation fails
**Solution**: Check the shader error messages in the log output. Common issues include:
- Unsupported GLSL version
- Syntax errors in shader code
- Missing uniforms or attributes

### Performance Issues
**Problem**: Low frame rate
**Solution**:
- Use Release build configuration for better performance
- Enable VSync in present call
- Optimize shader code
- Reduce draw calls

## Next Steps

1. **Explore Examples**: Check the `Examples/` directory for more comprehensive examples
2. **Read API Documentation**: Refer to the API Reference for detailed class documentation
3. **Experiment with Features**: Try adding textures, lighting, and more complex geometry
4. **Join the Community**: Participate in discussions and get help from other developers

## Additional Resources

### Documentation
- [Architecture Overview](../Architecture.md) - Detailed engine architecture
- [API Reference](../API_Reference/) - Complete API documentation
- [Math Library](../API_Reference/Math/MathLibrary.md) - Math library reference
- [Graphics System](../API_Reference/Graphics/GraphicsDevice.md) - Graphics system reference

### Examples
- `Examples/BasicGame/` - Basic game example with input handling
- `Examples/AdvancedRendering/` - Advanced rendering techniques
- `Examples/SceneManagement/` - Scene management and spatial partitioning

### Community
- **GitHub**: [https://github.com/yourusername/Pyramid](https://github.com/yourusername/Pyramid)
- **Issues**: Report bugs and request features
- **Discussions**: Ask questions and share knowledge
- **Discord**: Real-time chat (coming soon)

## Getting Help

If you encounter issues or have questions:

1. **Check the Documentation**: Most common issues are covered in the documentation
2. **Search Existing Issues**: Check GitHub Issues for similar problems
3. **Create a Minimal Example**: Isolate the issue in a simple, reproducible example
4. **File an Issue**: Provide detailed information about your problem:
   - Operating system and version
   - Graphics card and driver version
   - Full error messages and stack traces
   - Minimal code that reproduces the issue

Happy coding with Pyramid Engine!

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

    void onUpdate(float deltaTime) override {
        // Add your game logic here
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
// Window customization is handled through the platform-specific window implementation
void onCreate() override {
    // Initialize your game resources
    // Window properties are set automatically during Game initialization
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
    auto* device = GetGraphicsDevice();
    
    // Create graphics resources
    auto vertexBuffer = device->CreateVertexBuffer();
    auto shader = device->CreateShader();
}
```

### Shaders
```cpp
// Create and use shaders
void onCreate() override {
    auto* device = GetGraphicsDevice();
    
    // Create shader
    auto shader = device->CreateShader();
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
    auto* device = GetGraphicsDevice();
    
    // Create vertex buffer
    auto vertexBuffer = device->CreateVertexBuffer();
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
