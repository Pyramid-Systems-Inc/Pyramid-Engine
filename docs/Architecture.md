# Pyramid Game Engine Architecture

## Overview

Pyramid is a modern, multi-platform game engine designed with flexibility and performance in mind. The engine supports multiple graphics APIs and provides a clean abstraction layer for rendering.

## Core Components

### Graphics System

The graphics system is built around a flexible abstraction layer that supports multiple graphics APIs:

- **IGraphicsDevice**: Core interface for all graphics implementations
  - OpenGL (3.3 - 4.6)
  - DirectX 9 (planned)
  - DirectX 10 (planned)
  - DirectX 11 (planned)
  - DirectX 12 (planned)

#### Graphics Features
- Multiple graphics API support
- Version detection and fallback
- Automatic feature detection
- Unified resource management

### Window Management

- Platform-specific window implementations
- Event handling system
- Input management
- Resolution and display mode handling

### Game Loop

The main game loop is managed by the `OglGame` class, which provides:
- Game state management
- Update and render cycle
- Event processing
- Graphics device management

## Directory Structure

```
Pyramid/
├── OpenGL3D/               # Core engine library
│   ├── include/           # Public headers
│   │   └── OpenGL3D/
│   │       ├── Core/     # Core engine types and utilities
│   │       ├── Game/     # Game loop and management
│   │       ├── Graphics/ # Graphics abstraction and implementations
│   │       └── Window/   # Window management
│   └── source/           # Implementation files
├── Game/                  # Example game implementation
├── docs/                  # Documentation
└── build/                # Build output
```

## Dependencies

- GLAD: OpenGL loader
- Windows API: Window management (Windows platform)
- CMake: Build system
- Visual Studio 2022: Development environment

## Build System

The project uses CMake for build configuration:
- Minimum CMake version: 3.16.0
- Multi-configuration support
- Proper dependency management
- Installation rules

## Platform Support

Currently supported platforms:
- Windows 10/11 (primary)

Planned platform support:
- Linux
- macOS

## Graphics Pipeline

The graphics pipeline is abstracted through the following components:

1. **Graphics Device**
   - Device creation and management
   - Context handling
   - Resource management

2. **Resource Management**
   - Textures
   - Vertex buffers
   - Shaders
   - State objects

3. **Rendering**
   - Command submission
   - State management
   - Present/swap chain

## Future Development

Planned features and improvements:
- Multiple graphics API support
- Resource management system
- Material system
- Scene graph
- Physics integration
- Audio system

## Best Practices

When developing for the Pyramid Engine:

1. **Graphics API Independence**
   - Use the IGraphicsDevice interface
   - Don't make API-specific assumptions
   - Handle feature availability gracefully

2. **Resource Management**
   - Use engine resource handles
   - Properly manage resource lifetime
   - Handle device lost scenarios

3. **Error Handling**
   - Check initialization results
   - Handle graphics API errors
   - Provide meaningful error messages

4. **Performance**
   - Batch similar operations
   - Minimize state changes
   - Use appropriate resource types

## Version Support

The engine supports various graphics API versions:

### OpenGL
- Minimum: OpenGL 3.3
- Recommended: OpenGL 4.0
- Optional features: OpenGL 4.3+ (compute shaders)

### DirectX (Planned)
- DirectX 9.0c
- DirectX 10.0
- DirectX 11.0
- DirectX 12.0
