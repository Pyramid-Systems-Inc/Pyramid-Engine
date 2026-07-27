#pragma once

#include <Pyramid/Math/MathCommon.hpp>
#include <Pyramid/Math/Vec3.hpp>
#include <string>

namespace Pyramid {
namespace Math {

// Forward declarations
class Mat3;
class Mat4;

/**
 * @brief Quaternion class for 3D rotations
 * 
 * Represents a quaternion with x, y, z, w components for efficient 3D rotations.
 * Quaternions avoid gimbal lock and provide smooth interpolation between rotations.
 * 
 * Mathematical representation: q = w + xi + yj + zk
 * Where w is the scalar part and (x, y, z) is the vector part.
 */
class Quat
{
public:
    // Components: w is scalar, (x, y, z) is vector part
    f32 x, y, z, w;

    // Constructors
    /**
     * @brief Default constructor - creates identity quaternion (0, 0, 0, 1)
     */
    Quat();

    /**
     * @brief Constructor with explicit components
     * @param x X component (i coefficient)
     * @param y Y component (j coefficient)
     * @param z Z component (k coefficient)
     * @param w W component (scalar part)
     */
    Quat(f32 x, f32 y, f32 z, f32 w);

    /**
     * @brief Constructor from vector part and scalar
     * @param vector Vector part (x, y, z)
     * @param scalar Scalar part (w)
     */
    Quat(const Vec3& vector, f32 scalar);

    /**
     * @brief Copy constructor
     */
    Quat(const Quat& other) = default;

    /**
     * @brief Assignment operator
     */
    Quat& operator=(const Quat& other) = default;

    // Static construction methods
    /**
     * @brief Create quaternion from axis-angle representation
     * @param axis Rotation axis (should be normalized)
     * @param angle Rotation angle in radians
     * @return Quaternion representing the rotation
     */
    static Quat FromAxisAngle(const Vec3& axis, f32 angle);

    /**
     * @brief Create quaternion from Euler angles (ZYX order)
     * @param pitch Rotation around X axis (radians)
     * @param yaw Rotation around Y axis (radians)
     * @param roll Rotation around Z axis (radians)
     * @return Quaternion representing the rotation
     */
    static Quat FromEuler(f32 pitch, f32 yaw, f32 roll);

    /**
     * @brief Create quaternion from Euler angles vector
     * @param euler Euler angles (x=pitch, y=yaw, z=roll) in radians
     * @return Quaternion representing the rotation
     */
    static Quat FromEuler(const Vec3& euler);

    /**
     * @brief Create quaternion from rotation matrix
     * @param matrix Rotation matrix (3x3 or 4x4)
     * @return Quaternion representing the rotation
     */
    static Quat FromMatrix(const Mat3& matrix);
    static Quat FromMatrix(const Mat4& matrix);

    /**
     * @brief Create quaternion that rotates from one vector to another
     * @param from Source vector (should be normalized)
     * @param to Target vector (should be normalized)
     * @return Quaternion representing the rotation
     */
    static Quat FromToRotation(const Vec3& from, const Vec3& to);

    /**
     * @brief Create look rotation quaternion
     * @param forward Forward direction (should be normalized)
     * @param up Up direction (should be normalized)
     * @return Quaternion representing the look rotation
     */
    static Quat LookRotation(const Vec3& forward, const Vec3& up = Vec3::Up);

    // Arithmetic operators
    /**
     * @brief Quaternion addition
     */
    Quat operator+(const Quat& other) const;

    /**
     * @brief Quaternion subtraction
     */
    Quat operator-(const Quat& other) const;

    /**
     * @brief Quaternion multiplication (composition of rotations)
     */
    Quat operator*(const Quat& other) const;

    /**
     * @brief Scalar multiplication
     */
    Quat operator*(f32 scalar) const;

    /**
     * @brief Scalar division
     */
    Quat operator/(f32 scalar) const;

    /**
     * @brief Unary negation
     */
    Quat operator-() const;

    // Compound assignment operators
    Quat& operator+=(const Quat& other);
    Quat& operator-=(const Quat& other);
    Quat& operator*=(const Quat& other);
    Quat& operator*=(f32 scalar);
    Quat& operator/=(f32 scalar);

    // Comparison operators
    bool operator==(const Quat& other) const;
    bool operator!=(const Quat& other) const;

    // Array access operators
    f32 operator[](i32 index) const;
    f32& operator[](i32 index);

    // Quaternion operations
    /**
     * @brief Calculate the magnitude (length) of the quaternion
     */
    f32 Length() const;

    /**
     * @brief Calculate the squared magnitude (faster than Length())
     */
    f32 LengthSquared() const;

    /**
     * @brief Normalize the quaternion (make it unit length)
     */
    Quat& Normalize();

    /**
     * @brief Get a normalized copy of the quaternion
     */
    Quat Normalized() const;

    /**
     * @brief Check if the quaternion is normalized
     */
    bool IsNormalized() const;

    /**
     * @brief Check if the quaternion is identity
     */
    bool IsIdentity() const;

    /**
     * @brief Calculate the conjugate of the quaternion
     */
    Quat Conjugate() const;

    /**
     * @brief Calculate the inverse of the quaternion
     */
    Quat Inverse() const;

    /**
     * @brief Calculate the dot product with another quaternion
     */
    f32 Dot(const Quat& other) const;

    /**
     * @brief Get the angle of rotation represented by this quaternion
     */
    f32 GetAngle() const;

    /**
     * @brief Get the axis of rotation represented by this quaternion
     */
    Vec3 GetAxis() const;

    /**
     * @brief Convert to Euler angles (ZYX order)
     * @return Euler angles (x=pitch, y=yaw, z=roll) in radians
     */
    Vec3 ToEuler() const;

    /**
     * @brief Convert to rotation matrix (3x3)
     */
    Mat3 ToMatrix3() const;

    /**
     * @brief Convert to rotation matrix (4x4)
     */
    Mat4 ToMatrix4() const;

    /**
     * @brief Rotate a vector by this quaternion
     */
    Vec3 RotateVector(const Vec3& vector) const;

    /**
     * @brief Linear interpolation between quaternions (not recommended for rotations)
     */
    Quat Lerp(const Quat& other, f32 t) const;

    /**
     * @brief Spherical linear interpolation (SLERP) - smooth rotation interpolation
     */
    Quat Slerp(const Quat& other, f32 t) const;

    /**
     * @brief Normalized linear interpolation (NLERP) - faster approximation of SLERP
     */
    Quat Nlerp(const Quat& other, f32 t) const;

    /**
     * @brief Convert to string representation
     */
    std::string ToString() const;

    // Static constants
    /**
     * @brief Identity quaternion (no rotation)
     */
    static const Quat Identity;

    // Static utility functions
    /**
     * @brief Spherical linear interpolation between two quaternions
     */
    static Quat Slerp(const Quat& a, const Quat& b, f32 t);

    /**
     * @brief Normalized linear interpolation between two quaternions
     */
    static Quat Nlerp(const Quat& a, const Quat& b, f32 t);

    /**
     * @brief Calculate angle between two quaternions
     */
    static f32 Angle(const Quat& a, const Quat& b);

    /**
     * @brief Create quaternion representing rotation around X axis
     */
    static Quat AngleX(f32 angle);

    /**
     * @brief Create quaternion representing rotation around Y axis
     */
    static Quat AngleY(f32 angle);

    /**
     * @brief Create quaternion representing rotation around Z axis
     */
    static Quat AngleZ(f32 angle);
};

// Non-member operators
/**
 * @brief Scalar multiplication (scalar * quaternion)
 */
Quat operator*(f32 scalar, const Quat& quat);

} // namespace Math
} // namespace Pyramid
