# Pyramid Engine Examples and Tutorials

## Overview

This directory contains comprehensive examples and tutorials for the Pyramid Engine. Each example is designed to demonstrate specific features and provide educational value for developers learning the engine.

## Getting Started

Before running the examples, ensure you have:

1. **Built the Engine**: Follow the [Build Guide](../BuildGuide.md) to compile the engine
2. **Working Environment**: Visual Studio 2022 or compatible development environment
3. **Graphics Support**: OpenGL 3.3+ compatible graphics card with updated drivers

## Example Projects

### Basic Examples

#### 1. BasicGame
**Location**: `Examples/BasicGame/`  
**Difficulty**: Beginner  
**Concepts**: Core engine usage, game loop, basic rendering

**Description**:
The BasicGame example demonstrates the fundamental concepts of the Pyramid Engine. It showcases:
- Engine initialization and shutdown
- Basic game loop implementation
- Simple 3D rendering with shaders
- Camera system usage
- Input handling
- Resource management

**What You'll Learn**:
- How to create a `Game` class and override core methods
- Setting up graphics device and shaders
- Creating and managing 3D objects
- Basic camera controls and movement
- Event handling and user input

**Code Highlights**:
```cpp
class BasicGame : public Pyramid::Game {
public:
    void onCreate() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
};
```

**Running the Example**:
```cmd
cd build\bin\Debug
BasicGame.exe
```

---

#### 2. BasicRendering
**Location**: `Examples/BasicRendering/`  
**Difficulty**: Beginner  
**Concepts**: Advanced rendering, shaders, materials, lighting

**Description**:
The BasicRendering example demonstrates modern rendering techniques:
- Advanced shader system with uniform buffers
- Physically-based material properties
- Dynamic lighting with multiple light types
- Orbital camera system
- Real-time material property animation
- Performance monitoring and statistics

**What You'll Learn**:
- Creating and compiling GLSL shaders
- Using uniform buffer objects (UBOs) for efficient data transfer
- Implementing basic lighting models (Phong, Blinn-Phong)
- Managing material properties and textures
- Camera mathematics and orbital movement
- Performance profiling and optimization

**Key Features Demonstrated**:
- Vertex and fragment shader compilation
- Uniform buffer object management
- Dynamic material property updates
- Multi-light rendering
- Camera orbital mechanics
- Frame rate monitoring

**Running the Example**:
```cmd
cd build\bin\Debug
BasicRendering.exe
```

---

### Intermediate Examples (Planned)

#### 3. SceneManagement
**Location**: `Examples/SceneManagement/` (Planned)  
**Difficulty**: Intermediate  
**Concepts**: Scene graphs, spatial partitioning, culling

**Description**:
Demonstrates the engine's scene management capabilities:
- Hierarchical scene graph organization
- Octree spatial partitioning
- Frustum culling optimization
- Dynamic object management
- LOD (Level of Detail) systems
- Performance monitoring for large scenes

**What You'll Learn**:
- Building complex scene hierarchies
- Spatial partitioning for performance
- Visibility culling techniques
- Managing thousands of objects efficiently
- Performance optimization strategies

---

#### 4. AdvancedMath
**Location**: `Examples/AdvancedMath/` (Planned)  
**Difficulty**: Intermediate  
**Concepts**: SIMD math operations, performance optimization

**Description**:
Showcases the engine's SIMD-optimized math library:
- Vector and matrix operations with SIMD
- Performance comparisons between scalar and SIMD code
- Batch processing of mathematical operations
- Runtime CPU feature detection
- Memory alignment considerations

**What You'll Learn**:
- Writing SIMD-optimized game code
- Understanding performance implications of math operations
- Batch processing for optimal CPU utilization
- Memory alignment for SIMD operations

---

#### 5. PhysicsDemo
**Location**: `Examples/PhysicsDemo/` (Planned)  
**Difficulty**: Intermediate  
**Concepts**: Physics simulation, collision detection, rigid bodies

**Description**:
Demonstrates physics system integration:
- Rigid body dynamics
- Collision detection and response
- Constraint systems (springs, hinges)
- Character controller implementation
- Vehicle physics simulation

**What You'll Learn**:
- Setting up physics worlds and objects
- Implementing realistic character movement
- Creating interactive physics simulations
- Integrating physics with rendering

---

### Advanced Examples (Planned)

#### 6. MultiThreadingExample
**Location**: `Examples/MultiThreading/` (Planned)  
**Difficulty**: Advanced  
**Concepts**: Multi-threaded rendering, job systems, performance

**Description**:
Shows advanced multi-threading techniques:
- Multi-threaded rendering pipelines
- Job system implementation
- Lock-free data structures
- CPU utilization optimization
- Thread synchronization strategies

---

#### 7. CustomShaders
**Location**: `Examples/CustomShaders/` (Planned)  
**Difficulty**: Advanced  
**Concepts**: Advanced shading, post-processing, effects

**Description**:
Advanced shader programming examples:
- Custom shader effects (water, fire, etc.)
- Post-processing pipelines
- Compute shader usage
- Advanced lighting models (PBR)
- Screen-space effects (SSAO, SSR)

## Tutorial Series

### Tutorial 1: Your First Triangle
**File**: `Tutorials/01_FirstTriangle.md`

A step-by-step guide to rendering your first triangle:
1. Setting up the development environment
2. Creating a minimal game class
3. Basic shader creation and compilation
4. Vertex buffer setup
5. Rendering pipeline basics

### Tutorial 2: Adding Colors and Movement
**File**: `Tutorials/02_ColorsAndMovement.md`

Building on the first tutorial:
1. Adding vertex colors
2. Implementing simple animations
3. Time-based movement
4. Camera basics

### Tutorial 3: 3D Transformations
**File**: `Tutorials/03_3DTransformations.md`

Introduction to 3D graphics:
1. Understanding transformation matrices
2. Model, view, and projection matrices
3. 3D object positioning and rotation
4. Camera movement and control

### Tutorial 4: Textures and Materials
**File**: `Tutorials/04_TexturesAndMaterials.md`

Working with textures:
1. Loading textures from files
2. UV mapping and texture coordinates
3. Material system basics
4. Combining textures with lighting

### Tutorial 5: Lighting and Shading
**File**: `Tutorials/05_LightingAndShading.md`

Implementing realistic lighting:
1. Ambient, diffuse, and specular lighting
2. Multiple light sources
3. Normal mapping
4. Shadow mapping basics

## Building and Running Examples

### Prerequisites

Ensure you have built the engine successfully:

```cmd
cd Pyramid-Engine
cmake -B build -S .
cmake --build build --config Debug
```

### Running Individual Examples

Each example can be run independently:

```cmd
# Navigate to build output directory
cd build\bin\Debug

# Run specific example
BasicGame.exe
BasicRendering.exe
```

### Building Specific Examples

You can build individual examples:

```cmd
# Build only BasicGame
cmake --build build --target BasicGame --config Debug

# Build only BasicRendering
cmake --build build --target BasicRendering --config Debug
```

### Example Build Configuration

Examples can be disabled during CMake configuration:

```cmd
# Disable examples to speed up builds
cmake -B build -S . -DPYRAMID_BUILD_EXAMPLES=OFF

# Enable only specific examples (future feature)
cmake -B build -S . -DPYRAMID_BUILD_BASIC_EXAMPLES=ON -DPYRAMID_BUILD_ADVANCED_EXAMPLES=OFF
```

## Example Structure

Each example follows a consistent structure:

```
ExampleName/
├── CMakeLists.txt          # Build configuration
├── README.md               # Example-specific documentation
├── include/
│   └── ExampleName.hpp     # Header files
├── source/
│   ├── ExampleName.cpp     # Implementation
│   └── Main.cpp            # Entry point
└── assets/                 # Example-specific assets
    ├── shaders/
    ├── textures/
    └── models/
```

## Learning Path

We recommend following this learning path:

1. **Start with BasicGame**: Understand core engine concepts
2. **Progress to BasicRendering**: Learn advanced rendering techniques
3. **Study the Math Library**: Understand the mathematical foundations
4. **Explore Scene Management**: Learn about spatial organization
5. **Experiment with Physics**: Add realistic simulations
6. **Advanced Topics**: Multi-threading, custom shaders, optimization

## Common Issues and Solutions

### Examples Won't Start

**Problem**: Examples crash on startup

**Solutions**:
1. Ensure graphics drivers are up to date
2. Verify OpenGL 3.3+ support
3. Run from the correct directory (build\bin\Debug)
4. Check console output for error messages

### Poor Performance

**Problem**: Examples run slowly or stutter

**Solutions**:
1. Build in Release configuration for performance testing
2. Update graphics drivers
3. Check VSync settings
4. Monitor GPU and CPU usage

### Compilation Errors

**Problem**: Examples fail to compile

**Solutions**:
1. Ensure engine library compiled successfully
2. Check CMake configuration
3. Verify all dependencies are available
4. Clean and rebuild if necessary

## Contributing Examples

We welcome contributions of new examples! To contribute:

1. **Follow the Standard Structure**: Use the established directory layout
2. **Document Thoroughly**: Include comprehensive README files
3. **Focus on Learning**: Ensure examples are educational
4. **Test Thoroughly**: Verify examples work across different systems
5. **Submit Pull Request**: Follow the contribution guidelines

### Example Contribution Checklist

- [ ] Example follows standard directory structure
- [ ] CMakeLists.txt properly configured
- [ ] README.md with clear documentation
- [ ] Code is well-commented and educational
- [ ] Example builds and runs successfully
- [ ] No external dependencies beyond the engine
- [ ] Appropriate difficulty level indicated

## Feedback and Support

If you have questions about the examples or suggestions for new ones:

1. **GitHub Issues**: Report bugs or request features
2. **Discussions**: Ask questions and share knowledge
3. **Discord** (Coming Soon): Real-time community support

## Additional Resources

- [Engine Architecture](../Architecture.md) - Understanding the engine design
- [API Reference](../API_Reference/) - Detailed API documentation
- [Build Guide](../BuildGuide.md) - Building and configuration
- [Contributing Guide](../Contributing.md) - How to contribute to the project

## Planned Examples Roadmap

### Short Term (Next Release)
- [ ] SceneManagement example
- [ ] AdvancedMath demonstration
- [ ] Basic physics integration

### Medium Term
- [ ] Texture and material showcase
- [ ] Audio system integration
- [ ] Input system demonstration

### Long Term
- [ ] Multi-threading examples
- [ ] Custom shader gallery
- [ ] Performance optimization showcase
- [ ] VR/AR examples (when supported)

Stay tuned for updates and new examples as the engine continues to evolve!
