#include <Pyramid/Math/Vec2.hpp>
#include <sstream>
#include <iomanip>

namespace Pyramid
{
    namespace Math
    {

        // Static member definitions
        const Vec2 Vec2::Zero(0.0f, 0.0f);
        const Vec2 Vec2::Right(1.0f, 0.0f);
        const Vec2 Vec2::Up(0.0f, 1.0f);
        const Vec2 Vec2::Left(-1.0f, 0.0f);
        const Vec2 Vec2::Down(0.0f, -1.0f);
        const Vec2 Vec2::One(1.0f, 1.0f);

        // Constructors
        Vec2::Vec2() : x(0.0f), y(0.0f)
        {
        }

        Vec2::Vec2(f32 value) : x(value), y(value)
        {
        }

        Vec2::Vec2(f32 x, f32 y) : x(x), y(y)
        {
        }

        // Arithmetic operators
        Vec2 Vec2::operator+(const Vec2 &other) const
        {
            return Vec2(x + other.x, y + other.y);
        }

        Vec2 Vec2::operator-(const Vec2 &other) const
        {
            return Vec2(x - other.x, y - other.y);
        }

        Vec2 Vec2::operator*(f32 scalar) const
        {
            return Vec2(x * scalar, y * scalar);
        }

        Vec2 Vec2::operator*(const Vec2 &other) const
        {
            return Vec2(x * other.x, y * other.y);
        }

        Vec2 Vec2::operator/(f32 scalar) const
        {
            if (Math::IsZero(scalar))
            {
                return Vec2::Zero;
            }
            f32 invScalar = 1.0f / scalar;
            return Vec2(x * invScalar, y * invScalar);
        }

        Vec2 Vec2::operator/(const Vec2 &other) const
        {
            return Vec2(
                Math::IsZero(other.x) ? 0.0f : x / other.x,
                Math::IsZero(other.y) ? 0.0f : y / other.y);
        }

        Vec2 Vec2::operator-() const
        {
            return Vec2(-x, -y);
        }

        // Compound assignment operators
        Vec2 &Vec2::operator+=(const Vec2 &other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vec2 &Vec2::operator-=(const Vec2 &other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vec2 &Vec2::operator*=(f32 scalar)
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        Vec2 &Vec2::operator*=(const Vec2 &other)
        {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        Vec2 &Vec2::operator/=(f32 scalar)
        {
            if (!Math::IsZero(scalar))
            {
                f32 invScalar = 1.0f / scalar;
                x *= invScalar;
                y *= invScalar;
            }
            else
            {
                x = y = 0.0f;
            }
            return *this;
        }

        Vec2 &Vec2::operator/=(const Vec2 &other)
        {
            x = IsZero(other.x) ? 0.0f : x / other.x;
            y = IsZero(other.y) ? 0.0f : y / other.y;
            return *this;
        }

        // Comparison operators
        bool Vec2::operator==(const Vec2 &other) const
        {
            return IsEqual(x, other.x) && IsEqual(y, other.y);
        }

        bool Vec2::operator!=(const Vec2 &other) const
        {
            return !(*this == other);
        }

        // Array access operators
        f32 Vec2::operator[](i32 index) const
        {
            return (&x)[index];
        }

        f32 &Vec2::operator[](i32 index)
        {
            return (&x)[index];
        }

        // Vector operations
        f32 Vec2::Length() const
        {
            return SafeSqrt(x * x + y * y);
        }

        f32 Vec2::LengthSquared() const
        {
            return x * x + y * y;
        }

        Vec2 &Vec2::Normalize()
        {
            f32 length = Length();
            if (!IsZero(length))
            {
                f32 invLength = 1.0f / length;
                x *= invLength;
                y *= invLength;
            }
            return *this;
        }

        Vec2 Vec2::Normalized() const
        {
            Vec2 result(*this);
            result.Normalize();
            return result;
        }

        bool Vec2::IsNormalized() const
        {
            return IsEqual(LengthSquared(), 1.0f);
        }

        bool Vec2::IsZero() const
        {
            return Math::IsZero(x) && Math::IsZero(y);
        }

        f32 Vec2::Dot(const Vec2 &other) const
        {
            return x * other.x + y * other.y;
        }

        f32 Vec2::Distance(const Vec2 &other) const
        {
            return (*this - other).Length();
        }

        f32 Vec2::DistanceSquared(const Vec2 &other) const
        {
            return (*this - other).LengthSquared();
        }

        f32 Vec2::Angle() const
        {
            return Atan2(y, x);
        }

        f32 Vec2::AngleBetween(const Vec2 &other) const
        {
            f32 dot = Dot(other);
            f32 lengths = Length() * other.Length();
            if (IsZero(lengths))
            {
                return 0.0f;
            }
            return Acos(Clamp(dot / lengths, -1.0f, 1.0f));
        }

        Vec2 Vec2::Lerp(const Vec2 &other, f32 t) const
        {
            return Vec2(
                Math::Lerp(x, other.x, t),
                Math::Lerp(y, other.y, t));
        }

        Vec2 Vec2::Reflect(const Vec2 &normal) const
        {
            return *this - 2.0f * Dot(normal) * normal;
        }

        Vec2 Vec2::Perpendicular() const
        {
            return Vec2(-y, x);
        }

        std::string Vec2::ToString() const
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3);
            oss << "Vec2(" << x << ", " << y << ")";
            return oss.str();
        }

        // Non-member operators
        Vec2 operator*(f32 scalar, const Vec2 &vector)
        {
            return vector * scalar;
        }

    } // namespace Math
} // namespace Pyramid
