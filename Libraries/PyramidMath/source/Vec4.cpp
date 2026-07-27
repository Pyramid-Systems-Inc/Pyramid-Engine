#include <Pyramid/Math/Vec4.hpp>
#include <sstream>
#include <iomanip>

namespace Pyramid
{
    namespace Math
    {

        // Static member definitions
        const Vec4 Vec4::Zero(0.0f, 0.0f, 0.0f, 0.0f);
        const Vec4 Vec4::One(1.0f, 1.0f, 1.0f, 1.0f);
        const Vec4 Vec4::UnitX(1.0f, 0.0f, 0.0f, 0.0f);
        const Vec4 Vec4::UnitY(0.0f, 1.0f, 0.0f, 0.0f);
        const Vec4 Vec4::UnitZ(0.0f, 0.0f, 1.0f, 0.0f);
        const Vec4 Vec4::UnitW(0.0f, 0.0f, 0.0f, 1.0f);

        // Constructors
        Vec4::Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
        {
        }

        Vec4::Vec4(f32 value) : x(value), y(value), z(value), w(value)
        {
        }

        Vec4::Vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w)
        {
        }

        Vec4::Vec4(const Vec2 &vec2, f32 z, f32 w) : x(vec2.x), y(vec2.y), z(z), w(w)
        {
        }

        Vec4::Vec4(const Vec3 &vec3, f32 w) : x(vec3.x), y(vec3.y), z(vec3.z), w(w)
        {
        }

        // Conversion operators
        Vec2 Vec4::ToVec2() const
        {
            return Vec2(x, y);
        }

        Vec3 Vec4::ToVec3() const
        {
            return Vec3(x, y, z);
        }

        // Arithmetic operators
        Vec4 Vec4::operator+(const Vec4 &other) const
        {
            return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
        }

        Vec4 Vec4::operator-(const Vec4 &other) const
        {
            return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
        }

        Vec4 Vec4::operator*(f32 scalar) const
        {
            return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
        }

        Vec4 Vec4::operator*(const Vec4 &other) const
        {
            return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
        }

        Vec4 Vec4::operator/(f32 scalar) const
        {
            if (Math::IsZero(scalar))
            {
                return Vec4::Zero;
            }
            f32 invScalar = 1.0f / scalar;
            return Vec4(x * invScalar, y * invScalar, z * invScalar, w * invScalar);
        }

        Vec4 Vec4::operator/(const Vec4 &other) const
        {
            return Vec4(
                Math::IsZero(other.x) ? 0.0f : x / other.x,
                Math::IsZero(other.y) ? 0.0f : y / other.y,
                Math::IsZero(other.z) ? 0.0f : z / other.z,
                Math::IsZero(other.w) ? 0.0f : w / other.w);
        }

        Vec4 Vec4::operator-() const
        {
            return Vec4(-x, -y, -z, -w);
        }

        // Compound assignment operators
        Vec4 &Vec4::operator+=(const Vec4 &other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        Vec4 &Vec4::operator-=(const Vec4 &other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        Vec4 &Vec4::operator*=(f32 scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
            return *this;
        }

        Vec4 &Vec4::operator*=(const Vec4 &other)
        {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            w *= other.w;
            return *this;
        }

        Vec4 &Vec4::operator/=(f32 scalar)
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
                x = y = z = w = 0.0f;
            }
            return *this;
        }

        Vec4 &Vec4::operator/=(const Vec4 &other)
        {
            x = Math::IsZero(other.x) ? 0.0f : x / other.x;
            y = Math::IsZero(other.y) ? 0.0f : y / other.y;
            z = Math::IsZero(other.z) ? 0.0f : z / other.z;
            w = Math::IsZero(other.w) ? 0.0f : w / other.w;
            return *this;
        }

        // Comparison operators
        bool Vec4::operator==(const Vec4 &other) const
        {
            return IsEqual(x, other.x) && IsEqual(y, other.y) &&
                   IsEqual(z, other.z) && IsEqual(w, other.w);
        }

        bool Vec4::operator!=(const Vec4 &other) const
        {
            return !(*this == other);
        }

        // Array access operators
        f32 Vec4::operator[](i32 index) const
        {
            return (&x)[index];
        }

        f32 &Vec4::operator[](i32 index)
        {
            return (&x)[index];
        }

        // Vector operations
        f32 Vec4::Length() const
        {
            return SafeSqrt(x * x + y * y + z * z + w * w);
        }

        f32 Vec4::LengthSquared() const
        {
            return x * x + y * y + z * z + w * w;
        }

        Vec4 &Vec4::Normalize()
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

        Vec4 Vec4::Normalized() const
        {
            Vec4 result(*this);
            result.Normalize();
            return result;
        }

        bool Vec4::IsNormalized() const
        {
            return IsEqual(LengthSquared(), 1.0f);
        }

        bool Vec4::IsZero() const
        {
            return Math::IsZero(x) && Math::IsZero(y) && Math::IsZero(z) && Math::IsZero(w);
        }

        f32 Vec4::Dot(const Vec4 &other) const
        {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        f32 Vec4::Distance(const Vec4 &other) const
        {
            return (*this - other).Length();
        }

        f32 Vec4::DistanceSquared(const Vec4 &other) const
        {
            return (*this - other).LengthSquared();
        }

        f32 Vec4::AngleBetween(const Vec4 &other) const
        {
            f32 dot = Dot(other);
            f32 lengths = Length() * other.Length();
            if (Math::IsZero(lengths))
            {
                return 0.0f;
            }
            return Acos(Clamp(dot / lengths, -1.0f, 1.0f));
        }

        Vec4 Vec4::Lerp(const Vec4 &other, f32 t) const
        {
            return Vec4(
                Math::Lerp(x, other.x, t),
                Math::Lerp(y, other.y, t),
                Math::Lerp(z, other.z, t),
                Math::Lerp(w, other.w, t));
        }

        std::string Vec4::ToString() const
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "Vec4(" << x << ", " << y << ", " << z << ", " << w << ")";
            return oss.str();
        }

        // Non-member operators
        Vec4 operator*(f32 scalar, const Vec4 &vector)
        {
            return vector * scalar;
        }

    } // namespace Math
} // namespace Pyramid
