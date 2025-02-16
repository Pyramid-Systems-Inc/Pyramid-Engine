# Changelog

All notable changes to the Pyramid Game Engine will be documented in this file.

## [0.3.7] - 2025-02-16

### Added

- Vertex Array Objects (VAO) support
- Index buffer support
- Updated BasicGame example to use VAOs
- Vertex attribute management
- DrawIndexed and SetViewport methods

### Changed

- Refactored rendering pipeline to use VAOs
- Enhanced OpenGL buffer management
- Updated example to demonstrate vertex arrays and indexed drawing

## [0.3.6] - 2025-01-31

### Fixed

- Fixed window creation and OpenGL initialization issues
- Added proper window message processing in the game loop
- Fixed window title encoding for Unicode support
- Added GLAD initialization in OpenGLDevice
- Enabled depth testing by default
- Fixed window closing behavior

### Changed

- Moved graphics device initialization to Game constructor
- Improved window message handling in Win32OpenGLWindow
- Updated BasicGame example to use base class functionality

## [0.3.5] - 2025-01-31

### Added

- Initial project setup
- Core engine architecture
- Graphics abstraction layer
- Window management system
- Basic game loop
- OpenGL 3.3+ support
- GLAD integration
- CMake build system
- Basic game example
- Documentation

### Changed

- Reorganized project structure into modular components:
- Core: Core engine functionality
- Graphics: Graphics abstraction layer
- Platform: Platform-specific code
- Math: Math library
- Utils: Utility functions
- Renderer: Rendering system
- Input: Input handling
- Scene: Scene management
- Audio: Audio system
- Physics: Physics system
- Renamed OglGame to Game for better abstraction
- Renamed OglWindow to Window for better abstraction
- Updated documentation to reflect new structure

### Removed

- Direct OpenGL dependencies from public headers
- Platform-specific code from core components

## [0.3.4] - 2025-01-31

### Changed

- Major reorganization of engine structure:
  - Moved all core functionality to `Engine/` directory
  - Separated graphics, platform, and core components
  - Created proper module hierarchy
  - Improved code organization and maintainability
- Renamed files for better clarity and consistency:
  - OglGame -> Game
  - OglWindow -> Window
  - Added proper namespacing under Pyramid::
- Enhanced graphics abstraction:
  - Improved shader system
  - Better vertex buffer management
  - Cleaner graphics device interface

### Added

- New directory structure:
  - Engine/Core: Core engine functionality
  - Engine/Graphics: Graphics abstraction layer
  - Engine/Platform: Platform-specific code
  - Examples/BasicGame: Example game implementation
  - vendor/glad: OpenGL loader
- Comprehensive documentation for new structure
- Better separation of concerns between modules

### Removed

- Old OpenGL3D directory structure
- Redundant graphics engine implementation
- Deprecated file naming conventions

## [0.3.3] - 2025-01-31

### Added

- Proper window message processing
- Window state management (minimize, maximize, close)
- OpenGL context management improvements

### Fixed

- Window closing immediately after opening
- OpenGL context initialization issues
- Window procedure message handling
- Memory leaks in OpenGL context management

## [0.3.2] - 2025-01-31

### Added

- Proper vsync support
- Unicode support for window creation

### Changed

- Fixed window management system
- Better error handling in graphics initialization

### Fixed

- Unicode string handling in window creation
- Proper cleanup of OpenGL resources
- Vsync implementation
- Window class registration

## [0.3.1] - 2025-01-30

### Added

- Basic geometry rendering support
- Vertex buffer abstraction
- Shader system abstraction
- Test rectangle rendering with colored vertices
- Improved file organization for graphics components

### Changed

- Reorganized graphics-related files into more logical structure
- Enhanced OpenGL device implementation with proper resource management
- Improved version detection for OpenGL

## [0.3.0] - 2025-01-30

### Added

- Graphics API abstraction layer (IGraphicsDevice interface)
- OpenGL implementation of graphics device
- Support for multiple OpenGL versions (3.3 to 4.6)
- Core engine types and platform detection in Prerequisites.h
- Graphics API selection in game initialization

### Changed

- Refactored OglGame to use new graphics abstraction
- Improved error handling and initialization
- Better organization of graphics-related code
- Updated window management to work with graphics abstraction

### Prepared

- Framework for future DirectX 9/10/11/12 support
- Version detection and fallback system
- Graphics API feature management

## [0.2.3] - 2025-01-30

### Added

- Comprehensive Doxygen-style documentation for all major classes
- New `docs/` directory with detailed documentation:
  - Architecture.md: Engine architecture documentation
  - GettingStarted.md: User guide and tutorials
- Better code comments and explanations

### Changed

- Window title changed to "Pyramid Game Engine"
- Window class name updated to "PyramidGameEngine"
- Improved code formatting and style
- Enhanced error handling documentation

### Fixed

- Float truncation warnings in OglGame
- Unused variable warning in Win32OpenGLWindow
- Code style consistency

## [0.2.2] - 2025-01-30

### Added

- Source file grouping for better IDE organization
- Proper folder organization in Visual Studio
- More comprehensive .gitignore rules

### Changed

- Improved CMake configuration
- Better organization of source and header files in CMake
- Enhanced build type configuration
- Optimized installation rules using GNUInstallDirs
- Cleaned up project structure

### Removed

- Unused CMake utility files
- Redundant CMake configurations

## [0.2.1] - 2025-01-30

### Added

- Comprehensive README.md with build instructions and project information
- Detailed project structure documentation
- Build verification and testing

### Fixed

- Unicode string handling in Windows API calls
- Window creation and management in Win32 implementation
- Build system configuration for proper library linking

### Changed

- Improved code documentation and comments
- Enhanced error handling in window creation

## [0.2.0] - 2025-01-30

### Added

- Proper OpenGL and Windows system library linking
- Dependencies management through CMake/Dependencies.cmake
- Installation rules for headers and binaries
- Multi-processor compilation support for MSVC
- Better source organization in IDEs using source_group
- Support for both Debug and Release configurations

### Changed

- Updated to modern CMake practices with better target management
- Improved Windows API integration with proper Unicode support
- Better organized output directories for binaries and libraries
- Enhanced compiler options and warning levels
- Separated OpenGL3D and Game components into distinct targets

### Fixed

- Unicode/ANSI string conversion issues in Windows API calls
- OpenGL context creation and management
- CMake configuration issues with library linking
- Window creation in Win32 implementation

## [0.1.0] - 2025-01-30

### Added

- Initial project structure
- Basic CMake build system
- Comprehensive .gitignore file
- OpenGL3D library foundation
- Basic window management system
- Game executable target

### Changed

- Modernized C++ standard to C++17
- Improved project organization
- Better handling of platform-specific settings

### Fixed

- Initial build system configuration
- Project structure organization
- Basic dependency management
