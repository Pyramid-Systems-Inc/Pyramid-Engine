#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <cmath>
#include <algorithm>

namespace Pyramid {
namespace Math {

// Mathematical constants
constexpr f32 PI = 3.14159265359f;
constexpr f32 TWO_PI = 2.0f * PI;
constexpr f32 HALF_PI = PI / 2.0f;
constexpr f32 QUARTER_PI = PI / 4.0f;
constexpr f32 DEG_TO_RAD = PI / 180.0f;
constexpr f32 RAD_TO_DEG = 180.0f / PI;

// Floating point precision constants
constexpr f32 EPSILON = 1e-6f;
constexpr f32 SMALL_NUMBER = 1e-8f;
constexpr f32 BIG_NUMBER = 3.4e+38f;

/**
 * @brief Convert degrees to radians
 * @param degrees Angle in degrees
 * @return Angle in radians
 */
inline f32 Radians(f32 degrees)
{
    return degrees * DEG_TO_RAD;
}

/**
 * @brief Convert radians to degrees
 * @param radians Angle in radians
 * @return Angle in degrees
 */
inline f32 Degrees(f32 radians)
{
    return radians * RAD_TO_DEG;
}

/**
 * @brief Clamp a value between min and max
 * @param value Value to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped value
 */
inline f32 Clamp(f32 value, f32 min, f32 max)
{
    return std::max(min, std::min(max, value));
}

/**
 * @brief Linear interpolation between two values
 * @param a Start value
 * @param b End value
 * @param t Interpolation factor (0.0 to 1.0)
 * @return Interpolated value
 */
inline f32 Lerp(f32 a, f32 b, f32 t)
{
    return a + t * (b - a);
}

/**
 * @brief Check if two floating point values are approximately equal
 * @param a First value
 * @param b Second value
 * @param epsilon Tolerance for comparison
 * @return True if values are approximately equal
 */
inline bool IsEqual(f32 a, f32 b, f32 epsilon = EPSILON)
{
    return std::abs(a - b) <= epsilon;
}

/**
 * @brief Check if a floating point value is approximately zero
 * @param value Value to check
 * @param epsilon Tolerance for comparison
 * @return True if value is approximately zero
 */
inline bool IsZero(f32 value, f32 epsilon = EPSILON)
{
    return std::abs(value) <= epsilon;
}

/**
 * @brief Get the sign of a value
 * @param value Value to check
 * @return 1.0f if positive, -1.0f if negative, 0.0f if zero
 */
inline f32 Sign(f32 value)
{
    if (value > EPSILON) return 1.0f;
    if (value < -EPSILON) return -1.0f;
    return 0.0f;
}

/**
 * @brief Square a value
 * @param value Value to square
 * @return value * value
 */
inline f32 Square(f32 value)
{
    return value * value;
}

/**
 * @brief Safe square root that handles negative values
 * @param value Value to take square root of
 * @return Square root, or 0.0f if value is negative
 */
inline f32 SafeSqrt(f32 value)
{
    return value >= 0.0f ? std::sqrt(value) : 0.0f;
}

/**
 * @brief Wrap an angle to the range [0, 2*PI)
 * @param angle Angle in radians
 * @return Wrapped angle
 */
inline f32 WrapAngle(f32 angle)
{
    while (angle >= TWO_PI) angle -= TWO_PI;
    while (angle < 0.0f) angle += TWO_PI;
    return angle;
}

/**
 * @brief Get the minimum of two values
 * @param a First value
 * @param b Second value
 * @return Minimum value
 */
inline f32 Min(f32 a, f32 b)
{
    return std::min(a, b);
}

/**
 * @brief Get the maximum of two values
 * @param a First value
 * @param b Second value
 * @return Maximum value
 */
inline f32 Max(f32 a, f32 b)
{
    return std::max(a, b);
}

/**
 * @brief Get the absolute value
 * @param value Value to get absolute value of
 * @return Absolute value
 */
inline f32 Abs(f32 value)
{
    return std::abs(value);
}

/**
 * @brief Round a value to the nearest integer
 * @param value Value to round
 * @return Rounded value
 */
inline f32 Round(f32 value)
{
    return std::round(value);
}

/**
 * @brief Floor a value to the nearest lower integer
 * @param value Value to floor
 * @return Floored value
 */
inline f32 Floor(f32 value)
{
    return std::floor(value);
}

/**
 * @brief Ceiling a value to the nearest higher integer
 * @param value Value to ceiling
 * @return Ceiled value
 */
inline f32 Ceil(f32 value)
{
    return std::ceil(value);
}

/**
 * @brief Calculate power of a value
 * @param base Base value
 * @param exponent Exponent
 * @return base^exponent
 */
inline f32 Pow(f32 base, f32 exponent)
{
    return std::pow(base, exponent);
}

/**
 * @brief Calculate sine of an angle
 * @param angle Angle in radians
 * @return Sine value
 */
inline f32 Sin(f32 angle)
{
    return std::sin(angle);
}

/**
 * @brief Calculate cosine of an angle
 * @param angle Angle in radians
 * @return Cosine value
 */
inline f32 Cos(f32 angle)
{
    return std::cos(angle);
}

/**
 * @brief Calculate tangent of an angle
 * @param angle Angle in radians
 * @return Tangent value
 */
inline f32 Tan(f32 angle)
{
    return std::tan(angle);
}

/**
 * @brief Calculate arc sine
 * @param value Value between -1 and 1
 * @return Arc sine in radians
 */
inline f32 Asin(f32 value)
{
    return std::asin(Clamp(value, -1.0f, 1.0f));
}

/**
 * @brief Calculate arc cosine
 * @param value Value between -1 and 1
 * @return Arc cosine in radians
 */
inline f32 Acos(f32 value)
{
    return std::acos(Clamp(value, -1.0f, 1.0f));
}

/**
 * @brief Calculate arc tangent
 * @param value Value
 * @return Arc tangent in radians
 */
inline f32 Atan(f32 value)
{
    return std::atan(value);
}

/**
 * @brief Calculate arc tangent of y/x
 * @param y Y component
 * @param x X component
 * @return Arc tangent in radians
 */
inline f32 Atan2(f32 y, f32 x)
{
    return std::atan2(y, x);
}

} // namespace Math
} // namespace Pyramid
