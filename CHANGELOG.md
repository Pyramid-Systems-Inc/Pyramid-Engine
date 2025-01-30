# Changelog

All notable changes to the Pyramid Game Engine project will be documented in this file.

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
