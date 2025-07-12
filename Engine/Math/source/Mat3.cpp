#include <Pyramid/Math/Mat3.hpp>
#include <Pyramid/Math/Mat4.hpp>
#include <Pyramid/Math/Quat.hpp>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace Pyramid
{
    namespace Math
    {

        // Static member definitions
        const Mat3 Mat3::Identity = Mat3::CreateIdentity();
        const Mat3 Mat3::Zero = Mat3::CreateZero();

        // Constructors
        Mat3::Mat3()
        {
            SetIdentity();
        }

        Mat3::Mat3(f32 diagonal)
        {
            SetZero();
            m[0] = m[4] = m[8] = diagonal;
        }

        Mat3::Mat3(f32 m00, f32 m01, f32 m02,
                   f32 m10, f32 m11, f32 m12,
                   f32 m20, f32 m21, f32 m22)
        {
            // Convert from row-major input to column-major storage
            m[0] = m00;
            m[3] = m01;
            m[6] = m02;
            m[1] = m10;
            m[4] = m11;
            m[7] = m12;
            m[2] = m20;
            m[5] = m21;
            m[8] = m22;
        }

        Mat3::Mat3(const f32 *data)
        {
            std::memcpy(m, data, 9 * sizeof(f32));
        }

        Mat3::Mat3(const Mat4 &mat4)
        {
            // Extract upper-left 3x3 from Mat4
            m[0] = mat4.Get(0, 0);
            m[3] = mat4.Get(1, 0);
            m[6] = mat4.Get(2, 0);
            m[1] = mat4.Get(0, 1);
            m[4] = mat4.Get(1, 1);
            m[7] = mat4.Get(2, 1);
            m[2] = mat4.Get(0, 2);
            m[5] = mat4.Get(1, 2);
            m[8] = mat4.Get(2, 2);
        }

        Mat3::Mat3(const Quat &quat)
        {
            *this = CreateRotation(quat);
        }

        // Element access
        f32 Mat3::Get(i32 col, i32 row) const
        {
            return m[col * 3 + row];
        }

        void Mat3::Set(i32 col, i32 row, f32 value)
        {
            m[col * 3 + row] = value;
        }

        f32 Mat3::operator[](i32 index) const
        {
            return m[index];
        }

        f32 &Mat3::operator[](i32 index)
        {
            return m[index];
        }

        // Column and row access
        Vec3 Mat3::GetColumn(i32 col) const
        {
            i32 offset = col * 3;
            return Vec3(m[offset], m[offset + 1], m[offset + 2]);
        }

        void Mat3::SetColumn(i32 col, const Vec3 &column)
        {
            i32 offset = col * 3;
            m[offset] = column.x;
            m[offset + 1] = column.y;
            m[offset + 2] = column.z;
        }

        Vec3 Mat3::GetRow(i32 row) const
        {
            return Vec3(m[row], m[row + 3], m[row + 6]);
        }

        void Mat3::SetRow(i32 row, const Vec3 &rowVec)
        {
            m[row] = rowVec.x;
            m[row + 3] = rowVec.y;
            m[row + 6] = rowVec.z;
        }

        // Arithmetic operators
        Mat3 Mat3::operator+(const Mat3 &other) const
        {
            Mat3 result;
            for (i32 i = 0; i < 9; ++i)
            {
                result.m[i] = m[i] + other.m[i];
            }
            return result;
        }

        Mat3 Mat3::operator-(const Mat3 &other) const
        {
            Mat3 result;
            for (i32 i = 0; i < 9; ++i)
            {
                result.m[i] = m[i] - other.m[i];
            }
            return result;
        }

        Mat3 Mat3::operator*(const Mat3 &other) const
        {
            Mat3 result;

            for (i32 col = 0; col < 3; ++col)
            {
                for (i32 row = 0; row < 3; ++row)
                {
                    f32 sum = 0.0f;
                    for (i32 k = 0; k < 3; ++k)
                    {
                        sum += Get(k, row) * other.Get(col, k);
                    }
                    result.Set(col, row, sum);
                }
            }

            return result;
        }

        Mat3 Mat3::operator*(f32 scalar) const
        {
            Mat3 result;
            for (i32 i = 0; i < 9; ++i)
            {
                result.m[i] = m[i] * scalar;
            }
            return result;
        }

        Vec3 Mat3::operator*(const Vec3 &vec) const
        {
            return Vec3(
                Get(0, 0) * vec.x + Get(1, 0) * vec.y + Get(2, 0) * vec.z,
                Get(0, 1) * vec.x + Get(1, 1) * vec.y + Get(2, 1) * vec.z,
                Get(0, 2) * vec.x + Get(1, 2) * vec.y + Get(2, 2) * vec.z);
        }

        Mat3 Mat3::operator-() const
        {
            Mat3 result;
            for (i32 i = 0; i < 9; ++i)
            {
                result.m[i] = -m[i];
            }
            return result;
        }

        // Compound assignment operators
        Mat3 &Mat3::operator+=(const Mat3 &other)
        {
            for (i32 i = 0; i < 9; ++i)
            {
                m[i] += other.m[i];
            }
            return *this;
        }

        Mat3 &Mat3::operator-=(const Mat3 &other)
        {
            for (i32 i = 0; i < 9; ++i)
            {
                m[i] -= other.m[i];
            }
            return *this;
        }

        Mat3 &Mat3::operator*=(const Mat3 &other)
        {
            *this = *this * other;
            return *this;
        }

        Mat3 &Mat3::operator*=(f32 scalar)
        {
            for (i32 i = 0; i < 9; ++i)
            {
                m[i] *= scalar;
            }
            return *this;
        }

        // Comparison operators
        bool Mat3::operator==(const Mat3 &other) const
        {
            for (i32 i = 0; i < 9; ++i)
            {
                if (!IsEqual(m[i], other.m[i]))
                {
                    return false;
                }
            }
            return true;
        }

        bool Mat3::operator!=(const Mat3 &other) const
        {
            return !(*this == other);
        }

        // Matrix operations
        Mat3 Mat3::Transpose() const
        {
            Mat3 result;
            for (i32 col = 0; col < 3; ++col)
            {
                for (i32 row = 0; row < 3; ++row)
                {
                    result.Set(row, col, Get(col, row));
                }
            }
            return result;
        }

        Mat3 &Mat3::TransposeInPlace()
        {
            for (i32 col = 0; col < 3; ++col)
            {
                for (i32 row = col + 1; row < 3; ++row)
                {
                    f32 temp = Get(col, row);
                    Set(col, row, Get(row, col));
                    Set(row, col, temp);
                }
            }
            return *this;
        }

        f32 Mat3::Determinant() const
        {
            return Get(0, 0) * (Get(1, 1) * Get(2, 2) - Get(1, 2) * Get(2, 1)) -
                   Get(1, 0) * (Get(0, 1) * Get(2, 2) - Get(0, 2) * Get(2, 1)) +
                   Get(2, 0) * (Get(0, 1) * Get(1, 2) - Get(0, 2) * Get(1, 1));
        }

        Mat3 Mat3::Inverse() const
        {
            f32 det = Determinant();
            if (Math::IsZero(det))
            {
                return Mat3::Identity;
            }

            f32 invDet = 1.0f / det;

            Mat3 result;
            result.Set(0, 0, (Get(1, 1) * Get(2, 2) - Get(1, 2) * Get(2, 1)) * invDet);
            result.Set(1, 0, (Get(1, 2) * Get(2, 0) - Get(1, 0) * Get(2, 2)) * invDet);
            result.Set(2, 0, (Get(1, 0) * Get(2, 1) - Get(1, 1) * Get(2, 0)) * invDet);
            result.Set(0, 1, (Get(0, 2) * Get(2, 1) - Get(0, 1) * Get(2, 2)) * invDet);
            result.Set(1, 1, (Get(0, 0) * Get(2, 2) - Get(0, 2) * Get(2, 0)) * invDet);
            result.Set(2, 1, (Get(0, 1) * Get(2, 0) - Get(0, 0) * Get(2, 1)) * invDet);
            result.Set(0, 2, (Get(0, 1) * Get(1, 2) - Get(0, 2) * Get(1, 1)) * invDet);
            result.Set(1, 2, (Get(0, 2) * Get(1, 0) - Get(0, 0) * Get(1, 2)) * invDet);
            result.Set(2, 2, (Get(0, 0) * Get(1, 1) - Get(0, 1) * Get(1, 0)) * invDet);

            return result;
        }

        Mat3 &Mat3::InvertInPlace()
        {
            *this = Inverse();
            return *this;
        }

        bool Mat3::IsInvertible() const
        {
            return !Math::IsZero(Determinant());
        }

        bool Mat3::IsIdentity() const
        {
            return *this == Mat3::Identity;
        }

        Mat3 &Mat3::SetIdentity()
        {
            SetZero();
            m[0] = m[4] = m[8] = 1.0f;
            return *this;
        }

        Mat3 &Mat3::SetZero()
        {
            std::memset(m, 0, 9 * sizeof(f32));
            return *this;
        }

        const f32 *Mat3::Data() const
        {
            return m;
        }

        f32 *Mat3::Data()
        {
            return m;
        }

        std::string Mat3::ToString() const
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "Mat3(\n";
            for (i32 row = 0; row < 3; ++row)
            {
                oss << "  [";
                for (i32 col = 0; col < 3; ++col)
                {
                    oss << Get(col, row);
                    if (col < 2)
                        oss << ", ";
                }
                oss << "]\n";
            }
            oss << ")";
            return oss.str();
        }

        // Static utility functions
        Mat3 Mat3::CreateIdentity()
        {
            Mat3 result;
            result.SetIdentity();
            return result;
        }

        Mat3 Mat3::CreateZero()
        {
            Mat3 result;
            result.SetZero();
            return result;
        }

        // Transformation matrix creation functions
        Mat3 Mat3::CreateTranslation(const Vec2 &translation)
        {
            return CreateTranslation(translation.x, translation.y);
        }

        Mat3 Mat3::CreateTranslation(f32 x, f32 y)
        {
            Mat3 result = Mat3::Identity;
            result.Set(2, 0, x);
            result.Set(2, 1, y);
            return result;
        }

        Mat3 Mat3::CreateScale(const Vec2 &scale)
        {
            return CreateScale(scale.x, scale.y);
        }

        Mat3 Mat3::CreateScale(f32 scale)
        {
            return CreateScale(scale, scale);
        }

        Mat3 Mat3::CreateScale(f32 x, f32 y)
        {
            Mat3 result = Mat3::Identity;
            result.Set(0, 0, x);
            result.Set(1, 1, y);
            return result;
        }

        Mat3 Mat3::CreateRotation(f32 angle)
        {
            f32 cosAngle = Cos(angle);
            f32 sinAngle = Sin(angle);

            Mat3 result = Mat3::Identity;
            result.Set(0, 0, cosAngle);
            result.Set(0, 1, sinAngle);
            result.Set(1, 0, -sinAngle);
            result.Set(1, 1, cosAngle);

            return result;
        }

        Mat3 Mat3::CreateRotationX(f32 angle)
        {
            f32 cosAngle = Cos(angle);
            f32 sinAngle = Sin(angle);

            Mat3 result = Mat3::Identity;
            result.Set(1, 1, cosAngle);
            result.Set(1, 2, sinAngle);
            result.Set(2, 1, -sinAngle);
            result.Set(2, 2, cosAngle);

            return result;
        }

        Mat3 Mat3::CreateRotationY(f32 angle)
        {
            f32 cosAngle = Cos(angle);
            f32 sinAngle = Sin(angle);

            Mat3 result = Mat3::Identity;
            result.Set(0, 0, cosAngle);
            result.Set(0, 2, -sinAngle);
            result.Set(2, 0, sinAngle);
            result.Set(2, 2, cosAngle);

            return result;
        }

        Mat3 Mat3::CreateRotationZ(f32 angle)
        {
            f32 cosAngle = Cos(angle);
            f32 sinAngle = Sin(angle);

            Mat3 result = Mat3::Identity;
            result.Set(0, 0, cosAngle);
            result.Set(0, 1, sinAngle);
            result.Set(1, 0, -sinAngle);
            result.Set(1, 1, cosAngle);

            return result;
        }

        Mat3 Mat3::CreateRotation(const Vec3 &axis, f32 angle)
        {
            Vec3 normalizedAxis = axis.Normalized();
            f32 cosAngle = Cos(angle);
            f32 sinAngle = Sin(angle);
            f32 oneMinusCos = 1.0f - cosAngle;

            f32 x = normalizedAxis.x;
            f32 y = normalizedAxis.y;
            f32 z = normalizedAxis.z;

            Mat3 result;
            result.Set(0, 0, cosAngle + x * x * oneMinusCos);
            result.Set(1, 0, x * y * oneMinusCos - z * sinAngle);
            result.Set(2, 0, x * z * oneMinusCos + y * sinAngle);

            result.Set(0, 1, y * x * oneMinusCos + z * sinAngle);
            result.Set(1, 1, cosAngle + y * y * oneMinusCos);
            result.Set(2, 1, y * z * oneMinusCos - x * sinAngle);

            result.Set(0, 2, z * x * oneMinusCos - y * sinAngle);
            result.Set(1, 2, z * y * oneMinusCos + x * sinAngle);
            result.Set(2, 2, cosAngle + z * z * oneMinusCos);

            return result;
        }

        Mat3 Mat3::CreateRotation(const Quat &quat)
        {
            Quat q = quat.Normalized();

            f32 xx = q.x * q.x;
            f32 yy = q.y * q.y;
            f32 zz = q.z * q.z;
            f32 xy = q.x * q.y;
            f32 xz = q.x * q.z;
            f32 yz = q.y * q.z;
            f32 wx = q.w * q.x;
            f32 wy = q.w * q.y;
            f32 wz = q.w * q.z;

            Mat3 result;
            result.Set(0, 0, 1.0f - 2.0f * (yy + zz));
            result.Set(1, 0, 2.0f * (xy - wz));
            result.Set(2, 0, 2.0f * (xz + wy));

            result.Set(0, 1, 2.0f * (xy + wz));
            result.Set(1, 1, 1.0f - 2.0f * (xx + zz));
            result.Set(2, 1, 2.0f * (yz - wx));

            result.Set(0, 2, 2.0f * (xz - wy));
            result.Set(1, 2, 2.0f * (yz + wx));
            result.Set(2, 2, 1.0f - 2.0f * (xx + yy));

            return result;
        }

        Mat3 Mat3::CreateNormalMatrix(const Mat4 &modelMatrix)
        {
            // Normal matrix is the transpose of the inverse of the upper-left 3x3 of the model matrix
            Mat3 upperLeft(modelMatrix);
            return upperLeft.Inverse().Transpose();
        }

        Mat4 Mat3::ToMat4() const
        {
            return Mat4(
                Get(0, 0), Get(1, 0), Get(2, 0), 0.0f,
                Get(0, 1), Get(1, 1), Get(2, 1), 0.0f,
                Get(0, 2), Get(1, 2), Get(2, 2), 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
        }

        // Non-member operators
        Mat3 operator*(f32 scalar, const Mat3 &matrix)
        {
            return matrix * scalar;
        }

    } // namespace Math
} // namespace Pyramid
