#pragma once

#include <Pyramid/Math/MathCommon.hpp>
#include <Pyramid/Math/Vec3.hpp>
#include <Pyramid/Math/Vec4.hpp>
#include <Pyramid/Math/Mat4.hpp>

// SIMD intrinsics
#ifdef _MSC_VER
    #include <intrin.h>
#else
    #include <x86intrin.h>
#endif

namespace Pyramid
{
    namespace Math
    {
        namespace SIMD
        {

            /**
             * @brief SIMD-optimized mathematical operations using SSE/AVX
             * 
             * This namespace provides high-performance implementations of common
             * mathematical operations using SIMD instructions when available.
             * Falls back to scalar implementations on unsupported platforms.
             */

            // SIMD availability detection
            struct CPUFeatures
            {
                static bool HasSSE();
                static bool HasSSE2();
                static bool HasSSE3();
                static bool HasSSE4_1();
                static bool HasAVX();
                static bool HasAVX2();
                static bool HasFMA();
            };

            // SIMD-optimized vector operations
            namespace Vec4Ops
            {
                /**
                 * @brief SIMD vector addition
                 * @param a First vector
                 * @param b Second vector
                 * @return a + b
                 */
                Vec4 Add(const Vec4& a, const Vec4& b);

                /**
                 * @brief SIMD vector subtraction
                 * @param a First vector
                 * @param b Second vector
                 * @return a - b
                 */
                Vec4 Sub(const Vec4& a, const Vec4& b);

                /**
                 * @brief SIMD vector multiplication
                 * @param a First vector
                 * @param b Second vector
                 * @return a * b (component-wise)
                 */
                Vec4 Mul(const Vec4& a, const Vec4& b);

                /**
                 * @brief SIMD scalar multiplication
                 * @param v Vector
                 * @param s Scalar
                 * @return v * s
                 */
                Vec4 Scale(const Vec4& v, f32 s);

                /**
                 * @brief SIMD dot product
                 * @param a First vector
                 * @param b Second vector
                 * @return dot(a, b)
                 */
                f32 Dot(const Vec4& a, const Vec4& b);

                /**
                 * @brief SIMD vector length
                 * @param v Vector
                 * @return |v|
                 */
                f32 Length(const Vec4& v);

                /**
                 * @brief SIMD vector normalization
                 * @param v Vector to normalize
                 * @return normalized v
                 */
                Vec4 Normalize(const Vec4& v);

                /**
                 * @brief SIMD linear interpolation
                 * @param a Start vector
                 * @param b End vector
                 * @param t Interpolation factor [0,1]
                 * @return lerp(a, b, t)
                 */
                Vec4 Lerp(const Vec4& a, const Vec4& b, f32 t);
            }

            // SIMD-optimized Vec3 operations
            namespace Vec3Ops
            {
                /**
                 * @brief SIMD Vec3 addition
                 * @param a First vector
                 * @param b Second vector
                 * @return a + b
                 */
                Vec3 Add(const Vec3& a, const Vec3& b);

                /**
                 * @brief SIMD Vec3 subtraction
                 * @param a First vector
                 * @param b Second vector
                 * @return a - b
                 */
                Vec3 Sub(const Vec3& a, const Vec3& b);

                /**
                 * @brief SIMD Vec3 multiplication
                 * @param a First vector
                 * @param b Second vector
                 * @return a * b (component-wise)
                 */
                Vec3 Mul(const Vec3& a, const Vec3& b);

                /**
                 * @brief SIMD Vec3 scalar multiplication
                 * @param v Vector
                 * @param s Scalar
                 * @return v * s
                 */
                Vec3 Scale(const Vec3& v, f32 s);

                /**
                 * @brief SIMD Vec3 dot product
                 * @param a First vector
                 * @param b Second vector
                 * @return dot(a, b)
                 */
                f32 Dot(const Vec3& a, const Vec3& b);

                /**
                 * @brief SIMD Vec3 cross product
                 * @param a First vector
                 * @param b Second vector
                 * @return cross(a, b)
                 */
                Vec3 Cross(const Vec3& a, const Vec3& b);

                /**
                 * @brief SIMD Vec3 length
                 * @param v Vector
                 * @return |v|
                 */
                f32 Length(const Vec3& v);

                /**
                 * @brief SIMD Vec3 normalization
                 * @param v Vector to normalize
                 * @return normalized v
                 */
                Vec3 Normalize(const Vec3& v);

                /**
                 * @brief SIMD Vec3 linear interpolation
                 * @param a Start vector
                 * @param b End vector
                 * @param t Interpolation factor [0,1]
                 * @return lerp(a, b, t)
                 */
                Vec3 Lerp(const Vec3& a, const Vec3& b, f32 t);
            }

            // SIMD-optimized matrix operations
            namespace Mat4Ops
            {
                /**
                 * @brief SIMD matrix multiplication
                 * @param a First matrix
                 * @param b Second matrix
                 * @return a * b
                 */
                Mat4 Multiply(const Mat4& a, const Mat4& b);

                /**
                 * @brief SIMD matrix-vector multiplication
                 * @param m Matrix
                 * @param v Vector
                 * @return m * v
                 */
                Vec4 Transform(const Mat4& m, const Vec4& v);

                /**
                 * @brief SIMD matrix transpose
                 * @param m Matrix to transpose
                 * @return transpose(m)
                 */
                Mat4 Transpose(const Mat4& m);

                /**
                 * @brief SIMD matrix determinant
                 * @param m Matrix
                 * @return determinant(m)
                 */
                f32 Determinant(const Mat4& m);

                /**
                 * @brief SIMD matrix inverse
                 * @param m Matrix to invert
                 * @return inverse(m)
                 */
                Mat4 Inverse(const Mat4& m);
            }

            // Batch SIMD operations for arrays
            namespace Batch
            {
                /**
                 * @brief SIMD batch vector addition
                 * @param a Array of first vectors
                 * @param b Array of second vectors
                 * @param result Array to store results
                 * @param count Number of vectors (must be multiple of 4 for optimal performance)
                 */
                void AddVectors(const Vec4* a, const Vec4* b, Vec4* result, i32 count);

                /**
                 * @brief SIMD batch vector normalization
                 * @param vectors Array of vectors to normalize
                 * @param result Array to store normalized vectors
                 * @param count Number of vectors
                 */
                void NormalizeVectors(const Vec4* vectors, Vec4* result, i32 count);

                /**
                 * @brief SIMD batch matrix-vector transformation
                 * @param matrix Transformation matrix
                 * @param vectors Array of vectors to transform
                 * @param result Array to store transformed vectors
                 * @param count Number of vectors
                 */
                void TransformVectors(const Mat4& matrix, const Vec4* vectors, Vec4* result, i32 count);

                /**
                 * @brief SIMD batch dot products
                 * @param a Array of first vectors
                 * @param b Array of second vectors
                 * @param result Array to store dot products
                 * @param count Number of vector pairs
                 */
                void DotProducts(const Vec4* a, const Vec4* b, f32* result, i32 count);
            }

            // Geometric operations with SIMD
            namespace Geometry
            {
                /**
                 * @brief SIMD ray-sphere intersection
                 * @param rayOrigin Ray origin
                 * @param rayDirection Ray direction (normalized)
                 * @param sphereCenter Sphere center
                 * @param sphereRadius Sphere radius
                 * @param t Output parameter for intersection distance
                 * @return true if intersection exists
                 */
                bool RaySphereIntersect(const Vec3& rayOrigin, const Vec3& rayDirection,
                                      const Vec3& sphereCenter, f32 sphereRadius, f32& t);

                /**
                 * @brief SIMD ray-plane intersection
                 * @param rayOrigin Ray origin
                 * @param rayDirection Ray direction (normalized)
                 * @param planeNormal Plane normal (normalized)
                 * @param planeDistance Plane distance from origin
                 * @param t Output parameter for intersection distance
                 * @return true if intersection exists
                 */
                bool RayPlaneIntersect(const Vec3& rayOrigin, const Vec3& rayDirection,
                                     const Vec3& planeNormal, f32 planeDistance, f32& t);

                /**
                 * @brief SIMD point-in-sphere test
                 * @param point Point to test
                 * @param sphereCenter Sphere center
                 * @param sphereRadius Sphere radius
                 * @return true if point is inside sphere
                 */
                bool PointInSphere(const Vec3& point, const Vec3& sphereCenter, f32 sphereRadius);

                /**
                 * @brief SIMD sphere-sphere intersection test
                 * @param center1 First sphere center
                 * @param radius1 First sphere radius
                 * @param center2 Second sphere center
                 * @param radius2 Second sphere radius
                 * @return true if spheres intersect
                 */
                bool SphereSphereIntersect(const Vec3& center1, f32 radius1,
                                         const Vec3& center2, f32 radius2);
            }

            // Utility functions
            /**
             * @brief Check if SIMD operations are available on this CPU
             * @return true if SIMD is supported
             */
            bool IsAvailable();

            /**
             * @brief Get a string describing available SIMD features
             * @return Feature description string
             */
            std::string GetFeatureString();

            /**
             * @brief Enable/disable SIMD optimizations at runtime
             * @param enable true to enable SIMD, false to use scalar fallback
             */
            void SetEnabled(bool enable);

            /**
             * @brief Check if SIMD optimizations are currently enabled
             * @return true if SIMD is enabled
             */
            bool IsEnabled();

        } // namespace SIMD
    } // namespace Math
} // namespace Pyramid
