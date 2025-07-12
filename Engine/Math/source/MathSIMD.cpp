#include <Pyramid/Math/MathSIMD.hpp>
#include <Pyramid/Util/Log.hpp>
#include <string>
#include <sstream>

// Platform-specific SIMD includes
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#include <x86intrin.h>
#endif

namespace Pyramid
{
    namespace Math
    {
        namespace SIMD
        {
            // Global SIMD state
            static bool g_simdEnabled = true;
            static bool g_featuresDetected = false;
            static bool g_hasSSE = false;
            static bool g_hasSSE2 = false;
            static bool g_hasSSE3 = false;
            static bool g_hasSSE4_1 = false;
            static bool g_hasAVX = false;
            static bool g_hasAVX2 = false;
            static bool g_hasFMA = false;

            // CPU feature detection
            void DetectCPUFeatures()
            {
                if (g_featuresDetected)
                    return;

                int cpuInfo[4];

#ifdef _MSC_VER
                __cpuid(cpuInfo, 1);
#else
                __cpuid(1, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#endif

                // Check SSE features (ECX and EDX registers)
                g_hasSSE = (cpuInfo[3] & (1 << 25)) != 0;
                g_hasSSE2 = (cpuInfo[3] & (1 << 26)) != 0;
                g_hasSSE3 = (cpuInfo[2] & (1 << 0)) != 0;
                g_hasSSE4_1 = (cpuInfo[2] & (1 << 19)) != 0;

                // Check AVX features
                g_hasAVX = (cpuInfo[2] & (1 << 28)) != 0;
                g_hasFMA = (cpuInfo[2] & (1 << 12)) != 0;

                // Check AVX2 (requires separate CPUID call)
#ifdef _MSC_VER
                __cpuidex(cpuInfo, 7, 0);
#else
                __cpuid_count(7, 0, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#endif
                g_hasAVX2 = (cpuInfo[1] & (1 << 5)) != 0;

                g_featuresDetected = true;

                PYRAMID_LOG_INFO("SIMD Features Detected:");
                PYRAMID_LOG_INFO("  SSE: ", g_hasSSE ? "Yes" : "No");
                PYRAMID_LOG_INFO("  SSE2: ", g_hasSSE2 ? "Yes" : "No");
                PYRAMID_LOG_INFO("  SSE3: ", g_hasSSE3 ? "Yes" : "No");
                PYRAMID_LOG_INFO("  SSE4.1: ", g_hasSSE4_1 ? "Yes" : "No");
                PYRAMID_LOG_INFO("  AVX: ", g_hasAVX ? "Yes" : "No");
                PYRAMID_LOG_INFO("  AVX2: ", g_hasAVX2 ? "Yes" : "No");
                PYRAMID_LOG_INFO("  FMA: ", g_hasFMA ? "Yes" : "No");
            }

            // CPUFeatures implementation
            bool CPUFeatures::HasSSE()
            {
                DetectCPUFeatures();
                return g_hasSSE;
            }
            bool CPUFeatures::HasSSE2()
            {
                DetectCPUFeatures();
                return g_hasSSE2;
            }
            bool CPUFeatures::HasSSE3()
            {
                DetectCPUFeatures();
                return g_hasSSE3;
            }
            bool CPUFeatures::HasSSE4_1()
            {
                DetectCPUFeatures();
                return g_hasSSE4_1;
            }
            bool CPUFeatures::HasAVX()
            {
                DetectCPUFeatures();
                return g_hasAVX;
            }
            bool CPUFeatures::HasAVX2()
            {
                DetectCPUFeatures();
                return g_hasAVX2;
            }
            bool CPUFeatures::HasFMA()
            {
                DetectCPUFeatures();
                return g_hasFMA;
            }

            // Utility functions
            bool IsAvailable()
            {
                DetectCPUFeatures();
                return g_hasSSE2; // Minimum requirement
            }

            std::string GetFeatureString()
            {
                DetectCPUFeatures();
                std::ostringstream oss;
                oss << "SIMD Features: ";
                if (g_hasSSE)
                    oss << "SSE ";
                if (g_hasSSE2)
                    oss << "SSE2 ";
                if (g_hasSSE3)
                    oss << "SSE3 ";
                if (g_hasSSE4_1)
                    oss << "SSE4.1 ";
                if (g_hasAVX)
                    oss << "AVX ";
                if (g_hasAVX2)
                    oss << "AVX2 ";
                if (g_hasFMA)
                    oss << "FMA ";
                return oss.str();
            }

            void SetEnabled(bool enable)
            {
                g_simdEnabled = enable && IsAvailable();
                PYRAMID_LOG_INFO("SIMD optimizations ", g_simdEnabled ? "enabled" : "disabled");
            }

            bool IsEnabled()
            {
                return g_simdEnabled && IsAvailable();
            }

            // Vec4 SIMD operations
            namespace Vec4Ops
            {
                Vec4 Add(const Vec4 &a, const Vec4 &b)
                {
                    if (!IsEnabled())
                    {
                        return a + b; // Fallback to scalar
                    }

                    __m128 va = _mm_load_ps(&a.x);
                    __m128 vb = _mm_load_ps(&b.x);
                    __m128 result = _mm_add_ps(va, vb);

                    Vec4 output;
                    _mm_store_ps(&output.x, result);
                    return output;
                }

                Vec4 Sub(const Vec4 &a, const Vec4 &b)
                {
                    if (!IsEnabled())
                    {
                        return a - b; // Fallback to scalar
                    }

                    __m128 va = _mm_load_ps(&a.x);
                    __m128 vb = _mm_load_ps(&b.x);
                    __m128 result = _mm_sub_ps(va, vb);

                    Vec4 output;
                    _mm_store_ps(&output.x, result);
                    return output;
                }

                Vec4 Mul(const Vec4 &a, const Vec4 &b)
                {
                    if (!IsEnabled())
                    {
                        return Vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
                    }

                    __m128 va = _mm_load_ps(&a.x);
                    __m128 vb = _mm_load_ps(&b.x);
                    __m128 result = _mm_mul_ps(va, vb);

                    Vec4 output;
                    _mm_store_ps(&output.x, result);
                    return output;
                }

                Vec4 Scale(const Vec4 &v, f32 s)
                {
                    if (!IsEnabled())
                    {
                        return v * s; // Fallback to scalar
                    }

                    __m128 vv = _mm_load_ps(&v.x);
                    __m128 vs = _mm_set1_ps(s);
                    __m128 result = _mm_mul_ps(vv, vs);

                    Vec4 output;
                    _mm_store_ps(&output.x, result);
                    return output;
                }

                f32 Dot(const Vec4 &a, const Vec4 &b)
                {
                    if (!IsEnabled())
                    {
                        return a.Dot(b); // Fallback to scalar
                    }

                    __m128 va = _mm_load_ps(&a.x);
                    __m128 vb = _mm_load_ps(&b.x);
                    __m128 mul = _mm_mul_ps(va, vb);

                    // Horizontal add: [x, y, z, w] -> x+y+z+w
                    __m128 shuf = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 3, 0, 1));
                    __m128 sums = _mm_add_ps(mul, shuf);
                    shuf = _mm_movehl_ps(shuf, sums);
                    sums = _mm_add_ss(sums, shuf);

                    return _mm_cvtss_f32(sums);
                }

                f32 Length(const Vec4 &v)
                {
                    if (!IsEnabled())
                    {
                        return v.Length(); // Fallback to scalar
                    }

                    f32 dot = Dot(v, v);
                    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(dot)));
                }

                Vec4 Normalize(const Vec4 &v)
                {
                    if (!IsEnabled())
                    {
                        return v.Normalized(); // Fallback to scalar
                    }

                    f32 length = Length(v);
                    if (Math::IsZero(length))
                    {
                        return Vec4::Zero;
                    }

                    return Scale(v, 1.0f / length);
                }

                Vec4 Lerp(const Vec4 &a, const Vec4 &b, f32 t)
                {
                    if (!IsEnabled())
                    {
                        return a.Lerp(b, t); // Fallback to scalar
                    }

                    __m128 va = _mm_load_ps(&a.x);
                    __m128 vb = _mm_load_ps(&b.x);
                    __m128 vt = _mm_set1_ps(t);

                    // lerp(a, b, t) = a + t * (b - a)
                    __m128 diff = _mm_sub_ps(vb, va);
                    __m128 scaled = _mm_mul_ps(diff, vt);
                    __m128 result = _mm_add_ps(va, scaled);

                    Vec4 output;
                    _mm_store_ps(&output.x, result);
                    return output;
                }
            }

            // Mat4 SIMD operations
            namespace Mat4Ops
            {
                Mat4 Multiply(const Mat4 &a, const Mat4 &b)
                {
                    if (!IsEnabled())
                    {
                        return a * b; // Fallback to scalar
                    }

                    Mat4 result;

                    // Load matrix B columns
                    __m128 b0 = _mm_load_ps(&b.m[0]);
                    __m128 b1 = _mm_load_ps(&b.m[4]);
                    __m128 b2 = _mm_load_ps(&b.m[8]);
                    __m128 b3 = _mm_load_ps(&b.m[12]);

                    // Multiply each column of A with B
                    for (int i = 0; i < 4; ++i)
                    {
                        __m128 a_col = _mm_load_ps(&a.m[i * 4]);

                        __m128 result_col = _mm_mul_ps(_mm_shuffle_ps(a_col, a_col, 0x00), b0);
                        result_col = _mm_add_ps(result_col, _mm_mul_ps(_mm_shuffle_ps(a_col, a_col, 0x55), b1));
                        result_col = _mm_add_ps(result_col, _mm_mul_ps(_mm_shuffle_ps(a_col, a_col, 0xAA), b2));
                        result_col = _mm_add_ps(result_col, _mm_mul_ps(_mm_shuffle_ps(a_col, a_col, 0xFF), b3));

                        _mm_store_ps(&result.m[i * 4], result_col);
                    }

                    return result;
                }

                Vec4 Transform(const Mat4 &m, const Vec4 &v)
                {
                    if (!IsEnabled())
                    {
                        return m * v; // Fallback to scalar
                    }

                    __m128 vec = _mm_load_ps(&v.x);

                    __m128 result = _mm_mul_ps(_mm_shuffle_ps(vec, vec, 0x00), _mm_load_ps(&m.m[0]));
                    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(vec, vec, 0x55), _mm_load_ps(&m.m[4])));
                    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(vec, vec, 0xAA), _mm_load_ps(&m.m[8])));
                    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(vec, vec, 0xFF), _mm_load_ps(&m.m[12])));

                    Vec4 output;
                    _mm_store_ps(&output.x, result);
                    return output;
                }

                Mat4 Transpose(const Mat4 &m)
                {
                    if (!IsEnabled())
                    {
                        return m.Transposed(); // Fallback to scalar
                    }

                    __m128 row0 = _mm_load_ps(&m.m[0]);
                    __m128 row1 = _mm_load_ps(&m.m[4]);
                    __m128 row2 = _mm_load_ps(&m.m[8]);
                    __m128 row3 = _mm_load_ps(&m.m[12]);

                    _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

                    Mat4 result;
                    _mm_store_ps(&result.m[0], row0);
                    _mm_store_ps(&result.m[4], row1);
                    _mm_store_ps(&result.m[8], row2);
                    _mm_store_ps(&result.m[12], row3);

                    return result;
                }

                f32 Determinant(const Mat4 &m)
                {
                    // For now, fallback to scalar implementation
                    // Full SIMD determinant is complex and may not provide significant benefit
                    return m.Determinant();
                }

                Mat4 Inverse(const Mat4 &m)
                {
                    // For now, fallback to scalar implementation
                    // Full SIMD inverse is very complex
                    return m.Inverse();
                }
            }

            // Geometric operations
            namespace Geometry
            {
                bool RaySphereIntersect(const Vec3 &rayOrigin, const Vec3 &rayDirection,
                                        const Vec3 &sphereCenter, f32 sphereRadius, f32 &t)
                {
                    Vec3 oc = rayOrigin - sphereCenter;
                    f32 a = rayDirection.Dot(rayDirection);
                    f32 b = 2.0f * oc.Dot(rayDirection);
                    f32 c = oc.Dot(oc) - sphereRadius * sphereRadius;

                    f32 discriminant = b * b - 4 * a * c;
                    if (discriminant < 0)
                    {
                        return false;
                    }

                    f32 sqrt_discriminant = Math::SafeSqrt(discriminant);
                    f32 t1 = (-b - sqrt_discriminant) / (2.0f * a);
                    f32 t2 = (-b + sqrt_discriminant) / (2.0f * a);

                    if (t1 > 0)
                    {
                        t = t1;
                        return true;
                    }
                    else if (t2 > 0)
                    {
                        t = t2;
                        return true;
                    }

                    return false;
                }

                bool RayPlaneIntersect(const Vec3 &rayOrigin, const Vec3 &rayDirection,
                                       const Vec3 &planeNormal, f32 planeDistance, f32 &t)
                {
                    f32 denom = planeNormal.Dot(rayDirection);
                    if (Math::IsZero(denom))
                    {
                        return false; // Ray is parallel to plane
                    }

                    t = (planeDistance - planeNormal.Dot(rayOrigin)) / denom;
                    return t >= 0;
                }

                bool PointInSphere(const Vec3 &point, const Vec3 &sphereCenter, f32 sphereRadius)
                {
                    f32 distanceSquared = (point - sphereCenter).LengthSquared();
                    return distanceSquared <= sphereRadius * sphereRadius;
                }

                bool SphereSphereIntersect(const Vec3 &center1, f32 radius1,
                                           const Vec3 &center2, f32 radius2)
                {
                    f32 distanceSquared = (center2 - center1).LengthSquared();
                    f32 radiusSum = radius1 + radius2;
                    return distanceSquared <= radiusSum * radiusSum;
                }
            }

            // Batch operations
            namespace Batch
            {
                void AddVectors(const Vec4 *a, const Vec4 *b, Vec4 *result, i32 count)
                {
                    for (i32 i = 0; i < count; ++i)
                    {
                        result[i] = Vec4Ops::Add(a[i], b[i]);
                    }
                }

                void NormalizeVectors(const Vec4 *vectors, Vec4 *result, i32 count)
                {
                    for (i32 i = 0; i < count; ++i)
                    {
                        result[i] = Vec4Ops::Normalize(vectors[i]);
                    }
                }

                void TransformVectors(const Mat4 &matrix, const Vec4 *vectors, Vec4 *result, i32 count)
                {
                    for (i32 i = 0; i < count; ++i)
                    {
                        result[i] = Mat4Ops::Transform(matrix, vectors[i]);
                    }
                }

                void DotProducts(const Vec4 *a, const Vec4 *b, f32 *result, i32 count)
                {
                    for (i32 i = 0; i < count; ++i)
                    {
                        result[i] = Vec4Ops::Dot(a[i], b[i]);
                    }
                }
            }

        } // namespace SIMD
    } // namespace Math
} // namespace Pyramid
