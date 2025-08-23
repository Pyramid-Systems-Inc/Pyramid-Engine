# Game Class API Reference

## Overview

The `Game` class is the core entry point for Pyramid Engine applications. It manages the main game loop, graphics device initialization, and provides virtual methods for game-specific logic implementation.

## Class Declaration

```cpp
namespace Pyramid {
    class Game {
    public:
        explicit Game(GraphicsAPI api = GraphicsAPI::OpenGL);
        virtual ~Game();
        
        void run();
        void quit();
        
    protected:
        virtual void onCreate();
        virtual void onUpdate(float deltaTime);
        virtual void onRender();
        
        IGraphicsDevice* GetGraphicsDevice() const;
        
    private:
        std::unique_ptr<Window> m_window;
        std::unique_ptr<IGraphicsDevice> m_graphicsDevice;
        bool m_isRunning;
    };
}
```

## Constructor

### Game(GraphicsAPI api)

Creates a new game instance with the specified graphics API.

**Parameters:**
- `api`: The graphics API to use (defaults to `GraphicsAPI::OpenGL`)

**Example:**
```cpp
// Use default OpenGL
class MyGame : public Pyramid::Game {
public:
    MyGame() : Game() {}  // Uses OpenGL by default
};

// Explicitly specify graphics API
class MyGame : public Pyramid::Game {
public:
    MyGame() : Game(Pyramid::GraphicsAPI::OpenGL) {}
};
```

## Public Methods

### run()

Starts the main game loop. This method blocks until the game is quit.

```cpp
void run();
```

**Example:**
```cpp
int main() {
    MyGame game;
    game.run();  // Blocks until game exits
    return 0;
}
```

### quit()

Signals the game loop to stop gracefully.

```cpp
void quit();
```

**Example:**
```cpp
void MyGame::onUpdate(float deltaTime) {
    if (inputManager.IsKeyPressed(Key::Escape)) {
        quit();  // Exit the game
    }
}
```

## Protected Virtual Methods

### onCreate()

Called once when the game engine is first created. Override this method to initialize your game resources.

```cpp
virtual void onCreate();
```

**Example:**
```cpp
void MyGame::onCreate() {
    // Initialize game resources
    auto* device = GetGraphicsDevice();
    
    // Create shaders
    m_shader = device->CreateShader();
    m_shader->Compile(vertexSource, fragmentSource);
    
    // Create geometry
    CreateGameObjects();
    
    // Setup camera
    m_camera.SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
    m_camera.LookAt(Math::Vec3::Zero);
    
    PYRAMID_LOG_INFO("Game initialized successfully");
}
```

### onUpdate(float deltaTime)

Called every frame to update game logic. Override this method to implement your game update logic.

**Parameters:**
- `deltaTime`: Time elapsed since the last update in seconds

```cpp
virtual void onUpdate(float deltaTime);
```

**Example:**
```cpp
void MyGame::onUpdate(float deltaTime) {
    // Update game logic
    m_player.Update(deltaTime);
    m_enemies.Update(deltaTime);
    
    // Update camera
    UpdateCamera(deltaTime);
    
    // Check game conditions
    if (m_player.GetHealth() <= 0) {
        GameOver();
    }
}
```

### onRender()

Called every frame to render the game. Override this method to implement your game rendering.

```cpp
virtual void onRender();
```

**Example:**
```cpp
void MyGame::onRender() {
    auto* device = GetGraphicsDevice();
    
    // Clear the screen
    device->Clear(Color(0.1f, 0.1f, 0.1f, 1.0f));
    
    // Set up rendering state
    m_shader->Bind();
    m_shader->SetUniform("u_ViewProjection", m_camera.GetViewProjectionMatrix());
    
    // Render game objects
    m_player.Render(device);
    m_enemies.Render(device);
    m_environment.Render(device);
    
    // Present the frame
    device->Present(true);  // Enable VSync
}
```

## Protected Methods

### GetGraphicsDevice()

Returns a pointer to the graphics device instance.

```cpp
IGraphicsDevice* GetGraphicsDevice() const;
```

**Returns:** Pointer to the graphics device

**Example:**
```cpp
void MyGame::onCreate() {
    auto* device = GetGraphicsDevice();
    if (!device) {
        PYRAMID_LOG_ERROR("Graphics device not available");
        return;
    }
    
    // Use the graphics device
    auto shader = device->CreateShader();
    auto buffer = device->CreateVertexBuffer();
}
```

## Graphics API Support

The Game class supports multiple graphics APIs through the `GraphicsAPI` enumeration:

```cpp
enum class GraphicsAPI {
    OpenGL,    // OpenGL 3.3-4.6 (currently supported)
    DirectX9,  // DirectX 9 (planned)
    DirectX11, // DirectX 11 (planned)
    DirectX12, // DirectX 12 (planned)
    Vulkan     // Vulkan (planned)
};
```

## Game Loop Architecture

The game loop follows a standard pattern:

1. **Initialization**: `onCreate()` is called once
2. **Main Loop**: Repeats until quit is called:
   - Process window events
   - Calculate delta time
   - Call `onUpdate(deltaTime)`
   - Call `onRender()`
3. **Cleanup**: Automatic resource cleanup on destruction

```cpp
// Simplified game loop pseudocode
void Game::run() {
    onCreate();
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (m_isRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Process window events
        m_window->ProcessEvents();
        
        // Update and render
        onUpdate(deltaTime);
        onRender();
    }
    
    // Cleanup happens in destructor
}
```

## Complete Example

Here's a complete minimal game implementation:

```cpp
#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Math/Math.hpp>

class HelloTriangleGame : public Pyramid::Game {
public:
    HelloTriangleGame() : Game(Pyramid::GraphicsAPI::OpenGL) {}

protected:
    void onCreate() override {
        auto* device = GetGraphicsDevice();
        
        // Create shader
        m_shader = device->CreateShader();
        const char* vertexSource = R"(
            #version 330 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Color;
            
            uniform mat4 u_ViewProjection;
            out vec3 v_Color;
            
            void main() {
                gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
                v_Color = a_Color;
            }
        )";
        
        const char* fragmentSource = R"(
            #version 330 core
            in vec3 v_Color;
            out vec4 o_Color;
            
            void main() {
                o_Color = vec4(v_Color, 1.0);
            }
        )";
        
        m_shader->SetVertexSource(vertexSource);
        m_shader->SetFragmentSource(fragmentSource);
        m_shader->Compile();
        
        // Create triangle vertices
        float vertices[] = {
            // Position         // Color
           -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // Red
            0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // Green
            0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // Blue
        };
        
        m_vertexBuffer = device->CreateVertexBuffer();
        m_vertexBuffer->SetData(vertices, sizeof(vertices));
        
        // Set up vertex layout
        Pyramid::BufferLayout layout = {
            {Pyramid::ShaderDataType::Float3, "a_Position"},
            {Pyramid::ShaderDataType::Float3, "a_Color"}
        };
        m_vertexBuffer->SetLayout(layout);
        
        m_vertexArray = device->CreateVertexArray();
        m_vertexArray->AddVertexBuffer(m_vertexBuffer);
        
        // Setup camera
        m_camera = Pyramid::Camera(
            Pyramid::Math::Radians(60.0f),
            1280.0f / 720.0f,
            0.1f, 100.0f
        );
        m_camera.SetPosition(Pyramid::Math::Vec3(0.0f, 0.0f, 3.0f));
        
        PYRAMID_LOG_INFO("Hello Triangle game created!");
    }
    
    void onUpdate(float deltaTime) override {
        // Rotate triangle
        m_rotation += deltaTime;
    }
    
    void onRender() override {
        auto* device = GetGraphicsDevice();
        
        device->Clear(Pyramid::Color(0.2f, 0.2f, 0.2f, 1.0f));
        
        // Create rotation matrix
        auto rotationMatrix = Pyramid::Math::Mat4::CreateRotationZ(m_rotation);
        auto mvp = m_camera.GetViewProjectionMatrix() * rotationMatrix;
        
        m_shader->Bind();
        m_shader->SetUniform("u_ViewProjection", mvp);
        
        m_vertexArray->Bind();
        device->DrawArrays(3);
        
        device->Present(true);
    }

private:
    std::shared_ptr<Pyramid::IShader> m_shader;
    std::shared_ptr<Pyramid::IVertexBuffer> m_vertexBuffer;
    std::shared_ptr<Pyramid::IVertexArray> m_vertexArray;
    Pyramid::Camera m_camera;
    float m_rotation = 0.0f;
};

// Entry point
int main() {
    try {
        HelloTriangleGame game;
        game.run();
        return 0;
    } catch (const std::exception& e) {
        PYRAMID_LOG_ERROR("Game crashed: ", e.what());
        return 1;
    }
}
```

## Error Handling

The Game class provides basic error handling:

```cpp
void MyGame::onCreate() {
    auto* device = GetGraphicsDevice();
    if (!device) {
        PYRAMID_LOG_ERROR("Failed to get graphics device");
        quit();  // Exit gracefully
        return;
    }
    
    // Continue with initialization...
}
```

## Performance Considerations

### Delta Time Usage
- Always use `deltaTime` for frame-rate independent updates
- Clamp delta time to prevent large jumps (e.g., when debugging)

```cpp
void MyGame::onUpdate(float deltaTime) {
    // Clamp delta time to prevent physics issues
    deltaTime = std::min(deltaTime, 1.0f / 30.0f);  // Max 30 FPS minimum
    
    // Use delta time for all updates
    m_player.Move(velocity * deltaTime);
}
```

### Resource Management
- Initialize heavy resources in `onCreate()`
- Use smart pointers for automatic cleanup
- Avoid creating resources in the render loop

## See Also

- [Graphics Device API](../Graphics/GraphicsDevice.md) - Graphics device interface
- [Window Management](../Platform/Window.md) - Window and input handling
- [Camera System](../Graphics/Camera.md) - Camera management
- [Shader System](../Graphics/Shader.md) - Shader creation and management
