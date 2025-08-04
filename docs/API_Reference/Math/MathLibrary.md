# Pyramid Math Library API Reference

## Overview

The Pyramid Math Library is a comprehensive, SIMD-optimized mathematical library designed for high-performance 3D graphics and game development. Built with modern C++17 practices, the library provides efficient vector, matrix, and quaternion operations with automatic CPU feature detection for optimal performance.

## Key Features

- **SIMD Optimization**: Automatic runtime detection of CPU capabilities (SSE, SSE2, SSE3, SSE4.1, AVX, FMA)
- **Comprehensive Operations**: Full set of 3D math operations with operator overloading
- **Performance Focused**: Fast approximations, batch processing, and cache-friendly design
- **Memory Aligned**: 16-byte aligned structures for optimal SIMD performance
- **Type Safe**: Strong typing with comprehensive error checking
- **Easy to Use**: Intuitive API with comprehensive documentation

## Quick Start

```cpp
#include <Pyramid/Math/Math.hpp>

using namespace Pyramid::Math;

// Vector operations
Vec3 position(1.0f, 2.0f, 3.0f);
Vec3 direction = Vec3::Forward;
Vec3 newPosition = position + direction * 5.0f;

// Matrix transformations
Mat4 translation = Mat4::CreateTranslation(position);
Mat4 rotation = Mat4::CreateRotationY(Radians(45.0f));
Mat4 scale = Mat4::CreateScale(2.0f);
Mat4 transform = translation * rotation * scale;

// Camera setup
Mat4 view = Mat4::CreateLookAt(Vec3(0, 0, 5), Vec3::Zero, Vec3::Up);
Mat4 projection = Mat4::CreatePerspective(Radians(60.0f), 16.0f/9.0f, 0.1f, 1000.0f);
Mat4 mvp = projection * view * transform;
```

## Core Classes

### Vec2 - 2D Vector Class

The `Vec2` class represents a 2D vector with x and y components.

#### Key Features
- Full operator overloading (+, -, *, /)
- Vector operations (length, normalize, dot product)
- Geometric utilities (angle, distance, projection)
- Static constants (Zero, One, UnitX, UnitY)

#### Common Operations
```cpp
Vec2 a(1.0f, 2.0f);
Vec2 b(3.0f, 4.0f);

// Arithmetic
Vec2 c = a + b;           // (4.0f, 6.0f)
Vec2 d = a * 2.0f;        // (2.0f, 4.0f)
f32 dot = a.Dot(b);       // 11.0f

// Vector operations
f32 length = a.Length();  // 2.236f
Vec2 normalized = a.Normalized();  // (0.447f, 0.894f)
f32 angle = a.AngleBetween(b);    // Angle between vectors

// Utility
Vec2 lerped = a.Lerp(b, 0.5f);     // Midpoint
Vec2 reflected = a.Reflect(Vec2::UnitX);  // Reflection
```

### Vec3 - 3D Vector Class

The `Vec3` class is the workhorse for 3D operations, representing 3D vectors with x, y, and z components.

#### Key Features
- Complete 3D vector arithmetic
- Cross product and dot product operations
- Geometric operations (projection, rejection, reflection)
- Spherical coordinate conversion
- Static constants (Zero, One, Right, Up, Forward, etc.)

#### Common Operations
```cpp
Vec3 position(1.0f, 2.0f, 3.0f);
Vec3 direction = Vec3::Forward;
Vec3 target(5.0f, 2.0f, 3.0f);

// Arithmetic and operations
Vec3 newPosition = position + direction * 5.0f;
Vec3 toTarget = target - position;
f32 distance = toTarget.Length();  // 4.0f
Vec3 toTargetNormalized = toTarget.Normalized();

// 3D specific operations
Vec3 cross = Vec3::Right.Cross(Vec3::Up);  // Vec3::Forward
Vec3 projected = position.Project(direction);  // Project onto direction
Vec3 rejected = position.Reject(direction);   // Perpendicular component

// Geometric utilities
Vec3 fromSpherical = Vec3::FromSpherical(5.0f, Math::Radians(45.0f), Math::Radians(30.0f));
Vec3 slerped = Vec3::Slerp(Vec3::Forward, Vec3::Right, 0.5f);
```

### Vec4 - 4D Vector Class

The `Vec4` class represents 4D vectors, commonly used for homogeneous coordinates and color operations.

#### Key Features
- 4D vector arithmetic operations
- Homogeneous coordinate support
- Color operations (RGBA)
- Perspective divide operations
- Static constants (Zero, One, UnitX, UnitY, UnitZ, UnitW)

#### Common Operations
```cpp
Vec4 position(1.0f, 2.0f, 3.0f, 1.0f);  // Homogeneous position
Vec4 color(1.0f, 0.5f, 0.0f, 1.0f);    // RGBA color

// Arithmetic
Vec4 scaled = color * 0.8f;  // Dim color
f32 dot = position.Dot(Vec4::UnitX);  // 1.0f

// Homogeneous operations
Vec3 cartesian = position.XYZ() / position.w;  // Perspective divide
Vec4 homogeneous = Vec4(cartesian, 1.0f);    // To homogeneous
```

### Mat3 - 3x3 Matrix Class

The `Mat3` class represents 3x3 matrices, commonly used for 2D transformations and normal matrices.

#### Key Features
- 3x3 matrix operations
- 2D transformation support
- Normal matrix calculation
- Determinant and inverse operations
- Static creation methods for common transformations

#### Common Operations
```cpp
// 2D transformations
Mat3 translation = Mat3::CreateTranslation(Vec2(10.0f, 5.0f));
Mat3 rotation = Mat3::CreateRotation(Math::Radians(45.0f));
Mat3 scale = Mat3::CreateScale(Vec2(2.0f, 2.0f));
Mat3 transform = translation * rotation * scale;

// Matrix operations
Mat3 inverse = transform.Inverse();
f32 determinant = transform.Determinant();
Mat3 transpose = transform.Transpose();

// Vector transformation
Vec2 point(1.0f, 1.0f);
Vec2 transformed = transform * point;  // Apply transformation
```

### Mat4 - 4x4 Matrix Class

The `Mat4` class is essential for 3D graphics, representing 4x4 matrices in column-major order (OpenGL style).

#### Key Features
- Complete 4x4 matrix arithmetic
- 3D transformation methods
- Projection matrix creation
- View matrix creation
- Efficient element access and manipulation

#### Common Operations
```cpp
// 3D transformations
Vec3 position(1.0f, 2.0f, 3.0f);
Vec3 rotation(Math::Radians(0.0f), Math::Radians(45.0f), Math::Radians(0.0f));
Vec3 scale(2.0f, 2.0f, 2.0f);

Mat4 translationMat = Mat4::CreateTranslation(position);
Mat4 rotationMat = Mat4::CreateRotationY(rotation.y);
Mat4 scaleMat = Mat4::CreateScale(scale);
Mat4 worldMatrix = translationMat * rotationMat * scaleMat;

// Camera matrices
Mat4 viewMatrix = Mat4::CreateLookAt(
    Vec3(0.0f, 5.0f, 10.0f),  // Eye position
    Vec3::Zero,                 // Look at position
    Vec3::Up                    // Up vector
);

Mat4 projectionMatrix = Mat4::CreatePerspective(
    Math::Radians(60.0f),       // Field of view
    16.0f / 9.0f,              // Aspect ratio
    0.1f,                      // Near plane
    1000.0f                    // Far plane
);

// Combined transformation
Mat4 wvp = projectionMatrix * viewMatrix * worldMatrix;

// Matrix operations
Mat4 inverse = worldMatrix.Inverse();
Mat4 transpose = worldMatrix.Transpose();
f32 determinant = worldMatrix.Determinant();
```

### Quat - Quaternion Class

The `Quat` class represents quaternions for efficient rotation representation and interpolation.

#### Key Features
- Quaternion arithmetic operations
- Conversion to/from Euler angles and rotation matrices
- Spherical linear interpolation (SLERP)
- Fast rotation operations
- Gimbal lock avoidance

#### Common Operations
```cpp
// Creating quaternions
Quat identity = Quat::Identity;
Quat fromAxisAngle = Quat::FromAxisAngle(Vec3::Up, Math::Radians(45.0f));
Quat fromEuler = Quat::FromEulerAngles(Math::Radians(30.0f), Math::Radians(45.0f), Math::Radians(0.0f));
Quat fromMatrix = Quat::FromMatrix(rotationMatrix);

// Quaternion operations
Quat combined = rotation1 * rotation2;  // Combine rotations
Quat inverted = rotation.Inverse();
Quat normalized = rotation.Normalized();
f32 dot = rotation1.Dot(rotation2);

// Interpolation
Quat slerped = Quat::Slerp(rotation1, rotation2, 0.5f);  // Smooth interpolation
Quat nlerped = Quat::Nlerp(rotation1, rotation2, 0.5f);  // Normalized linear interpolation

// Conversion
Vec3 eulerAngles = rotation.ToEulerAngles();
Mat4 rotationMatrix = rotation.ToMatrix4();
Vec3 axis; f32 angle;
rotation.ToAxisAngle(axis, angle);  // Extract axis and angle

// Vector rotation
Vec3 direction = Vec3::Forward;
Vec3 rotated = rotation * direction;  // Apply quaternion rotation
```

## SIMD Optimizations

The math library automatically detects and utilizes available CPU features for optimal performance:

### CPU Feature Detection
```cpp
// Automatic detection at runtime
bool hasSSE = MathSIMD::HasSSE();
bool hasSSE2 = MathSIMD::HasSSE2();
bool hasSSE3 = MathSIMD::HasSSE3();
bool hasSSE41 = MathSIMD::HasSSE41();
bool hasAVX = MathSIMD::HasAVX();
bool hasFMA = MathSIMD::HasFMA();
```

### Batch Operations
```cpp
// SIMD-optimized batch operations
std::vector<Vec3> positions = { /* ... */ };
std::vector<Vec3> directions = { /* ... */ };
std::vector<Vec3> results;
std::vector<f32> distances;

// Batch operations use SIMD when available
MathSIMD::AddVectors(positions, directions, results);      // Vector addition
MathSIMD::CalculateDistances(positions, origin, distances); // Distance calculations
MathSIMD::TransformVectors(worldMatrix, positions, results); // Matrix transformations
```

### Performance Considerations

- **Memory Alignment**: All math types are 16-byte aligned for optimal SIMD performance
- **Cache Efficiency**: Data structures designed for cache-friendly access patterns
- **Branch Reduction**: Minimized branching in critical paths
- **Inline Optimization**: Critical functions are force-inlined for performance
- **Constant Folding**: Compile-time constant evaluation where possible

## Utility Functions

### Transformation Utilities
```cpp
// Combined transformations
Mat4 trs = Math::Transform::CreateTRS(position, rotation, scale);
Mat4 transform2D = Math::Transform::Create2D(position2D, rotation2D, scale2D);

// Camera utilities
Mat4 perspective = Math::Camera::CreatePerspective(60.0f, 16.0f/9.0f, 0.1f, 1000.0f);
Mat4 orthographic = Math::Camera::CreateOrthographic(1920.0f, 1080.0f, 0.1f, 1000.0f);
```

### Mathematical Constants
```cpp
// Common constants
f32 pi = Math::Constants::Pi;
f32 twoPi = Math::Constants::TwoPi;
f32 halfPi = Math::Constants::HalfPi;
f32 epsilon = Math::Constants::Epsilon;

// Angle conversions
f32 radians = Math::DegreesToRadians(180.0f);  // π radians
f32 degrees = Math::RadiansToDegrees(Math::Pi);  // 180 degrees
```

### Geometric Utilities
```cpp
// Intersection testing
bool raySphere = Math::Intersection::RaySphere(rayOrigin, rayDirection, sphereCenter, sphereRadius, t);
bool rayAABB = Math::Intersection::RayAABB(rayOrigin, rayDirection, aabbMin, aabbMax, t);
bool sphereSphere = Math::Intersection::SphereSphere(center1, radius1, center2, radius2);
bool aabbAABB = Math::Intersection::AABB_AABB(min1, max1, min2, max2);

// Frustum culling
bool inFrustum = Math::Frustum::ContainsPoint(frustumPlanes, point);
bool sphereInFrustum = Math::Frustum::ContainsSphere(frustumPlanes, center, radius);
```

## Best Practices

### Performance Tips
1. **Use const references**: Pass vectors and matrices by const reference to avoid copies
2. **Pre-allocate**: For batch operations, pre-allocate result vectors
3. **Normalize wisely**: Only normalize when necessary, it's expensive
4. **Cache matrices**: Reuse transformation matrices when possible
5. **Use SIMD batch operations**: For processing many vectors, use batch operations

### Numerical Stability
1. **Use epsilon comparisons**: Avoid direct floating-point equality checks
2. **Normalize before operations**: Ensure vectors are normalized before dot/cross products
3. **Check for zero division**: Be careful with division operations
4. **Use safe inverse**: Consider using safe inverse functions when numerical stability is critical

### Memory Management
1. **Stack allocation**: Prefer stack allocation for temporary math objects
2. **Alignment aware**: Be aware of 16-byte alignment for SIMD operations
3. **Pool allocation**: For frequent allocation/deallocation, consider memory pools

## Integration with Graphics System

The math library integrates seamlessly with the graphics system:

```cpp
// Camera setup
Camera camera(Math::Radians(60.0f), 16.0f/9.0f, 0.1f, 1000.0f);
camera.SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
camera.LookAt(Math::Vec3::Zero);

// Get matrices for rendering
const Math::Mat4& viewMatrix = camera.GetViewMatrix();
const Math::Mat4& projectionMatrix = camera.GetProjectionMatrix();
const Math::Mat4& viewProjectionMatrix = camera.GetViewProjectionMatrix();

// Use in shader uniforms
shader->SetUniform("u_ViewProjection", viewProjectionMatrix);
shader->SetUniform("u_WorldMatrix", worldMatrix);
shader->SetUniform("u_CameraPosition", camera.GetPosition());
```

## Error Handling

The math library provides comprehensive error handling:

```cpp
// Safe operations with error checking
Mat4 inverse;
if (matrix.IsInvertible()) {
    inverse = matrix.Inverse();
} else {
    // Handle non-invertible matrix
    PYRAMID_LOG_WARN("Attempted to invert non-invertible matrix");
    inverse = Mat4::Identity;
}

// Safe normalization
Vec3 normalized;
if (vector.Length() > Math::Constants::Epsilon) {
    normalized = vector.Normalized();
} else {
    // Handle zero-length vector
    normalized = Vec3::Zero;
}
```

## Conclusion

The Pyramid Math Library provides a comprehensive, high-performance mathematical foundation for 3D graphics and game development. With automatic SIMD optimization, comprehensive error handling, and seamless integration with the graphics system, it offers both performance and ease of use for developers building modern games and graphics applications.