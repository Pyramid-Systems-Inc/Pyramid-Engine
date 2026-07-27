#include <Pyramid/Math/Quat.hpp>
#include <Pyramid/Math/Mat3.hpp>
#include <Pyramid/Math/Mat4.hpp>
#include <sstream>
#include <iomanip>

namespace Pyramid
{
    namespace Math
    {

        // Static member definitions
        const Quat Quat::Identity(0.0f, 0.0f, 0.0f, 1.0f);

        // Constructors
        Quat::Quat() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
        {
        }

        Quat::Quat(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w)
        {
        }

        Quat::Quat(const Vec3 &vector, f32 scalar) : x(vector.x), y(vector.y), z(vector.z), w(scalar)
        {
        }

        // Static construction methods
        Quat Quat::FromAxisAngle(const Vec3 &axis, f32 angle)
        {
            f32 halfAngle = angle * 0.5f;
            f32 sinHalf = Sin(halfAngle);
            f32 cosHalf = Cos(halfAngle);

            Vec3 normalizedAxis = axis.Normalized();

            return Quat(
                normalizedAxis.x * sinHalf,
                normalizedAxis.y * sinHalf,
                normalizedAxis.z * sinHalf,
                cosHalf);
        }

        Quat Quat::FromEuler(f32 pitch, f32 yaw, f32 roll)
        {
            // Convert to half angles
            f32 halfPitch = pitch * 0.5f;
            f32 halfYaw = yaw * 0.5f;
            f32 halfRoll = roll * 0.5f;

            f32 cosPitch = Cos(halfPitch);
            f32 sinPitch = Sin(halfPitch);
            f32 cosYaw = Cos(halfYaw);
            f32 sinYaw = Sin(halfYaw);
            f32 cosRoll = Cos(halfRoll);
            f32 sinRoll = Sin(halfRoll);

            // ZYX order (roll * yaw * pitch)
            return Quat(
                sinPitch * cosYaw * cosRoll - cosPitch * sinYaw * sinRoll,
                cosPitch * sinYaw * cosRoll + sinPitch * cosYaw * sinRoll,
                cosPitch * cosYaw * sinRoll - sinPitch * sinYaw * cosRoll,
                cosPitch * cosYaw * cosRoll + sinPitch * sinYaw * sinRoll);
        }

        Quat Quat::FromEuler(const Vec3 &euler)
        {
            return FromEuler(euler.x, euler.y, euler.z);
        }

        Quat Quat::FromMatrix(const Mat3 &matrix)
        {
            f32 trace = matrix.Get(0, 0) + matrix.Get(1, 1) + matrix.Get(2, 2);

            if (trace > 0.0f)
            {
                f32 s = SafeSqrt(trace + 1.0f) * 2.0f;
                return Quat(
                    (matrix.Get(2, 1) - matrix.Get(1, 2)) / s,
                    (matrix.Get(0, 2) - matrix.Get(2, 0)) / s,
                    (matrix.Get(1, 0) - matrix.Get(0, 1)) / s,
                    0.25f * s);
            }
            else if (matrix.Get(0, 0) > matrix.Get(1, 1) && matrix.Get(0, 0) > matrix.Get(2, 2))
            {
                f32 s = SafeSqrt(1.0f + matrix.Get(0, 0) - matrix.Get(1, 1) - matrix.Get(2, 2)) * 2.0f;
                return Quat(
                    0.25f * s,
                    (matrix.Get(0, 1) + matrix.Get(1, 0)) / s,
                    (matrix.Get(0, 2) + matrix.Get(2, 0)) / s,
                    (matrix.Get(2, 1) - matrix.Get(1, 2)) / s);
            }
            else if (matrix.Get(1, 1) > matrix.Get(2, 2))
            {
                f32 s = SafeSqrt(1.0f + matrix.Get(1, 1) - matrix.Get(0, 0) - matrix.Get(2, 2)) * 2.0f;
                return Quat(
                    (matrix.Get(0, 1) + matrix.Get(1, 0)) / s,
                    0.25f * s,
                    (matrix.Get(1, 2) + matrix.Get(2, 1)) / s,
                    (matrix.Get(0, 2) - matrix.Get(2, 0)) / s);
            }
            else
            {
                f32 s = SafeSqrt(1.0f + matrix.Get(2, 2) - matrix.Get(0, 0) - matrix.Get(1, 1)) * 2.0f;
                return Quat(
                    (matrix.Get(0, 2) + matrix.Get(2, 0)) / s,
                    (matrix.Get(1, 2) + matrix.Get(2, 1)) / s,
                    0.25f * s,
                    (matrix.Get(1, 0) - matrix.Get(0, 1)) / s);
            }
        }

        Quat Quat::FromMatrix(const Mat4 &matrix)
        {
            // Extract the upper-left 3x3 and convert
            Mat3 rotationMatrix(matrix);
            return FromMatrix(rotationMatrix);
        }

        Quat Quat::FromToRotation(const Vec3 &from, const Vec3 &to)
        {
            Vec3 fromNorm = from.Normalized();
            Vec3 toNorm = to.Normalized();

            f32 dot = fromNorm.Dot(toNorm);

            // Check if vectors are parallel
            if (dot >= 1.0f - EPSILON)
            {
                return Quat::Identity;
            }

            // Check if vectors are opposite
            if (dot <= -1.0f + EPSILON)
            {
                // Find a perpendicular axis
                Vec3 axis = fromNorm.Cross(Vec3::Right);
                if (axis.LengthSquared() < EPSILON)
                {
                    axis = fromNorm.Cross(Vec3::Up);
                }
                axis.Normalize();
                return FromAxisAngle(axis, PI);
            }

            Vec3 cross = fromNorm.Cross(toNorm);
            f32 w = 1.0f + dot;

            Quat result(cross.x, cross.y, cross.z, w);
            return result.Normalized();
        }

        Quat Quat::LookRotation(const Vec3 &forward, const Vec3 &up)
        {
            Vec3 forwardNorm = forward.Normalized();
            Vec3 upNorm = up.Normalized();

            Vec3 right = forwardNorm.Cross(upNorm).Normalized();
            upNorm = right.Cross(forwardNorm);

            // Create rotation matrix and convert to quaternion
            f32 trace = right.x + upNorm.y + (-forwardNorm.z);

            if (trace > 0.0f)
            {
                f32 s = SafeSqrt(trace + 1.0f) * 2.0f;
                return Quat(
                    (upNorm.z - (-forwardNorm.y)) / s,
                    ((-forwardNorm.x) - right.z) / s,
                    (right.y - upNorm.x) / s,
                    0.25f * s);
            }
            else if (right.x > upNorm.y && right.x > (-forwardNorm.z))
            {
                f32 s = SafeSqrt(1.0f + right.x - upNorm.y - (-forwardNorm.z)) * 2.0f;
                return Quat(
                    0.25f * s,
                    (right.y + upNorm.x) / s,
                    ((-forwardNorm.x) + right.z) / s,
                    (upNorm.z - (-forwardNorm.y)) / s);
            }
            else if (upNorm.y > (-forwardNorm.z))
            {
                f32 s = SafeSqrt(1.0f + upNorm.y - right.x - (-forwardNorm.z)) * 2.0f;
                return Quat(
                    (right.y + upNorm.x) / s,
                    0.25f * s,
                    (upNorm.z + (-forwardNorm.y)) / s,
                    ((-forwardNorm.x) - right.z) / s);
            }
            else
            {
                f32 s = SafeSqrt(1.0f + (-forwardNorm.z) - right.x - upNorm.y) * 2.0f;
                return Quat(
                    ((-forwardNorm.x) + right.z) / s,
                    (upNorm.z + (-forwardNorm.y)) / s,
                    0.25f * s,
                    (right.y - upNorm.x) / s);
            }
        }

        // Arithmetic operators
        Quat Quat::operator+(const Quat &other) const
        {
            return Quat(x + other.x, y + other.y, z + other.z, w + other.w);
        }

        Quat Quat::operator-(const Quat &other) const
        {
            return Quat(x - other.x, y - other.y, z - other.z, w - other.w);
        }

        Quat Quat::operator*(const Quat &other) const
        {
            return Quat(
                w * other.x + x * other.w + y * other.z - z * other.y,
                w * other.y - x * other.z + y * other.w + z * other.x,
                w * other.z + x * other.y - y * other.x + z * other.w,
                w * other.w - x * other.x - y * other.y - z * other.z);
        }

        Quat Quat::operator*(f32 scalar) const
        {
            return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
        }

        Quat Quat::operator/(f32 scalar) const
        {
            if (Math::IsZero(scalar))
            {
                return Quat::Identity;
            }
            f32 invScalar = 1.0f / scalar;
            return Quat(x * invScalar, y * invScalar, z * invScalar, w * invScalar);
        }

        Quat Quat::operator-() const
        {
            return Quat(-x, -y, -z, -w);
        }

        // Compound assignment operators
        Quat &Quat::operator+=(const Quat &other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        Quat &Quat::operator-=(const Quat &other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        Quat &Quat::operator*=(const Quat &other)
        {
            *this = *this * other;
            return *this;
        }

        Quat &Quat::operator*=(f32 scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
            return *this;
        }

        Quat &Quat::operator/=(f32 scalar)
        {
            if (!Math::IsZero(scalar))
            {
                f32 invScalar = 1.0f / scalar;
                x *= invScalar;
                y *= invScalar;
                z *= invScalar;
                w *= invScalar;
            }
            else
            {
                *this = Quat::Identity;
            }
            return *this;
        }

        // Comparison operators
        bool Quat::operator==(const Quat &other) const
        {
            return IsEqual(x, other.x) && IsEqual(y, other.y) &&
                   IsEqual(z, other.z) && IsEqual(w, other.w);
        }

        bool Quat::operator!=(const Quat &other) const
        {
            return !(*this == other);
        }

        // Array access operators
        f32 Quat::operator[](i32 index) const
        {
            return (&x)[index];
        }

        f32 &Quat::operator[](i32 index)
        {
            return (&x)[index];
        }

        // Quaternion operations
        f32 Quat::Length() const
        {
            return SafeSqrt(x * x + y * y + z * z + w * w);
        }

        f32 Quat::LengthSquared() const
        {
            return x * x + y * y + z * z + w * w;
        }

        Quat &Quat::Normalize()
        {
            f32 length = Length();
            if (!Math::IsZero(length))
            {
                f32 invLength = 1.0f / length;
                x *= invLength;
                y *= invLength;
                z *= invLength;
                w *= invLength;
            }
            return *this;
        }

        Quat Quat::Normalized() const
        {
            Quat result(*this);
            result.Normalize();
            return result;
        }

        bool Quat::IsNormalized() const
        {
            return IsEqual(LengthSquared(), 1.0f);
        }

        bool Quat::IsIdentity() const
        {
            return IsEqual(x, 0.0f) && IsEqual(y, 0.0f) && IsEqual(z, 0.0f) && IsEqual(w, 1.0f);
        }

        Quat Quat::Conjugate() const
        {
            return Quat(-x, -y, -z, w);
        }

        Quat Quat::Inverse() const
        {
            f32 lengthSq = LengthSquared();
            if (Math::IsZero(lengthSq))
            {
                return Quat::Identity;
            }

            Quat conjugate = Conjugate();
            return conjugate / lengthSq;
        }

        f32 Quat::Dot(const Quat &other) const
        {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        f32 Quat::GetAngle() const
        {
            return 2.0f * Acos(Clamp(Abs(w), 0.0f, 1.0f));
        }

        Vec3 Quat::GetAxis() const
        {
            f32 sinHalfAngle = SafeSqrt(1.0f - w * w);
            if (Math::IsZero(sinHalfAngle))
            {
                return Vec3::Right; // Default axis when no rotation
            }

            f32 invSinHalfAngle = 1.0f / sinHalfAngle;
            return Vec3(x * invSinHalfAngle, y * invSinHalfAngle, z * invSinHalfAngle);
        }

        Vec3 Quat::ToEuler() const
        {
            // Convert quaternion to Euler angles (ZYX order)
            f32 sinr_cosp = 2.0f * (w * x + y * z);
            f32 cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
            f32 roll = Atan2(sinr_cosp, cosr_cosp);

            f32 sinp = 2.0f * (w * y - z * x);
            f32 pitch;
            if (Abs(sinp) >= 1.0f)
            {
                pitch = (sinp >= 0.0f) ? HALF_PI : -HALF_PI; // Use 90 degrees if out of range
            }
            else
            {
                pitch = Asin(sinp);
            }

            f32 siny_cosp = 2.0f * (w * z + x * y);
            f32 cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
            f32 yaw = Atan2(siny_cosp, cosy_cosp);

            return Vec3(pitch, yaw, roll);
        }

        Mat3 Quat::ToMatrix3() const
        {
            return Mat3::CreateRotation(*this);
        }

        Mat4 Quat::ToMatrix4() const
        {
            return ToMatrix3().ToMat4();
        }

        Vec3 Quat::RotateVector(const Vec3 &vector) const
        {
            // v' = q * v * q^-1
            // Optimized version: v' = v + 2 * cross(q.xyz, cross(q.xyz, v) + q.w * v)
            Vec3 qvec(x, y, z);
            Vec3 cross1 = qvec.Cross(vector);
            Vec3 cross2 = qvec.Cross(cross1 + w * vector);
            return vector + 2.0f * cross2;
        }

        Quat Quat::Lerp(const Quat &other, f32 t) const
        {
            return (*this * (1.0f - t) + other * t).Normalized();
        }

        Quat Quat::Slerp(const Quat &other, f32 t) const
        {
            return Slerp(*this, other, t);
        }

        Quat Quat::Nlerp(const Quat &other, f32 t) const
        {
            return Nlerp(*this, other, t);
        }

        std::string Quat::ToString() const
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "Quat(" << x << ", " << y << ", " << z << ", " << w << ")";
            return oss.str();
        }

        // Static utility functions
        Quat Quat::Slerp(const Quat &a, const Quat &b, f32 t)
        {
            f32 dot = a.Dot(b);

            // If the dot product is negative, slerp won't take the shorter path.
            // Note that v1 and -v1 are equivalent when the represent rotations.
            Quat b_corrected = b;
            if (dot < 0.0f)
            {
                b_corrected = -b;
                dot = -dot;
            }

            // If the inputs are too close for comfort, linearly interpolate
            if (dot > 0.9995f)
            {
                return Nlerp(a, b_corrected, t);
            }

            f32 theta_0 = Acos(dot);
            f32 theta = theta_0 * t;
            f32 sin_theta = Sin(theta);
            f32 sin_theta_0 = Sin(theta_0);

            f32 s0 = Cos(theta) - dot * sin_theta / sin_theta_0;
            f32 s1 = sin_theta / sin_theta_0;

            return (a * s0) + (b_corrected * s1);
        }

        Quat Quat::Nlerp(const Quat &a, const Quat &b, f32 t)
        {
            f32 dot = a.Dot(b);

            // Choose the shorter path
            Quat b_corrected = (dot < 0.0f) ? -b : b;

            return (a * (1.0f - t) + b_corrected * t).Normalized();
        }

        f32 Quat::Angle(const Quat &a, const Quat &b)
        {
            f32 dot = Abs(a.Dot(b));
            return 2.0f * Acos(Clamp(dot, 0.0f, 1.0f));
        }

        Quat Quat::AngleX(f32 angle)
        {
            return FromAxisAngle(Vec3::Right, angle);
        }

        Quat Quat::AngleY(f32 angle)
        {
            return FromAxisAngle(Vec3::Up, angle);
        }

        Quat Quat::AngleZ(f32 angle)
        {
            return FromAxisAngle(Vec3::Forward, angle);
        }

        // Non-member operators
        Quat operator*(f32 scalar, const Quat &quat)
        {
            return quat * scalar;
        }

    } // namespace Math
} // namespace Pyramid
