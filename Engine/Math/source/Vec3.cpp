#include <Pyramid/Math/Vec3.hpp>
#include <sstream>
#include <iomanip>

namespace Pyramid {
namespace Math {

// Static member definitions
const Vec3 Vec3::Zero(0.0f, 0.0f, 0.0f);
const Vec3 Vec3::Right(1.0f, 0.0f, 0.0f);
const Vec3 Vec3::Up(0.0f, 1.0f, 0.0f);
const Vec3 Vec3::Forward(0.0f, 0.0f, 1.0f);
const Vec3 Vec3::Left(-1.0f, 0.0f, 0.0f);
const Vec3 Vec3::Down(0.0f, -1.0f, 0.0f);
const Vec3 Vec3::Back(0.0f, 0.0f, -1.0f);
const Vec3 Vec3::One(1.0f, 1.0f, 1.0f);

// Constructors
Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f)
{
}

Vec3::Vec3(f32 value) : x(value), y(value), z(value)
{
}

Vec3::Vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z)
{
}

Vec3::Vec3(const Vec2& vec2, f32 z) : x(vec2.x), y(vec2.y), z(z)
{
}

// Conversion operators
Vec2 Vec3::ToVec2() const
{
    return Vec2(x, y);
}

// Arithmetic operators
Vec3 Vec3::operator+(const Vec3& other) const
{
    return Vec3(x + other.x, y + other.y, z + other.z);
}

Vec3 Vec3::operator-(const Vec3& other) const
{
    return Vec3(x - other.x, y - other.y, z - other.z);
}

Vec3 Vec3::operator*(f32 scalar) const
{
    return Vec3(x * scalar, y * scalar, z * scalar);
}

Vec3 Vec3::operator*(const Vec3& other) const
{
    return Vec3(x * other.x, y * other.y, z * other.z);
}

Vec3 Vec3::operator/(f32 scalar) const
{
    if (IsZero(scalar))
    {
        return Vec3::Zero;
    }
    f32 invScalar = 1.0f / scalar;
    return Vec3(x * invScalar, y * invScalar, z * invScalar);
}

Vec3 Vec3::operator/(const Vec3& other) const
{
    return Vec3(
        IsZero(other.x) ? 0.0f : x / other.x,
        IsZero(other.y) ? 0.0f : y / other.y,
        IsZero(other.z) ? 0.0f : z / other.z
    );
}

Vec3 Vec3::operator-() const
{
    return Vec3(-x, -y, -z);
}

// Compound assignment operators
Vec3& Vec3::operator+=(const Vec3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vec3& Vec3::operator-=(const Vec3& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vec3& Vec3::operator*=(f32 scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vec3& Vec3::operator*=(const Vec3& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}

Vec3& Vec3::operator/=(f32 scalar)
{
    if (!IsZero(scalar))
    {
        f32 invScalar = 1.0f / scalar;
        x *= invScalar;
        y *= invScalar;
        z *= invScalar;
    }
    else
    {
        x = y = z = 0.0f;
    }
    return *this;
}

Vec3& Vec3::operator/=(const Vec3& other)
{
    x = IsZero(other.x) ? 0.0f : x / other.x;
    y = IsZero(other.y) ? 0.0f : y / other.y;
    z = IsZero(other.z) ? 0.0f : z / other.z;
    return *this;
}

// Comparison operators
bool Vec3::operator==(const Vec3& other) const
{
    return IsEqual(x, other.x) && IsEqual(y, other.y) && IsEqual(z, other.z);
}

bool Vec3::operator!=(const Vec3& other) const
{
    return !(*this == other);
}

// Array access operators
f32 Vec3::operator[](i32 index) const
{
    return (&x)[index];
}

f32& Vec3::operator[](i32 index)
{
    return (&x)[index];
}

// Vector operations
f32 Vec3::Length() const
{
    return SafeSqrt(x * x + y * y + z * z);
}

f32 Vec3::LengthSquared() const
{
    return x * x + y * y + z * z;
}

Vec3& Vec3::Normalize()
{
    f32 length = Length();
    if (!IsZero(length))
    {
        f32 invLength = 1.0f / length;
        x *= invLength;
        y *= invLength;
        z *= invLength;
    }
    return *this;
}

Vec3 Vec3::Normalized() const
{
    Vec3 result(*this);
    result.Normalize();
    return result;
}

bool Vec3::IsNormalized() const
{
    return IsEqual(LengthSquared(), 1.0f);
}

bool Vec3::IsZero() const
{
    return Math::IsZero(x) && Math::IsZero(y) && Math::IsZero(z);
}

f32 Vec3::Dot(const Vec3& other) const
{
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::Cross(const Vec3& other) const
{
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

f32 Vec3::Distance(const Vec3& other) const
{
    return (*this - other).Length();
}

f32 Vec3::DistanceSquared(const Vec3& other) const
{
    return (*this - other).LengthSquared();
}

f32 Vec3::AngleBetween(const Vec3& other) const
{
    f32 dot = Dot(other);
    f32 lengths = Length() * other.Length();
    if (IsZero(lengths))
    {
        return 0.0f;
    }
    return Acos(Clamp(dot / lengths, -1.0f, 1.0f));
}

Vec3 Vec3::Lerp(const Vec3& other, f32 t) const
{
    return Vec3(
        Math::Lerp(x, other.x, t),
        Math::Lerp(y, other.y, t),
        Math::Lerp(z, other.z, t)
    );
}

Vec3 Vec3::Reflect(const Vec3& normal) const
{
    return *this - 2.0f * Dot(normal) * normal;
}

Vec3 Vec3::Project(const Vec3& other) const
{
    f32 otherLengthSq = other.LengthSquared();
    if (IsZero(otherLengthSq))
    {
        return Vec3::Zero;
    }
    return other * (Dot(other) / otherLengthSq);
}

Vec3 Vec3::Reject(const Vec3& other) const
{
    return *this - Project(other);
}

std::string Vec3::ToString() const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "Vec3(" << x << ", " << y << ", " << z << ")";
    return oss.str();
}

// Static utility functions
Vec3 Vec3::FromSpherical(f32 radius, f32 theta, f32 phi)
{
    f32 sinPhi = Sin(phi);
    return Vec3(
        radius * sinPhi * Cos(theta),
        radius * Cos(phi),
        radius * sinPhi * Sin(theta)
    );
}

Vec3 Vec3::Slerp(const Vec3& a, const Vec3& b, f32 t)
{
    f32 dot = a.Dot(b);
    dot = Clamp(dot, -1.0f, 1.0f);
    
    f32 theta = Acos(dot) * t;
    Vec3 relativeVec = (b - a * dot).Normalized();
    
    return a * Cos(theta) + relativeVec * Sin(theta);
}

// Non-member operators
Vec3 operator*(f32 scalar, const Vec3& vector)
{
    return vector * scalar;
}

} // namespace Math
} // namespace Pyramid
