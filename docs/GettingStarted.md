# Getting Started with Pyramid Game Engine

## Prerequisites

Before you begin, ensure you have the following installed:
- Visual Studio 2019 or later
- CMake 3.16 or later
- Git
- OpenGL-capable graphics card with drivers supporting OpenGL 4.0+

## Building the Engine

1. Clone the repository:
```bash
git clone https://github.com/yourusername/Pyramid.git
cd Pyramid
```

2. Generate the project files:
```bash
cmake -B build -S .
```

3. Build the project:
```bash
cmake --build build --config Debug  # or Release
```

## Project Structure

The engine is organized into several key components:

### OpenGL3D Library
- Core engine functionality
- Graphics and window management
- Game loop and event handling

### Game Project
- Example game implementation
- Starting point for your own games
- Demonstrates engine usage

## Creating Your First Game

1. Create a new class inheriting from `OglGame`:
```cpp
class MyGame : public OglGame {
public:
    void onCreate() override {
        // Initialize your game
    }

    void onUpdate() override {
        // Update game logic
    }

    void onQuit() override {
        // Cleanup resources
    }
};
```

2. Initialize and run your game:
```cpp
int main() {
    try {
        MyGame game;
        game.run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

## Best Practices

1. **Resource Management**
   - Use RAII principles
   - Prefer smart pointers
   - Clean up resources in onQuit()

2. **Error Handling**
   - Use try-catch blocks for error handling
   - Log errors appropriately
   - Clean up resources on error

3. **Performance**
   - Minimize state changes
   - Batch similar operations
   - Use appropriate data structures

## Getting Help

- Check the documentation in the `docs` folder
- Review the example game implementation
- Look at the API headers in `OpenGL3D/include`
