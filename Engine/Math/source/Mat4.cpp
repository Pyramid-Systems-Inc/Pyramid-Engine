#include <Pyramid/Math/Mat4.hpp>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace Pyramid
{
    namespace Math
    {

        // Static member definitions
        const Mat4 Mat4::Identity = Mat4::CreateIdentity();
        const Mat4 Mat4::Zero = Mat4::CreateZero();

        // Constructors
        Mat4::Mat4()
        {
            SetIdentity();
        }

        Mat4::Mat4(f32 diagonal)
        {
            SetZero();
            m[0] = m[5] = m[10] = m[15] = diagonal;
        }

        Mat4::Mat4(f32 m00, f32 m01, f32 m02, f32 m03,
                   f32 m10, f32 m11, f32 m12, f32 m13,
                   f32 m20, f32 m21, f32 m22, f32 m23,
                   f32 m30, f32 m31, f32 m32, f32 m33)
        {
            // Convert from row-major input to column-major storage
            m[0] = m00;
            m[4] = m01;
            m[8] = m02;
            m[12] = m03;
            m[1] = m10;
            m[5] = m11;
            m[9] = m12;
            m[13] = m13;
            m[2] = m20;
            m[6] = m21;
            m[10] = m22;
            m[14] = m23;
            m[3] = m30;
            m[7] = m31;
            m[11] = m32;
            m[15] = m33;
        }

        Mat4::Mat4(const f32 *data)
        {
            std::memcpy(m, data, 16 * sizeof(f32));
        }

        // Element access
        f32 Mat4::Get(i32 col, i32 row) const
        {
            return m[col * 4 + row];
        }

        void Mat4::Set(i32 col, i32 row, f32 value)
        {
            m[col * 4 + row] = value;
        }

        f32 Mat4::operator[](i32 index) const
        {
            return m[index];
        }

        f32 &Mat4::operator[](i32 index)
        {
            return m[index];
        }

        // Column and row access
        Vec4 Mat4::GetColumn(i32 col) const
        {
            i32 offset = col * 4;
            return Vec4(m[offset], m[offset + 1], m[offset + 2], m[offset + 3]);
        }

        void Mat4::SetColumn(i32 col, const Vec4 &column)
        {
            i32 offset = col * 4;
            m[offset] = column.x;
            m[offset + 1] = column.y;
            m[offset + 2] = column.z;
            m[offset + 3] = column.w;
        }

        Vec4 Mat4::GetRow(i32 row) const
        {
            return Vec4(m[row], m[row + 4], m[row + 8], m[row + 12]);
        }

        void Mat4::SetRow(i32 row, const Vec4 &rowVec)
        {
            m[row] = rowVec.x;
            m[row + 4] = rowVec.y;
            m[row + 8] = rowVec.z;
            m[row + 12] = rowVec.w;
        }

        // Arithmetic operators
        Mat4 Mat4::operator+(const Mat4 &other) const
        {
            Mat4 result;
            for (i32 i = 0; i < 16; ++i)
            {
                result.m[i] = m[i] + other.m[i];
            }
            return result;
        }

        Mat4 Mat4::operator-(const Mat4 &other) const
        {
            Mat4 result;
            for (i32 i = 0; i < 16; ++i)
            {
                result.m[i] = m[i] - other.m[i];
            }
            return result;
        }

        Mat4 Mat4::operator*(const Mat4 &other) const
        {
            Mat4 result;

            for (i32 col = 0; col < 4; ++col)
            {
                for (i32 row = 0; row < 4; ++row)
                {
                    f32 sum = 0.0f;
                    for (i32 k = 0; k < 4; ++k)
                    {
                        sum += Get(k, row) * other.Get(col, k);
                    }
                    result.Set(col, row, sum);
                }
            }

            return result;
        }

        Mat4 Mat4::operator*(f32 scalar) const
        {
            Mat4 result;
            for (i32 i = 0; i < 16; ++i)
            {
                result.m[i] = m[i] * scalar;
            }
            return result;
        }

        Vec4 Mat4::operator*(const Vec4 &vec) const
        {
            return Vec4(
                Get(0, 0) * vec.x + Get(1, 0) * vec.y + Get(2, 0) * vec.z + Get(3, 0) * vec.w,
                Get(0, 1) * vec.x + Get(1, 1) * vec.y + Get(2, 1) * vec.z + Get(3, 1) * vec.w,
                Get(0, 2) * vec.x + Get(1, 2) * vec.y + Get(2, 2) * vec.z + Get(3, 2) * vec.w,
                Get(0, 3) * vec.x + Get(1, 3) * vec.y + Get(2, 3) * vec.z + Get(3, 3) * vec.w);
        }

        Mat4 Mat4::operator-() const
        {
            Mat4 result;
            for (i32 i = 0; i < 16; ++i)
            {
                result.m[i] = -m[i];
            }
            return result;
        }

        // Compound assignment operators
        Mat4 &Mat4::operator+=(const Mat4 &other)
        {
            for (i32 i = 0; i < 16; ++i)
            {
                m[i] += other.m[i];
            }
            return *this;
        }

        Mat4 &Mat4::operator-=(const Mat4 &other)
        {
            for (i32 i = 0; i < 16; ++i)
            {
                m[i] -= other.m[i];
            }
            return *this;
        }

        Mat4 &Mat4::operator*=(const Mat4 &other)
        {
            *this = *this * other;
            return *this;
        }

        Mat4 &Mat4::operator*=(f32 scalar)
        {
            for (i32 i = 0; i < 16; ++i)
            {
                m[i] *= scalar;
            }
            return *this;
        }

        // Comparison operators
        bool Mat4::operator==(const Mat4 &other) const
        {
            for (i32 i = 0; i < 16; ++i)
            {
                if (!IsEqual(m[i], other.m[i]))
                {
                    return false;
                }
            }
            return true;
        }

        bool Mat4::operator!=(const Mat4 &other) const
        {
            return !(*this == other);
        }

        // Matrix operations
        Mat4 Mat4::Transpose() const
        {
            Mat4 result;
            for (i32 col = 0; col < 4; ++col)
            {
                for (i32 row = 0; row < 4; ++row)
                {
                    result.Set(row, col, Get(col, row));
                }
            }
            return result;
        }

        Mat4 &Mat4::TransposeInPlace()
        {
            for (i32 col = 0; col < 4; ++col)
            {
                for (i32 row = col + 1; row < 4; ++row)
                {
                    f32 temp = Get(col, row);
                    Set(col, row, Get(row, col));
                    Set(row, col, temp);
                }
            }
            return *this;
        }

        f32 Mat4::Determinant() const
        {
            // Calculate determinant using cofactor expansion along first row
            f32 det = 0.0f;

            for (i32 col = 0; col < 4; ++col)
            {
                // Calculate 3x3 minor determinant
                f32 minor = 0.0f;
                i32 sign = (col % 2 == 0) ? 1 : -1;

                // This is a simplified version - full implementation would be more complex
                // For now, we'll use a basic approach
                minor = Get(col, 0);
                det += sign * minor;
            }

            return det;
        }

        Mat4 Mat4::Inverse() const
        {
            // For now, return identity if not easily invertible
            // Full inverse calculation is complex and will be implemented later
            f32 det = Determinant();
            if (IsZero(det))
            {
                return Mat4::Identity;
            }

            // Simplified inverse - full implementation needed
            return Mat4::Identity;
        }

        Mat4 &Mat4::InvertInPlace()
        {
            *this = Inverse();
            return *this;
        }

        bool Mat4::IsInvertible() const
        {
            return !IsZero(Determinant());
        }

        bool Mat4::IsIdentity() const
        {
            return *this == Mat4::Identity;
        }

        Mat4 &Mat4::SetIdentity()
        {
            SetZero();
            m[0] = m[5] = m[10] = m[15] = 1.0f;
            return *this;
        }

        Mat4 &Mat4::SetZero()
        {
            std::memset(m, 0, 16 * sizeof(f32));
            return *this;
        }

        const f32 *Mat4::Data() const
        {
            return m;
        }

        f32 *Mat4::Data()
        {
            return m;
        }

        std::string Mat4::ToString() const
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "Mat4(\n";
            for (i32 row = 0; row < 4; ++row)
            {
                oss << "  [";
                for (i32 col = 0; col < 4; ++col)
                {
                    oss << Get(col, row);
                    if (col < 3)
                        oss << ", ";
                }
                oss << "]\n";
            }
            oss << ")";
            return oss.str();
        }

        // Static utility functions
        Mat4 Mat4::CreateIdentity()
        {
            Mat4 result;
            result.SetIdentity();
            return result;
        }

        Mat4 Mat4::CreateZero()
        {
            Mat4 result;
            result.SetZero();
            return result;
        }

        // Transformation matrix creation functions
        Mat4 Mat4::CreateTranslation(const Vec3 &translation)
        {
            return CreateTranslation(translation.x, translation.y, translation.z);
        }

        Mat4 Mat4::CreateTranslation(f32 x, f32 y, f32 z)
        {
            Mat4 result = Mat4::Identity;
            result.Set(3, 0, x);
            result.Set(3, 1, y);
            result.Set(3, 2, z);
            return result;
        }

        Mat4 Mat4::CreateScale(const Vec3 &scale)
        {
            return CreateScale(scale.x, scale.y, scale.z);
        }

        Mat4 Mat4::CreateScale(f32 scale)
        {
            return CreateScale(scale, scale, scale);
        }

        Mat4 Mat4::CreateScale(f32 x, f32 y, f32 z)
        {
            Mat4 result = Mat4::Identity;
            result.Set(0, 0, x);
            result.Set(1, 1, y);
            result.Set(2, 2, z);
            return result;
        }

        Mat4 Mat4::CreateRotationX(f32 angle)
        {
            Mat4 result = Mat4::Identity;
            f32 cosAngle = Cos(angle);
            f32 sinAngle = Sin(angle);

            result.Set(1, 1, cosAngle);
            result.Set(1, 2, sinAngle);
            result.Set(2, 1, -sinAngle);
            result.Set(2, 2, cosAngle);

            return result;
        }

        Mat4 Mat4::CreateRotationY(f32 angle)
        {
            Mat4 result = Mat4::Identity;
            f32 cosAngle = Cos(angle);
            f32 sinAngle = Sin(angle);

            result.Set(0, 0, cosAngle);
            result.Set(0, 2, -sinAngle);
            result.Set(2, 0, sinAngle);
            result.Set(2, 2, cosAngle);

            return result;
        }

        Mat4 Mat4::CreateRotationZ(f32 angle)
        {
            Mat4 result = Mat4::Identity;
            f32 cosAngle = Cos(angle);
            f32 sinAngle = Sin(angle);

            result.Set(0, 0, cosAngle);
            result.Set(0, 1, sinAngle);
            result.Set(1, 0, -sinAngle);
            result.Set(1, 1, cosAngle);

            return result;
        }

        Mat4 Mat4::CreateRotation(const Vec3 &axis, f32 angle)
        {
            Vec3 normalizedAxis = axis.Normalized();
            f32 cosAngle = Cos(angle);
            f32 sinAngle = Sin(angle);
            f32 oneMinusCos = 1.0f - cosAngle;

            f32 x = normalizedAxis.x;
            f32 y = normalizedAxis.y;
            f32 z = normalizedAxis.z;

            Mat4 result;
            result.Set(0, 0, cosAngle + x * x * oneMinusCos);
            result.Set(1, 0, x * y * oneMinusCos - z * sinAngle);
            result.Set(2, 0, x * z * oneMinusCos + y * sinAngle);
            result.Set(3, 0, 0.0f);

            result.Set(0, 1, y * x * oneMinusCos + z * sinAngle);
            result.Set(1, 1, cosAngle + y * y * oneMinusCos);
            result.Set(2, 1, y * z * oneMinusCos - x * sinAngle);
            result.Set(3, 1, 0.0f);

            result.Set(0, 2, z * x * oneMinusCos - y * sinAngle);
            result.Set(1, 2, z * y * oneMinusCos + x * sinAngle);
            result.Set(2, 2, cosAngle + z * z * oneMinusCos);
            result.Set(3, 2, 0.0f);

            result.Set(0, 3, 0.0f);
            result.Set(1, 3, 0.0f);
            result.Set(2, 3, 0.0f);
            result.Set(3, 3, 1.0f);

            return result;
        }

        Mat4 Mat4::CreatePerspective(f32 fov, f32 aspect, f32 nearPlane, f32 farPlane)
        {
            f32 tanHalfFov = Tan(fov * 0.5f);
            f32 range = farPlane - nearPlane;

            Mat4 result;
            result.SetZero();

            result.Set(0, 0, 1.0f / (aspect * tanHalfFov));
            result.Set(1, 1, 1.0f / tanHalfFov);
            result.Set(2, 2, -(farPlane + nearPlane) / range);
            result.Set(3, 2, -2.0f * farPlane * nearPlane / range);
            result.Set(2, 3, -1.0f);

            return result;
        }

        Mat4 Mat4::CreateOrthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearPlane, f32 farPlane)
        {
            Mat4 result;
            result.SetZero();

            result.Set(0, 0, 2.0f / (right - left));
            result.Set(1, 1, 2.0f / (top - bottom));
            result.Set(2, 2, -2.0f / (farPlane - nearPlane));
            result.Set(3, 0, -(right + left) / (right - left));
            result.Set(3, 1, -(top + bottom) / (top - bottom));
            result.Set(3, 2, -(farPlane + nearPlane) / (farPlane - nearPlane));
            result.Set(3, 3, 1.0f);

            return result;
        }

        Mat4 Mat4::CreateLookAt(const Vec3 &eye, const Vec3 &target, const Vec3 &up)
        {
            Vec3 forward = (target - eye).Normalized();
            Vec3 right = forward.Cross(up).Normalized();
            Vec3 newUp = right.Cross(forward);

            Mat4 result;
            result.SetIdentity();

            result.Set(0, 0, right.x);
            result.Set(1, 0, right.y);
            result.Set(2, 0, right.z);
            result.Set(0, 1, newUp.x);
            result.Set(1, 1, newUp.y);
            result.Set(2, 1, newUp.z);
            result.Set(0, 2, -forward.x);
            result.Set(1, 2, -forward.y);
            result.Set(2, 2, -forward.z);
            result.Set(3, 0, -right.Dot(eye));
            result.Set(3, 1, -newUp.Dot(eye));
            result.Set(3, 2, forward.Dot(eye));

            return result;
        }

        // Non-member operators
        Mat4 operator*(f32 scalar, const Mat4 &matrix)
        {
            return matrix * scalar;
        }

    } // namespace Math
} // namespace Pyramid
