#pragma once

#include <Pyramid/Math/MathCommon.hpp>

namespace Pyramid {
namespace Math {

// Memory alignment macros for SIMD optimization
#ifdef _MSC_VER
    #define PYRAMID_ALIGN(x) __declspec(align(x))
    #define PYRAMID_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define PYRAMID_ALIGN(x) __attribute__((aligned(x)))
    #define PYRAMID_FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define PYRAMID_ALIGN(x)
    #define PYRAMID_FORCE_INLINE inline
#endif

// SIMD-ready vector structures (16-byte aligned for SSE)
struct PYRAMID_ALIGN(16) Vec4Aligned
{
    f32 x, y, z, w;
    
    Vec4Aligned() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vec4Aligned(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
    
    // Conversion to/from regular Vec4
    explicit Vec4Aligned(const Vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    Vec4 ToVec4() const { return Vec4(x, y, z, w); }
};

struct PYRAMID_ALIGN(16) Mat4Aligned
{
    f32 m[16];
    
    Mat4Aligned() { std::memset(m, 0, sizeof(m)); }
    explicit Mat4Aligned(const Mat4& mat) { std::memcpy(m, mat.Data(), sizeof(m)); }
    Mat4 ToMat4() const { return Mat4(m); }
};

// Fast mathematical operations
namespace Fast {

/**
 * @brief Fast inverse square root (Quake III algorithm)
 * @param x Input value
 * @return Approximate 1/sqrt(x)
 */
PYRAMID_FORCE_INLINE f32 InvSqrt(f32 x)
{
    union {
        f32 f;
        u32 i;
    } conv = { x };
    
    conv.i = 0x5f3759df - (conv.i >> 1);
    conv.f *= 1.5f - (x * 0.5f * conv.f * conv.f);
    return conv.f;
}

/**
 * @brief Fast square root using inverse square root
 * @param x Input value
 * @return Approximate sqrt(x)
 */
PYRAMID_FORCE_INLINE f32 Sqrt(f32 x)
{
    return x * InvSqrt(x);
}

/**
 * @brief Fast vector length using fast sqrt
 * @param x X component
 * @param y Y component
 * @param z Z component
 * @return Approximate length
 */
PYRAMID_FORCE_INLINE f32 Length3(f32 x, f32 y, f32 z)
{
    return Fast::Sqrt(x * x + y * y + z * z);
}

/**
 * @brief Fast vector normalization
 * @param x X component (will be modified)
 * @param y Y component (will be modified)
 * @param z Z component (will be modified)
 */
PYRAMID_FORCE_INLINE void Normalize3(f32& x, f32& y, f32& z)
{
    f32 invLength = InvSqrt(x * x + y * y + z * z);
    x *= invLength;
    y *= invLength;
    z *= invLength;
}

/**
 * @brief Fast sine approximation using Taylor series
 * @param x Angle in radians
 * @return Approximate sine value
 */
PYRAMID_FORCE_INLINE f32 Sin(f32 x)
{
    // Normalize to [-PI, PI]
    while (x > PI) x -= TWO_PI;
    while (x < -PI) x += TWO_PI;
    
    // Taylor series approximation: sin(x) ≈ x - x³/6 + x⁵/120
    f32 x2 = x * x;
    f32 x3 = x2 * x;
    f32 x5 = x3 * x2;
    
    return x - (x3 / 6.0f) + (x5 / 120.0f);
}

/**
 * @brief Fast cosine approximation
 * @param x Angle in radians
 * @return Approximate cosine value
 */
PYRAMID_FORCE_INLINE f32 Cos(f32 x)
{
    return Sin(x + HALF_PI);
}

/**
 * @brief Fast atan2 approximation
 * @param y Y component
 * @param x X component
 * @return Approximate atan2 value
 */
PYRAMID_FORCE_INLINE f32 Atan2(f32 y, f32 x)
{
    if (Math::IsZero(x))
    {
        return (y > 0.0f) ? HALF_PI : -HALF_PI;
    }
    
    f32 atan = y / x;
    f32 atan2 = atan * atan;
    f32 atan3 = atan2 * atan;
    
    // Approximation: atan(x) ≈ x - x³/3 + x⁵/5
    f32 result = atan - (atan3 / 3.0f) + (atan3 * atan2 / 5.0f);
    
    if (x < 0.0f)
    {
        result += (y >= 0.0f) ? PI : -PI;
    }
    
    return result;
}

} // namespace Fast

// Batch operations for multiple vectors/matrices
namespace Batch {

/**
 * @brief Transform multiple Vec3 points by a matrix
 * @param points Array of points to transform
 * @param count Number of points
 * @param matrix Transformation matrix
 * @param results Array to store results (can be same as points)
 */
inline void TransformPoints(const Vec3* points, i32 count, const Mat4& matrix, Vec3* results)
{
    for (i32 i = 0; i < count; ++i)
    {
        Vec4 point(points[i], 1.0f);
        Vec4 transformed = matrix * point;
        results[i] = transformed.ToVec3();
    }
}

/**
 * @brief Transform multiple Vec3 directions by a matrix (w=0)
 * @param directions Array of directions to transform
 * @param count Number of directions
 * @param matrix Transformation matrix
 * @param results Array to store results (can be same as directions)
 */
inline void TransformDirections(const Vec3* directions, i32 count, const Mat4& matrix, Vec3* results)
{
    for (i32 i = 0; i < count; ++i)
    {
        Vec4 direction(directions[i], 0.0f);
        Vec4 transformed = matrix * direction;
        results[i] = transformed.ToVec3();
    }
}

/**
 * @brief Normalize multiple Vec3 vectors
 * @param vectors Array of vectors to normalize
 * @param count Number of vectors
 * @param results Array to store results (can be same as vectors)
 */
inline void NormalizeVectors(const Vec3* vectors, i32 count, Vec3* results)
{
    for (i32 i = 0; i < count; ++i)
    {
        results[i] = vectors[i].Normalized();
    }
}

/**
 * @brief Calculate dot products between corresponding vectors
 * @param vectorsA First array of vectors
 * @param vectorsB Second array of vectors
 * @param count Number of vectors
 * @param results Array to store dot product results
 */
inline void DotProducts(const Vec3* vectorsA, const Vec3* vectorsB, i32 count, f32* results)
{
    for (i32 i = 0; i < count; ++i)
    {
        results[i] = vectorsA[i].Dot(vectorsB[i]);
    }
}

/**
 * @brief Linear interpolation between corresponding vectors
 * @param vectorsA Start vectors
 * @param vectorsB End vectors
 * @param t Interpolation factor
 * @param count Number of vectors
 * @param results Array to store interpolated results
 */
inline void LerpVectors(const Vec3* vectorsA, const Vec3* vectorsB, f32 t, i32 count, Vec3* results)
{
    for (i32 i = 0; i < count; ++i)
    {
        results[i] = vectorsA[i].Lerp(vectorsB[i], t);
    }
}

} // namespace Batch

// Cache-friendly data structures
/**
 * @brief Structure of Arrays (SoA) for Vec3 data
 * Better cache performance for batch operations
 */
struct Vec3SoA
{
    f32* x;
    f32* y;
    f32* z;
    i32 count;
    i32 capacity;
    
    Vec3SoA(i32 capacity) : count(0), capacity(capacity)
    {
        x = new f32[capacity];
        y = new f32[capacity];
        z = new f32[capacity];
    }
    
    ~Vec3SoA()
    {
        delete[] x;
        delete[] y;
        delete[] z;
    }
    
    void Add(const Vec3& vec)
    {
        if (count < capacity)
        {
            x[count] = vec.x;
            y[count] = vec.y;
            z[count] = vec.z;
            count++;
        }
    }
    
    Vec3 Get(i32 index) const
    {
        return Vec3(x[index], y[index], z[index]);
    }
    
    void Set(i32 index, const Vec3& vec)
    {
        x[index] = vec.x;
        y[index] = vec.y;
        z[index] = vec.z;
    }
    
    // Batch normalize all vectors
    void NormalizeAll()
    {
        for (i32 i = 0; i < count; ++i)
        {
            f32 length = Math::SafeSqrt(x[i] * x[i] + y[i] * y[i] + z[i] * z[i]);
            if (!Math::IsZero(length))
            {
                f32 invLength = 1.0f / length;
                x[i] *= invLength;
                y[i] *= invLength;
                z[i] *= invLength;
            }
        }
    }
};

// Performance hints and utilities
namespace Perf {

/**
 * @brief Hint to the compiler that a branch is likely to be taken
 */
#ifdef __GNUC__
    #define LIKELY(x) __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LIKELY(x) (x)
    #define UNLIKELY(x) (x)
#endif

/**
 * @brief Prefetch memory for better cache performance
 * @param ptr Pointer to memory to prefetch
 */
inline void Prefetch(const void* ptr)
{
#ifdef __GNUC__
    __builtin_prefetch(ptr, 0, 3);
#elif defined(_MSC_VER)
    _mm_prefetch(static_cast<const char*>(ptr), _MM_HINT_T0);
#endif
}

} // namespace Perf

} // namespace Math
} // namespace Pyramid
