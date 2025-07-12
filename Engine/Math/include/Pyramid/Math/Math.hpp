#pragma once

/**
 * @file Math.hpp
 * @brief Main header for the Pyramid Math library
 *
 * This header includes all math components and provides unified access
 * to the Pyramid::Math namespace. Include this file to access all
 * mathematical functionality in the Pyramid Engine.
 */

// Core mathematical constants and utility functions
#include <Pyramid/Math/MathCommon.hpp>

// Vector classes
#include <Pyramid/Math/Vec2.hpp>
#include <Pyramid/Math/Vec3.hpp>
#include <Pyramid/Math/Vec4.hpp>

// Matrix classes
#include <Pyramid/Math/Mat3.hpp>
#include <Pyramid/Math/Mat4.hpp>

// Quaternion class
#include <Pyramid/Math/Quat.hpp>

// Performance optimizations (optional)
#include <Pyramid/Math/MathPerformance.hpp>

// SIMD optimizations (optional, auto-detected)
#include <Pyramid/Math/MathSIMD.hpp>

namespace Pyramid
{
    namespace Math
    {

        /**
         * @brief Pyramid Math Library
         *
         * The Pyramid Math library provides comprehensive mathematical functionality
         * for 3D graphics, game development, and scientific computing.
         *
         * Key Features:
         * - Vector operations (Vec2, Vec3, Vec4)
         * - Matrix operations (Mat4, Mat3)
         * - Quaternion rotations (Quat)
         * - Transformation utilities
         * - Projection matrices
         * - Mathematical constants and functions
         *
         * All classes are designed for performance and ease of use, with
         * comprehensive operator overloading and utility functions.
         *
         * Example Usage:
         * @code
         * using namespace Pyramid::Math;
         *
         * // Vector operations
         * Vec3 position(1.0f, 2.0f, 3.0f);
         * Vec3 direction = Vec3::Forward;
         * Vec3 newPosition = position + direction * 5.0f;
         *
         * // Matrix transformations
         * Mat4 translation = Mat4::CreateTranslation(position);
         * Mat4 rotation = Mat4::CreateRotationY(Radians(45.0f));
         * Mat4 scale = Mat4::CreateScale(2.0f);
         * Mat4 transform = translation * rotation * scale;
         *
         * // Camera matrices
         * Mat4 view = Mat4::CreateLookAt(Vec3(0, 0, 5), Vec3::Zero, Vec3::Up);
         * Mat4 projection = Mat4::CreatePerspective(Radians(60.0f), 16.0f/9.0f, 0.1f, 100.0f);
         * Mat4 mvp = projection * view * transform;
         * @endcode
         */

        // Type aliases for convenience
        using Vector2 = Vec2;
        using Vector3 = Vec3;
        using Vector4 = Vec4;
        using Matrix3 = Mat3;
        using Matrix4 = Mat4;
        using Quaternion = Quat;

        // Common vector constants are available as static members of each class
        // For example: Vec2::Zero, Vec3::Forward, etc.

        // Common transformation utilities
        namespace Transform
        {
            /**
             * @brief Create a complete transformation matrix
             * @param position Translation vector
             * @param rotation Rotation angles in radians (Euler angles)
             * @param scale Scale vector
             * @return Combined transformation matrix
             */
            inline Mat4 CreateTRS(const Vec3 &position, const Vec3 &rotation, const Vec3 &scale)
            {
                Mat4 translation = Mat4::CreateTranslation(position);
                Mat4 rotationX = Mat4::CreateRotationX(rotation.x);
                Mat4 rotationY = Mat4::CreateRotationY(rotation.y);
                Mat4 rotationZ = Mat4::CreateRotationZ(rotation.z);
                Mat4 scaleMatrix = Mat4::CreateScale(scale);

                return translation * rotationZ * rotationY * rotationX * scaleMatrix;
            }

            /**
             * @brief Create a 2D transformation matrix
             * @param position 2D position
             * @param rotation Rotation angle in radians
             * @param scale 2D scale
             * @return 2D transformation matrix (as Mat4)
             */
            inline Mat4 Create2D(const Vec2 &position, f32 rotation, const Vec2 &scale)
            {
                return CreateTRS(Vec3(position, 0.0f), Vec3(0.0f, 0.0f, rotation), Vec3(scale, 1.0f));
            }
        }

        // Common camera utilities
        namespace Camera
        {
            /**
             * @brief Create a standard perspective camera matrix
             * @param fovDegrees Field of view in degrees
             * @param aspectRatio Aspect ratio (width/height)
             * @param nearPlane Near clipping plane
             * @param farPlane Far clipping plane
             * @return Perspective projection matrix
             */
            inline Mat4 CreatePerspective(f32 fovDegrees, f32 aspectRatio, f32 nearPlane, f32 farPlane)
            {
                return Mat4::CreatePerspective(Radians(fovDegrees), aspectRatio, nearPlane, farPlane);
            }

            /**
             * @brief Create a standard orthographic camera matrix
             * @param width Orthographic width
             * @param height Orthographic height
             * @param nearPlane Near clipping plane
             * @param farPlane Far clipping plane
             * @return Orthographic projection matrix
             */
            inline Mat4 CreateOrthographic(f32 width, f32 height, f32 nearPlane, f32 farPlane)
            {
                f32 halfWidth = width * 0.5f;
                f32 halfHeight = height * 0.5f;
                return Mat4::CreateOrthographic(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
            }
        }

    } // namespace Math
} // namespace Pyramid

// Global using declarations for convenience (optional)
// Uncomment these if you want to use math types without namespace qualification
// using Pyramid::Math::Vec2;
// using Pyramid::Math::Vec3;
// using Pyramid::Math::Vec4;
// using Pyramid::Math::Mat4;
