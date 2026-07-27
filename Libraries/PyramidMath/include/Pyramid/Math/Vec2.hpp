#pragma once

#include <Pyramid/Math/MathCommon.hpp>
#include <string>

namespace Pyramid {
namespace Math {

/**
 * @brief 2D Vector class for mathematical operations
 * 
 * Represents a 2D vector with x and y components.
 * Provides comprehensive mathematical operations for 2D vector arithmetic.
 */
class Vec2
{
public:
    // Components
    f32 x, y;

    // Constructors
    /**
     * @brief Default constructor - initializes to zero vector
     */
    Vec2();

    /**
     * @brief Constructor with single value for both components
     * @param value Value for both x and y
     */
    explicit Vec2(f32 value);

    /**
     * @brief Constructor with separate x and y values
     * @param x X component
     * @param y Y component
     */
    Vec2(f32 x, f32 y);

    /**
     * @brief Copy constructor
     * @param other Vector to copy
     */
    Vec2(const Vec2& other) = default;

    /**
     * @brief Assignment operator
     * @param other Vector to assign
     * @return Reference to this vector
     */
    Vec2& operator=(const Vec2& other) = default;

    // Arithmetic operators
    /**
     * @brief Addition operator
     * @param other Vector to add
     * @return Result of addition
     */
    Vec2 operator+(const Vec2& other) const;

    /**
     * @brief Subtraction operator
     * @param other Vector to subtract
     * @return Result of subtraction
     */
    Vec2 operator-(const Vec2& other) const;

    /**
     * @brief Scalar multiplication operator
     * @param scalar Scalar to multiply by
     * @return Result of multiplication
     */
    Vec2 operator*(f32 scalar) const;

    /**
     * @brief Component-wise multiplication operator
     * @param other Vector to multiply with
     * @return Result of component-wise multiplication
     */
    Vec2 operator*(const Vec2& other) const;

    /**
     * @brief Scalar division operator
     * @param scalar Scalar to divide by
     * @return Result of division
     */
    Vec2 operator/(f32 scalar) const;

    /**
     * @brief Component-wise division operator
     * @param other Vector to divide by
     * @return Result of component-wise division
     */
    Vec2 operator/(const Vec2& other) const;

    /**
     * @brief Unary negation operator
     * @return Negated vector
     */
    Vec2 operator-() const;

    // Compound assignment operators
    /**
     * @brief Addition assignment operator
     * @param other Vector to add
     * @return Reference to this vector
     */
    Vec2& operator+=(const Vec2& other);

    /**
     * @brief Subtraction assignment operator
     * @param other Vector to subtract
     * @return Reference to this vector
     */
    Vec2& operator-=(const Vec2& other);

    /**
     * @brief Scalar multiplication assignment operator
     * @param scalar Scalar to multiply by
     * @return Reference to this vector
     */
    Vec2& operator*=(f32 scalar);

    /**
     * @brief Component-wise multiplication assignment operator
     * @param other Vector to multiply with
     * @return Reference to this vector
     */
    Vec2& operator*=(const Vec2& other);

    /**
     * @brief Scalar division assignment operator
     * @param scalar Scalar to divide by
     * @return Reference to this vector
     */
    Vec2& operator/=(f32 scalar);

    /**
     * @brief Component-wise division assignment operator
     * @param other Vector to divide by
     * @return Reference to this vector
     */
    Vec2& operator/=(const Vec2& other);

    // Comparison operators
    /**
     * @brief Equality operator
     * @param other Vector to compare with
     * @return True if vectors are equal
     */
    bool operator==(const Vec2& other) const;

    /**
     * @brief Inequality operator
     * @param other Vector to compare with
     * @return True if vectors are not equal
     */
    bool operator!=(const Vec2& other) const;

    // Array access operators
    /**
     * @brief Array access operator (const)
     * @param index Index (0 for x, 1 for y)
     * @return Component value
     */
    f32 operator[](i32 index) const;

    /**
     * @brief Array access operator (non-const)
     * @param index Index (0 for x, 1 for y)
     * @return Reference to component
     */
    f32& operator[](i32 index);

    // Vector operations
    /**
     * @brief Calculate the length (magnitude) of the vector
     * @return Length of the vector
     */
    f32 Length() const;

    /**
     * @brief Calculate the squared length of the vector (faster than Length())
     * @return Squared length of the vector
     */
    f32 LengthSquared() const;

    /**
     * @brief Normalize the vector (make it unit length)
     * @return Reference to this vector
     */
    Vec2& Normalize();

    /**
     * @brief Get a normalized copy of the vector
     * @return Normalized vector
     */
    Vec2 Normalized() const;

    /**
     * @brief Check if the vector is normalized (unit length)
     * @return True if the vector is normalized
     */
    bool IsNormalized() const;

    /**
     * @brief Check if the vector is zero
     * @return True if the vector is zero
     */
    bool IsZero() const;

    /**
     * @brief Calculate the dot product with another vector
     * @param other Vector to calculate dot product with
     * @return Dot product result
     */
    f32 Dot(const Vec2& other) const;

    /**
     * @brief Calculate the distance to another vector
     * @param other Vector to calculate distance to
     * @return Distance between vectors
     */
    f32 Distance(const Vec2& other) const;

    /**
     * @brief Calculate the squared distance to another vector (faster than Distance())
     * @param other Vector to calculate squared distance to
     * @return Squared distance between vectors
     */
    f32 DistanceSquared(const Vec2& other) const;

    /**
     * @brief Get the angle of this vector in radians
     * @return Angle in radians
     */
    f32 Angle() const;

    /**
     * @brief Get the angle between this vector and another in radians
     * @param other Vector to calculate angle with
     * @return Angle between vectors in radians
     */
    f32 AngleBetween(const Vec2& other) const;

    /**
     * @brief Linear interpolation between this vector and another
     * @param other Target vector
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated vector
     */
    Vec2 Lerp(const Vec2& other, f32 t) const;

    /**
     * @brief Reflect this vector across a normal
     * @param normal Normal vector to reflect across
     * @return Reflected vector
     */
    Vec2 Reflect(const Vec2& normal) const;

    /**
     * @brief Get a perpendicular vector (rotated 90 degrees counter-clockwise)
     * @return Perpendicular vector
     */
    Vec2 Perpendicular() const;

    /**
     * @brief Convert vector to string representation
     * @return String representation
     */
    std::string ToString() const;

    // Static utility functions
    /**
     * @brief Zero vector (0, 0)
     */
    static const Vec2 Zero;

    /**
     * @brief Unit vector pointing right (1, 0)
     */
    static const Vec2 Right;

    /**
     * @brief Unit vector pointing up (0, 1)
     */
    static const Vec2 Up;

    /**
     * @brief Unit vector pointing left (-1, 0)
     */
    static const Vec2 Left;

    /**
     * @brief Unit vector pointing down (0, -1)
     */
    static const Vec2 Down;

    /**
     * @brief Vector with both components set to 1 (1, 1)
     */
    static const Vec2 One;
};

// Non-member operators
/**
 * @brief Scalar multiplication (scalar * vector)
 * @param scalar Scalar value
 * @param vector Vector to multiply
 * @return Result of multiplication
 */
Vec2 operator*(f32 scalar, const Vec2& vector);

} // namespace Math
} // namespace Pyramid
