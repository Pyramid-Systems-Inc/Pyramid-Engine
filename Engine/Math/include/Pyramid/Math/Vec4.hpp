#pragma once

#include <Pyramid/Math/MathCommon.hpp>
#include <Pyramid/Math/Vec2.hpp>
#include <Pyramid/Math/Vec3.hpp>
#include <string>

namespace Pyramid {
namespace Math {

/**
 * @brief 4D Vector class for mathematical operations
 * 
 * Represents a 4D vector with x, y, z, and w components.
 * Commonly used for homogeneous coordinates in 3D graphics.
 */
class Vec4
{
public:
    // Components
    f32 x, y, z, w;

    // Constructors
    /**
     * @brief Default constructor - initializes to zero vector
     */
    Vec4();

    /**
     * @brief Constructor with single value for all components
     * @param value Value for x, y, z, and w
     */
    explicit Vec4(f32 value);

    /**
     * @brief Constructor with separate x, y, z, and w values
     * @param x X component
     * @param y Y component
     * @param z Z component
     * @param w W component
     */
    Vec4(f32 x, f32 y, f32 z, f32 w);

    /**
     * @brief Constructor from Vec2 and z, w components
     * @param vec2 Vec2 for x and y components
     * @param z Z component
     * @param w W component
     */
    Vec4(const Vec2& vec2, f32 z, f32 w);

    /**
     * @brief Constructor from Vec3 and w component
     * @param vec3 Vec3 for x, y, and z components
     * @param w W component
     */
    Vec4(const Vec3& vec3, f32 w);

    /**
     * @brief Copy constructor
     * @param other Vector to copy
     */
    Vec4(const Vec4& other) = default;

    /**
     * @brief Assignment operator
     * @param other Vector to assign
     * @return Reference to this vector
     */
    Vec4& operator=(const Vec4& other) = default;

    // Conversion operators
    /**
     * @brief Convert to Vec2 (drops z and w components)
     * @return Vec2 with x and y components
     */
    Vec2 ToVec2() const;

    /**
     * @brief Convert to Vec3 (drops w component)
     * @return Vec3 with x, y, and z components
     */
    Vec3 ToVec3() const;

    // Arithmetic operators
    Vec4 operator+(const Vec4& other) const;
    Vec4 operator-(const Vec4& other) const;
    Vec4 operator*(f32 scalar) const;
    Vec4 operator*(const Vec4& other) const;
    Vec4 operator/(f32 scalar) const;
    Vec4 operator/(const Vec4& other) const;
    Vec4 operator-() const;

    // Compound assignment operators
    Vec4& operator+=(const Vec4& other);
    Vec4& operator-=(const Vec4& other);
    Vec4& operator*=(f32 scalar);
    Vec4& operator*=(const Vec4& other);
    Vec4& operator/=(f32 scalar);
    Vec4& operator/=(const Vec4& other);

    // Comparison operators
    bool operator==(const Vec4& other) const;
    bool operator!=(const Vec4& other) const;

    // Array access operators
    f32 operator[](i32 index) const;
    f32& operator[](i32 index);

    // Vector operations
    f32 Length() const;
    f32 LengthSquared() const;
    Vec4& Normalize();
    Vec4 Normalized() const;
    bool IsNormalized() const;
    bool IsZero() const;

    f32 Dot(const Vec4& other) const;
    f32 Distance(const Vec4& other) const;
    f32 DistanceSquared(const Vec4& other) const;
    f32 AngleBetween(const Vec4& other) const;
    Vec4 Lerp(const Vec4& other, f32 t) const;

    std::string ToString() const;

    // Static utility functions
    static const Vec4 Zero;
    static const Vec4 One;
    static const Vec4 UnitX;
    static const Vec4 UnitY;
    static const Vec4 UnitZ;
    static const Vec4 UnitW;
};

// Non-member operators
Vec4 operator*(f32 scalar, const Vec4& vector);

} // namespace Math
} // namespace Pyramid
