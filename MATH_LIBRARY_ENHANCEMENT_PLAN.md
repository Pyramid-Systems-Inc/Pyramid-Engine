# Pyramid Engine Math Library Enhancement Plan

## Current State Analysis

### Existing Components
- **Vector4.hpp**: Basic OGLVec4 class with minimal functionality
  - Only has x, y, z, w members
  - No mathematical operations
  - Uses legacy naming convention (OGL prefix)
  - Includes deprecated header path

### Critical Gaps Identified
1. **No Vector2/Vector3 classes** - Essential for 2D/3D operations
2. **No mathematical operations** - Addition, subtraction, multiplication, etc.
3. **No matrix classes** - Mat3, Mat4 for transformations
4. **No quaternion support** - Required for 3D rotations
5. **No utility functions** - Dot product, cross product, normalization
6. **Legacy architecture** - Not following Pyramid namespace conventions
7. **No integration** - Not connected to graphics or scene systems

## Proposed Architecture

### Core Mathematical Types

#### 1. Vector Classes
```cpp
namespace Pyramid::Math {
    class Vec2 {
        f32 x, y;
        // Basic operations: +, -, *, /, ==, !=
        // Utility: Length(), LengthSquared(), Normalize(), Dot()
    };
    
    class Vec3 {
        f32 x, y, z;
        // All Vec2 operations plus Cross()
        // Conversion constructors from Vec2
    };
    
    class Vec4 {
        f32 x, y, z, w;
        // All Vec3 operations
        // Conversion constructors from Vec2/Vec3
    };
}
```

#### 2. Matrix Classes
```cpp
namespace Pyramid::Math {
    class Mat3 {
        f32 m[9]; // 3x3 matrix in column-major order
        // Basic operations, determinant, inverse, transpose
    };
    
    class Mat4 {
        f32 m[16]; // 4x4 matrix in column-major order
        // Transformation matrices: translation, rotation, scale
        // Projection matrices: perspective, orthographic
        // View matrices: lookAt
    };
}
```

#### 3. Quaternion Class
```cpp
namespace Pyramid::Math {
    class Quat {
        f32 x, y, z, w;
        // Rotation operations, SLERP, conversion to/from matrices
    };
}
```

#### 4. Utility Functions and Constants
```cpp
namespace Pyramid::Math {
    // Constants
    constexpr f32 PI = 3.14159265359f;
    constexpr f32 TWO_PI = 2.0f * PI;
    constexpr f32 HALF_PI = PI / 2.0f;
    constexpr f32 DEG_TO_RAD = PI / 180.0f;
    constexpr f32 RAD_TO_DEG = 180.0f / PI;
    
    // Utility functions
    f32 Radians(f32 degrees);
    f32 Degrees(f32 radians);
    f32 Clamp(f32 value, f32 min, f32 max);
    f32 Lerp(f32 a, f32 b, f32 t);
    bool IsEqual(f32 a, f32 b, f32 epsilon = 1e-6f);
}
```

## Implementation Priority

### Phase 1: Foundation (Week 1)
1. **Math namespace setup** - Create proper directory structure
2. **Vec2 class** - Complete implementation with all operations
3. **Vec3 class** - Build on Vec2, add cross product
4. **Mathematical constants** - PI, conversion factors, etc.
5. **Basic utility functions** - Radians, Degrees, Clamp, Lerp

### Phase 2: Core 3D Math (Week 2)
1. **Vec4 class** - Complete implementation
2. **Mat4 class** - Basic matrix operations
3. **Transformation functions** - Translation, rotation, scale matrices
4. **Integration tests** - Verify all operations work correctly

### Phase 3: Advanced Features (Week 3)
1. **Quaternion class** - Complete rotation system
2. **Mat3 class** - For 2D transformations and normal matrices
3. **Projection matrices** - Perspective and orthographic
4. **View matrices** - LookAt and camera utilities

### Phase 4: Integration (Week 4)
1. **Graphics integration** - Update shaders to use new math types
2. **Scene system preparation** - Transform classes using new math
3. **Performance optimization** - SIMD considerations for future
4. **Documentation** - Complete API documentation

## Dependencies for Scene Management

The Scene Management system requires:
- **Vec3**: For positions, scales, directions
- **Quat**: For rotations (preferred over Euler angles)
- **Mat4**: For transformation matrices and MVP calculations
- **Utility functions**: For interpolation and comparisons

## File Structure

```
Engine/Math/
├── include/Pyramid/Math/
│   ├── MathCommon.hpp      # Constants and utility functions
│   ├── Vec2.hpp            # 2D vector class
│   ├── Vec3.hpp            # 3D vector class  
│   ├── Vec4.hpp            # 4D vector class
│   ├── Mat3.hpp            # 3x3 matrix class
│   ├── Mat4.hpp            # 4x4 matrix class
│   ├── Quat.hpp            # Quaternion class
│   └── Math.hpp            # Main include header
├── source/
│   ├── Vec2.cpp            # Vector2 implementation
│   ├── Vec3.cpp            # Vector3 implementation
│   ├── Vec4.cpp            # Vector4 implementation
│   ├── Mat3.cpp            # Matrix3 implementation
│   ├── Mat4.cpp            # Matrix4 implementation
│   └── Quat.cpp            # Quaternion implementation
└── CMakeLists.txt          # Updated build configuration
```

## Integration Points

### Graphics System
- Shader uniforms will use Mat4 for MVP matrices
- Vertex positions will use Vec3
- Texture coordinates will use Vec2
- Colors can use Vec3/Vec4

### Scene Management
- Transform components will use Vec3 + Quat + Vec3 (position, rotation, scale)
- Camera will use Mat4 for view and projection matrices
- Bounding volumes will use Vec3 for centers and extents

### Future Physics Integration
- Rigid bodies will use Vec3 for velocity, acceleration
- Collision detection will use Vec3 for normals, contact points
- Quaternions for angular velocity and orientation

## Performance Considerations

### Current Implementation
- Focus on correctness and clean API
- Use standard floating-point operations
- Inline small functions for performance

### Future Optimizations
- SIMD intrinsics for vector operations (SSE/AVX)
- Memory alignment for cache efficiency
- Batch operations for multiple transforms
- GPU compute shaders for large datasets

## Testing Strategy

### Unit Tests
- Mathematical correctness for all operations
- Edge cases (zero vectors, identity matrices)
- Precision and floating-point comparisons
- Conversion between different types

### Integration Tests
- Graphics pipeline with new math types
- Scene transformation hierarchies
- Camera projection and view calculations
- Performance benchmarks

## Migration from Legacy Code

### Current OGLVec4 Class
- Deprecated and replaced with new Vec4
- Provide conversion utilities if needed
- Update all references in existing code
- Remove legacy includes and dependencies

### Graphics System Updates
- Update shader uniform setting to use new types
- Modify vertex structures to use new Vec3/Vec2
- Update transformation calculations in rendering

This plan provides a solid foundation for 3D mathematics in the Pyramid Engine while maintaining clean architecture and preparing for future enhancements.
