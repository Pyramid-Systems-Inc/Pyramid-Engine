#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <cmath>
#include <algorithm>

namespace Pyramid
{
    namespace Math
    {

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
            if (value > EPSILON)
                return 1.0f;
            if (value < -EPSILON)
                return -1.0f;
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
            while (angle >= TWO_PI)
                angle -= TWO_PI;
            while (angle < 0.0f)
                angle += TWO_PI;
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

        // Advanced interpolation functions
        /**
         * @brief Smooth step interpolation (3rd order polynomial)
         * @param edge0 Lower edge
         * @param edge1 Upper edge
         * @param x Input value
         * @return Smoothly interpolated value between 0 and 1
         */
        inline f32 SmoothStep(f32 edge0, f32 edge1, f32 x)
        {
            f32 t = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
            return t * t * (3.0f - 2.0f * t);
        }

        /**
         * @brief Smoother step interpolation (5th order polynomial)
         * @param edge0 Lower edge
         * @param edge1 Upper edge
         * @param x Input value
         * @return Smoothly interpolated value between 0 and 1
         */
        inline f32 SmootherStep(f32 edge0, f32 edge1, f32 x)
        {
            f32 t = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
            return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
        }

        // Easing functions
        /**
         * @brief Ease-in quadratic function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value
         */
        inline f32 EaseInQuad(f32 t)
        {
            return t * t;
        }

        /**
         * @brief Ease-out quadratic function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value
         */
        inline f32 EaseOutQuad(f32 t)
        {
            return 1.0f - (1.0f - t) * (1.0f - t);
        }

        /**
         * @brief Ease-in-out quadratic function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value
         */
        inline f32 EaseInOutQuad(f32 t)
        {
            return t < 0.5f ? 2.0f * t * t : 1.0f - Pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
        }

        /**
         * @brief Ease-in cubic function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value
         */
        inline f32 EaseInCubic(f32 t)
        {
            return t * t * t;
        }

        /**
         * @brief Ease-out cubic function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value
         */
        inline f32 EaseOutCubic(f32 t)
        {
            return 1.0f - Pow(1.0f - t, 3.0f);
        }

        /**
         * @brief Ease-in-out cubic function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value
         */
        inline f32 EaseInOutCubic(f32 t)
        {
            return t < 0.5f ? 4.0f * t * t * t : 1.0f - Pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
        }

        /**
         * @brief Ease-in sine function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value
         */
        inline f32 EaseInSine(f32 t)
        {
            return 1.0f - Cos(t * HALF_PI);
        }

        /**
         * @brief Ease-out sine function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value
         */
        inline f32 EaseOutSine(f32 t)
        {
            return Sin(t * HALF_PI);
        }

        /**
         * @brief Ease-in-out sine function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value
         */
        inline f32 EaseInOutSine(f32 t)
        {
            return -(Cos(PI * t) - 1.0f) / 2.0f;
        }

        // Random number generation utilities
        /**
         * @brief Simple pseudo-random number generator (LCG)
         * @param seed Seed value (will be modified)
         * @return Random value between 0 and RAND_MAX
         */
        inline u32 Random(u32 &seed)
        {
            seed = seed * 1664525u + 1013904223u;
            return seed;
        }

        /**
         * @brief Generate random float between 0.0 and 1.0
         * @param seed Seed value (will be modified)
         * @return Random float [0.0, 1.0]
         */
        inline f32 RandomFloat(u32 &seed)
        {
            return static_cast<f32>(Random(seed)) / static_cast<f32>(0xFFFFFFFFu);
        }

        /**
         * @brief Generate random float in range
         * @param seed Seed value (will be modified)
         * @param min Minimum value
         * @param max Maximum value
         * @return Random float [min, max]
         */
        inline f32 RandomRange(u32 &seed, f32 min, f32 max)
        {
            return min + RandomFloat(seed) * (max - min);
        }

        /**
         * @brief Generate random integer in range
         * @param seed Seed value (will be modified)
         * @param min Minimum value (inclusive)
         * @param max Maximum value (exclusive)
         * @return Random integer [min, max)
         */
        inline i32 RandomInt(u32 &seed, i32 min, i32 max)
        {
            return min + static_cast<i32>(Random(seed) % static_cast<u32>(max - min));
        }

        // Advanced mathematical functions
        /**
         * @brief Hermite interpolation
         * @param p0 Start point
         * @param m0 Start tangent
         * @param p1 End point
         * @param m1 End tangent
         * @param t Interpolation factor (0.0 to 1.0)
         * @return Interpolated value
         */
        inline f32 Hermite(f32 p0, f32 m0, f32 p1, f32 m1, f32 t)
        {
            f32 t2 = t * t;
            f32 t3 = t2 * t;

            f32 h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
            f32 h10 = t3 - 2.0f * t2 + t;
            f32 h01 = -2.0f * t3 + 3.0f * t2;
            f32 h11 = t3 - t2;

            return h00 * p0 + h10 * m0 + h01 * p1 + h11 * m1;
        }

        /**
         * @brief Catmull-Rom spline interpolation
         * @param p0 Point before start
         * @param p1 Start point
         * @param p2 End point
         * @param p3 Point after end
         * @param t Interpolation factor (0.0 to 1.0)
         * @return Interpolated value
         */
        inline f32 CatmullRom(f32 p0, f32 p1, f32 p2, f32 p3, f32 t)
        {
            f32 t2 = t * t;
            f32 t3 = t2 * t;

            return 0.5f * ((2.0f * p1) +
                           (-p0 + p2) * t +
                           (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                           (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
        }

        /**
         * @brief Bounce easing function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value with bounce effect
         */
        inline f32 EaseOutBounce(f32 t)
        {
            const f32 n1 = 7.5625f;
            const f32 d1 = 2.75f;

            if (t < 1.0f / d1)
            {
                return n1 * t * t;
            }
            else if (t < 2.0f / d1)
            {
                t -= 1.5f / d1;
                return n1 * t * t + 0.75f;
            }
            else if (t < 2.5f / d1)
            {
                t -= 2.25f / d1;
                return n1 * t * t + 0.9375f;
            }
            else
            {
                t -= 2.625f / d1;
                return n1 * t * t + 0.984375f;
            }
        }

        /**
         * @brief Elastic easing function
         * @param t Input value (0.0 to 1.0)
         * @return Eased value with elastic effect
         */
        inline f32 EaseOutElastic(f32 t)
        {
            const f32 c4 = (2.0f * PI) / 3.0f;

            if (IsZero(t))
                return 0.0f;
            if (IsEqual(t, 1.0f))
                return 1.0f;

            return Pow(2.0f, -10.0f * t) * Sin((t * 10.0f - 0.75f) * c4) + 1.0f;
        }

    } // namespace Math
} // namespace Pyramid
