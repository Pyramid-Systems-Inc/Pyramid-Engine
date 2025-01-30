# Pyramid Game Engine

A modern, multi-platform game engine with support for multiple graphics APIs. Currently focusing on OpenGL with planned support for DirectX 9/10/11/12.

## Features

- **Multiple Graphics API Support**
  - OpenGL 3.3 - 4.6 (current)
  - DirectX 9/10/11/12 (planned)
  - Clean abstraction layer
  - Automatic version detection

- **Modern C++ Design**
  - C++17 features
  - RAII resource management
  - Smart pointer usage
  - Exception safety

- **Window Management**
  - Native window handling
  - Event processing
  - Multiple monitor support
  - Resolution management

- **Graphics Features**
  - Multiple API support
  - Version detection
  - Feature management
  - Resource abstraction

## Requirements

- Windows 10 or later
- Visual Studio 2022
- CMake 3.16.0 or later
- Graphics card supporting:
  - OpenGL 3.3+ (current)
  - DirectX 9+ (for future features)

## Quick Start

1. Clone the repository:
```bash
git clone https://github.com/yourusername/Pyramid.git
cd Pyramid
```

2. Generate project files:
```bash
cmake -B build -S .
```

3. Build the engine:
```bash
cmake --build build --config Debug
```

4. Run the example game:
```bash
./build/bin/Debug/PyramidGame.exe
```

## Documentation

- [Getting Started Guide](docs/GettingStarted.md)
- [Architecture Overview](docs/Architecture.md)
- [Changelog](CHANGELOG.md)

## Project Structure

```
Pyramid/
├── OpenGL3D/               # Core engine library
│   ├── include/           # Public headers
│   │   └── OpenGL3D/
│   │       ├── Core/     # Core engine types
│   │       ├── Game/     # Game loop and management
│   │       ├── Graphics/ # Graphics abstraction
│   │       └── Window/   # Window management
│   └── source/           # Implementation files
├── Game/                  # Example game
├── docs/                  # Documentation
└── build/                # Build output
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Submit a pull request
4. Follow our coding standards

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- GLAD for OpenGL loading
- Windows API for window management
- CMake for build system

## Roadmap

### Current
- [x] OpenGL 3.3+ support
- [x] Basic window management
- [x] Graphics abstraction layer

### Short Term
- [ ] DirectX 9 support
- [ ] Resource management system
- [ ] Shader system improvements

### Long Term
- [ ] DirectX 10/11/12 support
- [ ] Physics integration
- [ ] Audio system
- [ ] Asset management

## Contact

- Submit issues through GitHub
- Join our Discord community (coming soon)
- Follow development updates on Twitter (coming soon)
