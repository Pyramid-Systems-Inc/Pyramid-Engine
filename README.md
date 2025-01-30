# Pyramid Game Engine

A modern, cross-platform game engine written in C++ with OpenGL support.

## Features

- Modern C++17 implementation
- OpenGL-based rendering system
- Native window management
- Modular architecture
- Cross-platform support (currently Windows)

## Requirements

- CMake 3.16.0 or higher
- C++17 compatible compiler
- OpenGL compatible graphics card
- Windows 10 or higher (for Windows builds)

## Building

### Windows

1. Clone the repository:
```bash
git clone https://github.com/yourusername/Pyramid.git
cd Pyramid
```

2. Generate build files:
```bash
cmake -B build -S .
```

3. Build the project:
```bash
cmake --build build --config Debug  # or Release
```

The compiled binaries will be in:
- `build/bin/Debug/` or `build/bin/Release/` for executables
- `build/lib/Debug/` or `build/lib/Release/` for libraries

## Project Structure

- `Game/` - Example game implementation
- `OpenGL3D/` - Core engine library
  - `include/` - Public headers
  - `source/` - Implementation files
  - `vendor/` - Third-party dependencies
- `CMake/` - CMake configuration files

## Dependencies

- OpenGL - Graphics API
- GLAD - OpenGL loader
- Windows API (on Windows) - Window management

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [CHANGELOG.md](CHANGELOG.md) file.

## Authors

- **Omar Aglan** - *Initial work*
