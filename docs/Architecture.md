# Pyramid Game Engine Architecture

## Overview

Pyramid is a modern, OpenGL-based game engine designed with modularity and performance in mind. The engine is built using C++17 and provides a robust foundation for game development.

## Core Components

### 1. Window Management (`OpenGL3D/Window`)
- Handles window creation and management
- Provides platform-specific implementations (currently Windows)
- Manages OpenGL context creation and lifecycle

### 2. Graphics Engine (`OpenGL3D/Graphic`)
- OpenGL-based rendering system
- Handles graphics context and rendering pipeline
- Supports modern OpenGL features (4.0+)

### 3. Game Framework (`OpenGL3D/Game`)
- Core game loop implementation
- Event handling system
- Base game class for user implementations

## Build System

The project uses CMake (3.16+) as its build system with the following features:
- Multi-configuration support (Debug/Release)
- Modern CMake practices
- Proper dependency management
- IDE integration with source grouping
- Installation rules for headers and binaries

## Dependencies

### External
- OpenGL 4.0+
- GLAD for OpenGL loading
- Windows API (for Windows platform)

### Internal
- OpenGL3D - Core engine library
- Game - Example game implementation

## Directory Structure

```
Pyramid/
├── CMake/              # CMake configuration files
├── docs/               # Documentation
├── Game/               # Example game implementation
├── OpenGL3D/          # Core engine library
│   ├── include/       # Public headers
│   ├── source/        # Implementation files
│   └── vendor/        # Third-party dependencies
└── README.md          # Project documentation
```

## Future Development

Planned features and improvements:
1. Cross-platform support (Linux, macOS)
2. Vulkan renderer
3. Asset management system
4. Physics integration
5. Audio system
