# Contributing to Pyramid Engine

Welcome to the Pyramid Engine project! We're excited that you're interested in contributing. This document outlines the guidelines and processes for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Contributing Process](#contributing-process)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Documentation Guidelines](#documentation-guidelines)
- [Submitting Changes](#submitting-changes)
- [Review Process](#review-process)
- [Community](#community)

## Code of Conduct

This project adheres to a code of conduct that promotes a welcoming and inclusive environment:

### Our Pledge

We pledge to make participation in our project a harassment-free experience for everyone, regardless of:
- Age, body size, disability, ethnicity, gender identity and expression
- Level of experience, nationality, personal appearance, race, religion
- Sexual identity and orientation

### Expected Behavior

- Use welcoming and inclusive language
- Respect different viewpoints and experiences
- Accept constructive criticism gracefully
- Focus on what's best for the community
- Show empathy towards other community members

### Unacceptable Behavior

- Harassment, trolling, or discriminatory comments
- Public or private harassment
- Publishing others' private information without permission
- Other conduct that could reasonably be considered inappropriate

## Getting Started

### Prerequisites

Before contributing, ensure you have:

1. **Development Environment**: Windows 10/11 with Visual Studio 2022
2. **Tools**: CMake 3.16+, Git, and graphics drivers supporting OpenGL 3.3+
3. **Knowledge**: Basic understanding of C++, graphics programming, or game development
4. **GitHub Account**: For submitting pull requests and tracking issues

### Types of Contributions

We welcome various types of contributions:

#### Code Contributions
- **Bug Fixes**: Resolve existing issues or bugs
- **Features**: Implement new engine features or capabilities
- **Performance Improvements**: Optimize existing code
- **Platform Support**: Add support for new platforms

#### Documentation Contributions
- **API Documentation**: Improve or expand API reference
- **Tutorials**: Create new learning materials
- **Examples**: Add new example projects
- **Guides**: Write setup or usage guides

#### Community Contributions
- **Issue Reporting**: Report bugs or suggest features
- **Testing**: Test new features and provide feedback
- **Support**: Help other users in discussions
- **Code Review**: Review pull requests from other contributors

## Development Setup

### 1. Fork and Clone the Repository

```bash
# Fork the repository on GitHub, then clone your fork
git clone https://github.com/your-username/Pyramid-Engine.git
cd Pyramid-Engine

# Add the original repository as upstream
git remote add upstream https://github.com/original-owner/Pyramid-Engine.git
```

### 2. Set Up Development Environment

Follow the [Build Guide](BuildGuide.md) to set up your development environment:

```cmd
# Configure and build the engine
cmake -B build -S . -DPYRAMID_BUILD_EXAMPLES=ON -DPYRAMID_BUILD_TESTS=ON
cmake --build build --config Debug
```

### 3. Verify Setup

Ensure everything is working:

```cmd
# Run basic tests
cd build\bin\Debug
BasicGame.exe

# Check that examples work
BasicRendering.exe
```

### 4. Create Development Branch

```bash
# Create a new branch for your work
git checkout -b feature/your-feature-name

# Or for bug fixes
git checkout -b fix/bug-description
```

## Contributing Process

### 1. Choose What to Work On

#### Existing Issues
- Browse [GitHub Issues](https://github.com/your-repo/Pyramid-Engine/issues)
- Look for issues labeled `good first issue` for beginners
- Comment on issues you'd like to work on to avoid duplication

#### New Features
- Create an issue first to discuss the feature
- Ensure the feature aligns with project goals
- Get approval from maintainers before starting significant work

#### Bug Reports
- Search existing issues to avoid duplicates
- Provide detailed reproduction steps
- Include system information and error messages

### 2. Development Workflow

#### Keep Your Fork Updated
```bash
# Fetch latest changes from upstream
git fetch upstream
git checkout main
git merge upstream/main

# Update your feature branch
git checkout feature/your-feature-name
git rebase main
```

#### Make Incremental Changes
- Make small, focused commits
- Write clear commit messages
- Test your changes frequently
- Follow coding standards

#### Example Development Cycle
```bash
# Make your changes
# ... edit files ...

# Stage and commit changes
git add .
git commit -m "Add SIMD optimization for vector operations

- Implement SSE2/AVX optimized Vec3 operations
- Add runtime CPU feature detection
- Benchmarks show 3x performance improvement
- Maintains full backward compatibility"

# Push to your fork
git push origin feature/your-feature-name
```

## Coding Standards

### C++ Standards

#### Language Version
- **C++17**: Use modern C++17 features appropriately
- **Compatibility**: Ensure code works with MSVC 2022 and recent GCC/Clang
- **Standards Compliance**: Follow ISO C++ standards

#### Code Style

##### Naming Conventions
```cpp
// Classes: PascalCase
class GraphicsDevice;
class Vec3;

// Functions/Methods: PascalCase
void Initialize();
void CreateBuffer();

// Variables: camelCase
float deltaTime;
u32 vertexCount;

// Member Variables: m_ prefix + camelCase
class Camera {
private:
    Math::Vec3 m_position;
    float m_fieldOfView;
};

// Constants: PascalCase
const float Pi = 3.14159f;
const u32 MaxVertices = 10000;

// Enums: PascalCase
enum class TextureFormat {
    RGB8,
    RGBA8,
    Depth24Stencil8
};
```

##### File Organization
```cpp
// Header files (.hpp)
#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
// ... other includes ...

namespace Pyramid {
    namespace Graphics {
        class MyClass {
            // Public interface first
        public:
            MyClass();
            ~MyClass();
            
            // Public methods
            void DoSomething();
            
        protected:
            // Protected members
            
        private:
            // Private implementation
        };
    }
}
```

##### Documentation
```cpp
/**
 * @brief Brief description of the class or function
 * 
 * Detailed description explaining the purpose, usage,
 * and any important considerations.
 * 
 * @param paramName Description of parameter
 * @return Description of return value
 * 
 * @example
 * MyClass obj;
 * obj.DoSomething();
 */
class MyClass {
public:
    /**
     * @brief Does something important
     * @param value The input value to process
     * @return True if successful, false otherwise
     */
    bool DoSomething(float value);
};
```

### Error Handling

#### Use RAII Principles
```cpp
// Good: RAII resource management
class GraphicsBuffer {
public:
    GraphicsBuffer(size_t size) {
        m_handle = CreateBuffer(size);
    }
    
    ~GraphicsBuffer() {
        if (m_handle) {
            DestroyBuffer(m_handle);
        }
    }
    
private:
    BufferHandle m_handle = nullptr;
};
```

#### Logging for Errors
```cpp
// Use the engine's logging system
if (!InitializeGraphics()) {
    PYRAMID_LOG_ERROR("Failed to initialize graphics system");
    return false;
}

PYRAMID_LOG_INFO("Graphics system initialized successfully");
```

#### Assertions for Development
```cpp
// Use engine assertions for development checks
PYRAMID_ASSERT(vertices != nullptr, "Vertex data cannot be null");
PYRAMID_CORE_ASSERT(m_initialized, "System must be initialized before use");
```

### Performance Guidelines

#### Memory Management
```cpp
// Prefer stack allocation when possible
Vec3 position(1.0f, 2.0f, 3.0f);

// Use smart pointers for dynamic allocation
auto texture = std::make_unique<Texture>();
auto sharedResource = std::make_shared<Shader>();

// Avoid raw pointers except for non-owning references
void ProcessVertices(const Vertex* vertices, size_t count);
```

#### SIMD Considerations
```cpp
// Use engine math types for SIMD optimization
using namespace Pyramid::Math;

Vec3 position;  // Automatically SIMD-optimized
Mat4 transform; // 16-byte aligned for SIMD

// Batch operations when possible
std::vector<Vec3> positions;
SIMD::Batch::NormalizeVectors(positions.data(), positions.size());
```

## Testing Guidelines

### Unit Tests (Planned)

```cpp
// Example unit test structure
#include <gtest/gtest.h>
#include <Pyramid/Math/Vec3.hpp>

using namespace Pyramid::Math;

TEST(Vec3Test, DefaultConstruction) {
    Vec3 v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST(Vec3Test, Addition) {
    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(4.0f, 5.0f, 6.0f);
    Vec3 c = a + b;
    
    EXPECT_FLOAT_EQ(c.x, 5.0f);
    EXPECT_FLOAT_EQ(c.y, 7.0f);
    EXPECT_FLOAT_EQ(c.z, 9.0f);
}
```

### Integration Tests

```cpp
// Test engine components working together
TEST(GraphicsIntegrationTest, BufferCreationAndUsage) {
    auto device = CreateGraphicsDevice();
    ASSERT_TRUE(device->Initialize());
    
    auto buffer = device->CreateVertexBuffer();
    ASSERT_NE(buffer, nullptr);
    
    std::vector<float> data = {1.0f, 2.0f, 3.0f};
    buffer->SetData(data.data(), data.size() * sizeof(float));
    
    EXPECT_EQ(buffer->GetSize(), data.size() * sizeof(float));
}
```

### Manual Testing

For features that require manual testing:

1. **Create Test Scenarios**: Document specific test cases
2. **Cross-Platform Testing**: Test on different hardware configurations
3. **Performance Testing**: Measure performance impact of changes
4. **Visual Testing**: For graphics features, provide before/after screenshots

## Documentation Guidelines

### API Documentation

Use Doxygen-style comments for all public APIs:

```cpp
/**
 * @class GraphicsDevice
 * @brief Interface for graphics device implementations
 * 
 * The GraphicsDevice provides a platform-independent interface
 * for graphics operations including rendering, buffer management,
 * and shader compilation.
 * 
 * @see OpenGLDevice for the primary implementation
 * @since Version 0.6.0
 */
class GraphicsDevice {
public:
    /**
     * @brief Initialize the graphics device
     * 
     * Initializes the graphics device and creates the rendering context.
     * This must be called before any other graphics operations.
     * 
     * @return true if initialization succeeded, false otherwise
     * 
     * @note This function must be called from the main thread
     * @warning Calling this function multiple times has undefined behavior
     * 
     * @example
     * @code
     * auto device = CreateGraphicsDevice();
     * if (!device->Initialize()) {
     *     // Handle initialization failure
     * }
     * @endcode
     */
    virtual bool Initialize() = 0;
};
```

### Markdown Documentation

For guides and tutorials:

```markdown
# Title Using H1

## Section Using H2

Brief introduction paragraph explaining what this section covers.

### Subsection Using H3

Detailed explanation with code examples:

```cpp
// Code example with comments
Vec3 position(1.0f, 2.0f, 3.0f);
Vec3 velocity = CalculateVelocity(position, deltaTime);
```

Key points:
- Use bullet points for important information
- **Bold** for emphasis on critical points
- `inline code` for class names, function names, and small code snippets

> **Note**: Use blockquotes for important notes or warnings
```

### Example Projects

When adding new examples:

1. **Clear Purpose**: Each example should demonstrate specific concepts
2. **Progressive Complexity**: Build on previous examples
3. **Comprehensive Comments**: Explain not just what, but why
4. **README Files**: Include detailed README with learning objectives

```cpp
/**
 * @file BasicRendering.cpp
 * @brief Demonstrates advanced rendering techniques
 * 
 * This example showcases:
 * - Shader compilation and usage
 * - Uniform buffer objects
 * - Basic lighting calculations
 * - Camera movement and controls
 * 
 * Learning objectives:
 * - Understanding the rendering pipeline
 * - Working with shaders and uniforms
 * - Implementing basic lighting models
 */
```

## Submitting Changes

### Pull Request Process

#### 1. Prepare Your Changes

```bash
# Ensure your branch is up to date
git fetch upstream
git rebase upstream/main

# Run tests and verify examples still work
cmake --build build --config Debug
cd build\bin\Debug
BasicGame.exe
BasicRendering.exe
```

#### 2. Create Pull Request

1. **Push to Your Fork**:
   ```bash
   git push origin feature/your-feature-name
   ```

2. **Create PR on GitHub**:
   - Navigate to your fork on GitHub
   - Click "Compare & pull request"
   - Choose the correct base branch (usually `main`)

#### 3. PR Description Template

```markdown
## Description
Brief description of what this PR does and why it's needed.

## Type of Change
- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Breaking change (fix or feature that causes existing functionality to change)
- [ ] Documentation update
- [ ] Performance improvement
- [ ] Code refactoring

## Testing
- [ ] I have tested this change locally
- [ ] I have added/updated unit tests for this change
- [ ] I have tested with the example projects
- [ ] I have verified no performance regression

## Related Issues
Closes #123
Related to #456

## Screenshots (if applicable)
For visual changes, include before/after screenshots.

## Checklist
- [ ] My code follows the project's coding standards
- [ ] I have performed a self-review of my code
- [ ] I have commented my code, particularly hard-to-understand areas
- [ ] I have made corresponding changes to the documentation
- [ ] My changes generate no new warnings
- [ ] I have added tests that prove my fix is effective or that my feature works
```

### Commit Message Guidelines

Use conventional commit format:

```
type(scope): brief description

Detailed explanation of what was changed and why.
Include any breaking changes or migration notes.

- Bullet points for multiple changes
- Reference issues: Fixes #123, Closes #456
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

Examples:
```
feat(math): add SIMD optimizations for Vec3 operations

Implement SSE2/AVX optimized vector operations with runtime
CPU feature detection. Provides 3x performance improvement
for vector-heavy operations while maintaining backward
compatibility.

- Add SIMD::Vec3Add, Vec3Multiply, Vec3Normalize functions
- Implement runtime CPU feature detection
- Add fallback paths for unsupported CPUs
- Update benchmarks and documentation

Closes #142
```

## Review Process

### What to Expect

1. **Automated Checks**: CI/CD will run automated tests
2. **Code Review**: Maintainers will review your code
3. **Feedback**: You may receive requests for changes
4. **Discussion**: Be open to discussion and suggestions
5. **Approval**: Once approved, your PR will be merged

### Code Review Guidelines

#### As a Contributor
- **Respond Promptly**: Address feedback in a timely manner
- **Be Open to Feedback**: Consider suggestions constructively
- **Ask Questions**: If you don't understand feedback, ask for clarification
- **Make Requested Changes**: Implement suggested improvements

#### As a Reviewer
- **Be Constructive**: Provide helpful, actionable feedback
- **Explain Why**: Don't just point out problems, explain better approaches
- **Consider Impact**: Think about performance, maintainability, and usability
- **Test Changes**: Actually run and test the code when possible

### Common Review Comments

#### Performance Issues
```cpp
// ❌ Avoid: Unnecessary allocation in hot path
std::string GetName() const {
    return std::string(m_name.begin(), m_name.end());
}

// ✅ Better: Return const reference
const std::string& GetName() const {
    return m_name;
}
```

#### Error Handling
```cpp
// ❌ Avoid: Silent failures
void LoadTexture(const std::string& path) {
    auto data = ReadFile(path);
    // No error checking
    CreateTexture(data);
}

// ✅ Better: Proper error handling
bool LoadTexture(const std::string& path) {
    auto data = ReadFile(path);
    if (!data) {
        PYRAMID_LOG_ERROR("Failed to read texture file: ", path);
        return false;
    }
    return CreateTexture(data);
}
```

#### Code Clarity
```cpp
// ❌ Avoid: Magic numbers and unclear intent
if (distance < 0.001f) {
    // What does 0.001f represent?
}

// ✅ Better: Named constants and clear intent
const float EPSILON = 0.001f;
if (distance < EPSILON) {
    // Handle near-zero distance
}
```

## Community

### Communication Channels

#### GitHub
- **Issues**: Bug reports, feature requests, discussions
- **Pull Requests**: Code contributions and reviews
- **Discussions**: General questions and community conversations

#### Discord (Coming Soon)
- **Real-time Chat**: Quick questions and community interaction
- **Voice Channels**: Pair programming and collaboration
- **Announcements**: Project updates and news

### Getting Help

#### For Contributors
1. **Read Documentation**: Start with existing docs and examples
2. **Search Issues**: Look for similar problems or questions
3. **Ask Questions**: Create an issue or discussion if you're stuck
4. **Join Community**: Participate in Discord when available

#### For Maintainers
1. **Review Process**: Follow established review guidelines
2. **Release Planning**: Coordinate with core team on releases
3. **Community Support**: Help answer questions and guide new contributors

### Recognition

We recognize contributors in several ways:

#### Contributors File
- All contributors are listed in CONTRIBUTORS.md
- Significant contributions are highlighted in release notes
- Long-term contributors may be invited to join the core team

#### Release Notes
```markdown
## Version 0.7.0 - "Performance Update"

### New Features
- SIMD optimizations for math library (@contributor-name)
- Advanced shader system (@another-contributor)

### Bug Fixes
- Fixed memory leak in texture loading (@bug-fixer)

### Documentation
- Complete API reference overhaul (@doc-writer)

### Special Thanks
Special thanks to @major-contributor for their ongoing work
on the physics system architecture.
```

## Conclusion

Thank you for considering contributing to Pyramid Engine! Your contributions help make the engine better for everyone. Whether you're fixing bugs, adding features, improving documentation, or helping other users, every contribution is valuable.

Remember:
- Start small with your first contributions
- Don't hesitate to ask questions
- Focus on code quality and user experience
- Have fun and learn from the community!

For more information, see:
- [Build Guide](BuildGuide.md) - Setting up your development environment
- [Architecture Overview](Architecture.md) - Understanding the engine design
- [API Reference](API_Reference/) - Detailed API documentation
- [Examples](Examples/) - Learning from example projects

Welcome to the Pyramid Engine community! 🎮
