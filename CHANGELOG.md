# Changelog

All notable changes to the Pyramid Game Engine project will be documented in this file.

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
