#pragma once

#include <Pyramid/Math/MathCommon.hpp>
#include <Pyramid/Math/Vec2.hpp>
#include <string>

namespace Pyramid {
namespace Math {

/**
 * @brief 3D Vector class for mathematical operations
 * 
 * Represents a 3D vector with x, y, and z components.
 * Provides comprehensive mathematical operations for 3D vector arithmetic.
 */
class Vec3
{
public:
    // Components
    f32 x, y, z;

    // Constructors
    /**
     * @brief Default constructor - initializes to zero vector
     */
    Vec3();

    /**
     * @brief Constructor with single value for all components
     * @param value Value for x, y, and z
     */
    explicit Vec3(f32 value);

    /**
     * @brief Constructor with separate x, y, and z values
     * @param x X component
     * @param y Y component
     * @param z Z component
     */
    Vec3(f32 x, f32 y, f32 z);

    /**
     * @brief Constructor from Vec2 and z component
     * @param vec2 Vec2 for x and y components
     * @param z Z component
     */
    Vec3(const Vec2& vec2, f32 z);

    /**
     * @brief Copy constructor
     * @param other Vector to copy
     */
    Vec3(const Vec3& other) = default;

    /**
     * @brief Assignment operator
     * @param other Vector to assign
     * @return Reference to this vector
     */
    Vec3& operator=(const Vec3& other) = default;

    // Conversion operators
    /**
     * @brief Convert to Vec2 (drops z component)
     * @return Vec2 with x and y components
     */
    Vec2 ToVec2() const;

    // Arithmetic operators
    /**
     * @brief Addition operator
     * @param other Vector to add
     * @return Result of addition
     */
    Vec3 operator+(const Vec3& other) const;

    /**
     * @brief Subtraction operator
     * @param other Vector to subtract
     * @return Result of subtraction
     */
    Vec3 operator-(const Vec3& other) const;

    /**
     * @brief Scalar multiplication operator
     * @param scalar Scalar to multiply by
     * @return Result of multiplication
     */
    Vec3 operator*(f32 scalar) const;

    /**
     * @brief Component-wise multiplication operator
     * @param other Vector to multiply with
     * @return Result of component-wise multiplication
     */
    Vec3 operator*(const Vec3& other) const;

    /**
     * @brief Scalar division operator
     * @param scalar Scalar to divide by
     * @return Result of division
     */
    Vec3 operator/(f32 scalar) const;

    /**
     * @brief Component-wise division operator
     * @param other Vector to divide by
     * @return Result of component-wise division
     */
    Vec3 operator/(const Vec3& other) const;

    /**
     * @brief Unary negation operator
     * @return Negated vector
     */
    Vec3 operator-() const;

    // Compound assignment operators
    Vec3& operator+=(const Vec3& other);
    Vec3& operator-=(const Vec3& other);
    Vec3& operator*=(f32 scalar);
    Vec3& operator*=(const Vec3& other);
    Vec3& operator/=(f32 scalar);
    Vec3& operator/=(const Vec3& other);

    // Comparison operators
    bool operator==(const Vec3& other) const;
    bool operator!=(const Vec3& other) const;

    // Array access operators
    f32 operator[](i32 index) const;
    f32& operator[](i32 index);

    // Vector operations
    f32 Length() const;
    f32 LengthSquared() const;
    Vec3& Normalize();
    Vec3 Normalized() const;
    bool IsNormalized() const;
    bool IsZero() const;

    /**
     * @brief Calculate the dot product with another vector
     * @param other Vector to calculate dot product with
     * @return Dot product result
     */
    f32 Dot(const Vec3& other) const;

    /**
     * @brief Calculate the cross product with another vector
     * @param other Vector to calculate cross product with
     * @return Cross product result
     */
    Vec3 Cross(const Vec3& other) const;

    f32 Distance(const Vec3& other) const;
    f32 DistanceSquared(const Vec3& other) const;

    /**
     * @brief Get the angle between this vector and another in radians
     * @param other Vector to calculate angle with
     * @return Angle between vectors in radians
     */
    f32 AngleBetween(const Vec3& other) const;

    Vec3 Lerp(const Vec3& other, f32 t) const;
    Vec3 Reflect(const Vec3& normal) const;

    /**
     * @brief Project this vector onto another vector
     * @param other Vector to project onto
     * @return Projected vector
     */
    Vec3 Project(const Vec3& other) const;

    /**
     * @brief Get the component of this vector perpendicular to another vector
     * @param other Vector to get perpendicular component relative to
     * @return Perpendicular component
     */
    Vec3 Reject(const Vec3& other) const;

    std::string ToString() const;

    // Static utility functions
    static const Vec3 Zero;
    static const Vec3 Right;
    static const Vec3 Up;
    static const Vec3 Forward;
    static const Vec3 Left;
    static const Vec3 Down;
    static const Vec3 Back;
    static const Vec3 One;

    /**
     * @brief Create a vector from spherical coordinates
     * @param radius Radius (distance from origin)
     * @param theta Azimuthal angle in radians (around Y axis)
     * @param phi Polar angle in radians (from Y axis)
     * @return Vector in Cartesian coordinates
     */
    static Vec3 FromSpherical(f32 radius, f32 theta, f32 phi);

    /**
     * @brief Spherical linear interpolation between two vectors
     * @param a Start vector
     * @param b End vector
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated vector
     */
    static Vec3 Slerp(const Vec3& a, const Vec3& b, f32 t);
};

// Non-member operators
Vec3 operator*(f32 scalar, const Vec3& vector);

} // namespace Math
} // namespace Pyramid
