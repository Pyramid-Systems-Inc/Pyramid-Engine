# Basic Rendering Example

This example demonstrates the fundamental rendering capabilities of the Pyramid Engine. It provides a simple, educational starting point for new users to understand the core concepts of 3D rendering with the engine.

## Features Demonstrated

- **Basic 3D Rendering**: Renders a colorful 3D cube with proper depth testing
- **Shader System**: Uses vertex and fragment shaders with GLSL 4.60
- **Uniform Buffers**: Efficient data transfer from CPU to GPU using uniform buffer objects
- **Camera System**: Implements an orbital camera that rotates around the cube
- **Basic Lighting**: Simple Phong lighting model with diffuse, ambient, and specular components
- **Material System**: Animated material properties (base color, emissive color, metallic, roughness)
- **Math Library**: Uses the Pyramid Engine's math library for vector and matrix operations

## Key Components

### 1. Shader System
The example uses a simple shader pair that demonstrates:
- Vertex transformation using view-projection matrices
- Basic lighting calculations
- Material property integration
- Time-based animations

### 2. Geometry Creation
Creates a colorful cube with:
- Position, normal, texture coordinate, and color attributes
- Proper face culling and winding order
- Indexed rendering for efficiency

### 3. Camera System
Implements an orbital camera that:
- Rotates around the cube at a fixed distance
- Maintains proper look-at orientation
- Uses perspective projection

### 4. Uniform Buffers
Uses two uniform buffer objects:
- **Scene UBO**: Contains view/projection matrices, camera position, lighting data
- **Material UBO**: Contains material properties that animate over time

## Building and Running

### Prerequisites
- CMake 3.16 or higher
- C++17 compatible compiler
- Pyramid Engine built and installed

### Build Instructions
1. Navigate to the `Examples/BasicRendering` directory
2. Create a build directory: `mkdir build && cd build`
3. Generate build files: `cmake ..`
4. Build the example: `cmake --build .`
5. Run the executable: `./BasicRenderingExample` (Windows) or `BasicRenderingExample` (Linux)

## Code Structure

```
Examples/BasicRendering/
├── BasicRendering.hpp    # Main class declaration
├── BasicRendering.cpp    # Implementation of rendering logic
├── Main.cpp             # Entry point
├── CMakeLists.txt       # Build configuration
└── README.md            # This file
```

## Key Classes and Methods

### BasicRendering Class
- `onCreate()`: Initializes shaders, geometry, camera, and uniform buffers
- `onUpdate()`: Updates camera position and uniform buffer data
- `onRender()`: Renders the scene with proper state management

### Helper Methods
- `InitializeShaders()`: Creates and compiles the shader program
- `CreateGeometry()`: Generates the cube mesh with vertex attributes
- `SetupCamera()`: Initializes the orbital camera
- `SetupUniformBuffers()`: Creates and initializes UBOs
- `UpdateCamera()`: Updates camera position for orbital movement
- `UpdateUniformBuffers()`: Updates UBO data with animations
- `RenderScene()`: Performs the actual rendering

## Learning Points

This example teaches several important concepts:

1. **Engine Initialization**: How to properly initialize the Pyramid Engine
2. **Resource Management**: Creating and managing shaders, buffers, and geometry
3. **Rendering Pipeline**: Understanding the basic rendering flow
4. **Shader Programming**: Writing GLSL shaders for basic lighting
5. **Math Integration**: Using the engine's math library for transformations
6. **Camera Systems**: Implementing different camera behaviors
7. **Uniform Buffers**: Efficient GPU data transfer patterns

## Customization Ideas

Try modifying the example to experiment with:

1. **Different Shapes**: Replace the cube with a sphere, cylinder, or other primitives
2. **Camera Modes**: Implement different camera behaviors (first-person, third-person, etc.)
3. **Lighting Models**: Implement more advanced lighting like PBR
4. **Textures**: Add texture mapping to the cube
5. **Animation**: Add vertex animations or skeletal animation
6. **Post-processing**: Add simple post-processing effects

## Next Steps

After understanding this basic example, you can explore:

1. **Advanced 3D Scene Example**: More complex scenes with multiple objects
2. **Math Library Showcase**: Demonstrating SIMD optimizations
3. **Scene Management Demo**: Using the engine's scene management system
4. **Performance Benchmark Example**: Measuring and optimizing performance

## Troubleshooting

If you encounter issues:

1. **Build Errors**: Ensure Pyramid Engine is properly installed and all dependencies are available
2. **Runtime Errors**: Check that your graphics drivers support OpenGL 4.6
3. **Black Screen**: Verify that the shader compilation succeeded and no OpenGL errors occurred
4. **Poor Performance**: Check that vsync is enabled and your system meets the minimum requirements

For more information, refer to the main Pyramid Engine documentation.