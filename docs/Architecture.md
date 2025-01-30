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
- Shader system
- Vertex buffer abstraction

### Window Management

The window management system provides a platform-independent interface for window creation and management:

#### Components
- **Window**: Abstract window interface
  - Window creation and destruction
  - Event processing
  - Context management
  - Window state handling

- **Win32OpenGLWindow**: Windows implementation
  - Native Win32 API integration
  - OpenGL context management
  - Window procedure handling
  - Message processing

#### Features
- Platform-specific window implementations
- Event handling system
- Input management
- Resolution and display mode handling
- Window state management (minimize, maximize, close)
- Message processing and event dispatch
- Graphics context integration

### Game Loop

The main game loop is managed by the `Game` class, which provides:
- Game state management
- Update and render cycle
- Event processing
- Graphics device management

## Directory Structure

```
Pyramid/
├── Engine/                # Core engine library
│   ├── Core/             # Core engine functionality
│   │   ├── include/      # Public headers
│   │   └── source/       # Implementation files
│   ├── Graphics/         # Graphics abstraction layer
│   │   ├── include/      # Public headers
│   │   └── source/       # Implementation files
│   ├── Platform/         # Platform-specific code
│   │   ├── include/      # Public headers
│   │   └── source/       # Implementation files
│   ├── Math/             # Math library
│   ├── Utils/            # Utility functions
│   ├── Renderer/         # Rendering system
│   ├── Input/            # Input handling
│   ├── Scene/            # Scene management
│   ├── Audio/            # Audio system
│   └── Physics/          # Physics system
├── Examples/             # Example projects
│   └── BasicGame/        # Basic game example
├── Tools/                # Development tools
│   └── AssetProcessor/   # Asset processing tools
├── Tests/                # Test projects
│   ├── Unit/            # Unit tests
│   └── Integration/     # Integration tests
├── vendor/              # Third-party dependencies
│   └── glad/            # OpenGL loader
└── docs/                # Documentation
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

The graphics pipeline is designed to be API-agnostic:

1. **Graphics Device Layer**
   - IGraphicsDevice interface
   - API-specific implementations (OpenGL, DirectX)
   - Resource management
   - State management

2. **Shader System**
   - Abstract shader interface
   - GLSL support (OpenGL)
   - HLSL support (planned for DirectX)
   - Shader compilation and linking
   - Uniform/constant buffer management

3. **Buffer Management**
   - Vertex buffer abstraction
   - Index buffer support (planned)
   - Constant/uniform buffer support (planned)
   - Dynamic buffer updates

4. **Rendering**
   - Geometry management
   - Material system (planned)
   - Texture management (planned)
   - Render state management (planned)

## Future Development

Planned features and improvements:
- Complete math library
- Physics system
- Audio system
- Scene graph
- Asset management
- Input system
- Networking
- Scripting support
