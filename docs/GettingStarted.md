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

1. Create a new class inheriting from `Pyramid::OglGame`:

```cpp
#include <OpenGL3D/Game/OglGame.h>

class MyGame : public Pyramid::OglGame {
public:
    // Choose your graphics API (OpenGL is default)
    MyGame() : OglGame(Pyramid::GraphicsAPI::OpenGL) {}

    void onCreate() override {
        // Initialize your game resources
    }

    void onUpdate() override {
        // Call base class update first
        OglGame::onUpdate();
        
        // Add your game logic here
    }

    void onQuit() override {
        // Clean up your resources
        OglGame::onQuit();
    }
};
```

2. Create the main entry point:

```cpp
int main() {
    MyGame game;
    game.run();
    return 0;
}
```

## Graphics API Support

### OpenGL
- Default graphics API
- Supports versions 3.3 through 4.6
- Automatic version detection and fallback

### DirectX (Planned)
- DirectX 9.0c support coming soon
- Future support for DirectX 10/11/12
- Unified API interface

## Best Practices

1. **Graphics API Independence**
   ```cpp
   // Good - Use the abstract interface
   m_GraphicsDevice->Clear(Color(0.0f, 0.0f, 0.2f, 1.0f));

   // Bad - Direct API calls
   glClear(GL_COLOR_BUFFER_BIT);
   ```

2. **Resource Management**
   ```cpp
   // Coming soon: Resource handle system
   ResourceHandle texture = resourceManager.CreateTexture(desc);
   ```

3. **Error Handling**
   ```cpp
   // Always check initialization
   if (!m_GraphicsDevice->Initialize()) {
       // Handle error
       return false;
   }
   ```

## Project Structure

Organize your game project like this:
```
MyGame/
├── src/
│   ├── Game.h        # Your game class
│   ├── Game.cpp      # Implementation
│   └── Main.cpp      # Entry point
├── assets/
│   ├── textures/
│   ├── models/
│   └── shaders/
└── CMakeLists.txt
```

## Getting Help

1. Check the documentation in the `docs/` directory
2. Look at example code in the `Game/` directory
3. Review the Architecture.md file
4. Submit issues on GitHub

## Common Issues

1. **OpenGL Version**
   - Error: "Failed to create OpenGL context"
   - Solution: Check your graphics driver supports OpenGL 3.3+

2. **Build Issues**
   - Error: "CMake not found"
   - Solution: Install CMake 3.16.0 or later

3. **Runtime Errors**
   - Error: "Failed to create graphics device"
   - Solution: Ensure graphics drivers are up to date

## Next Steps

1. Review the Architecture.md document
2. Explore the example game code
3. Try creating different graphics effects
4. Experiment with the upcoming DirectX support

## Contributing

1. Fork the repository
2. Create a feature branch
3. Submit a pull request
4. Follow the coding standards in Architecture.md
