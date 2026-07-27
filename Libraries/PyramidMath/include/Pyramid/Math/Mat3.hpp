#pragma once

#include <Pyramid/Math/MathCommon.hpp>
#include <Pyramid/Math/Vec2.hpp>
#include <Pyramid/Math/Vec3.hpp>
#include <string>

namespace Pyramid {
namespace Math {

// Forward declarations
class Mat4;
class Quat;

/**
 * @brief 3x3 Matrix class for 2D transformations and 3D rotations
 * 
 * Represents a 3x3 matrix stored in column-major order.
 * Used for 2D transformations, 3D rotation matrices, and normal transformations.
 */
class Mat3
{
public:
    // Matrix data stored in column-major order
    // m[column][row] or m[column * 3 + row]
    f32 m[9];

    // Constructors
    /**
     * @brief Default constructor - initializes to identity matrix
     */
    Mat3();

    /**
     * @brief Constructor with single value for diagonal elements
     * @param diagonal Value for diagonal elements (others are zero)
     */
    explicit Mat3(f32 diagonal);

    /**
     * @brief Constructor with all 9 values
     * @param m00-m22 Matrix elements in row-major order for readability
     */
    Mat3(f32 m00, f32 m01, f32 m02,
         f32 m10, f32 m11, f32 m12,
         f32 m20, f32 m21, f32 m22);

    /**
     * @brief Constructor from array of 9 floats (column-major order)
     * @param data Array of 9 floats
     */
    explicit Mat3(const f32* data);

    /**
     * @brief Constructor from Mat4 (extracts upper-left 3x3)
     * @param mat4 4x4 matrix to extract from
     */
    explicit Mat3(const Mat4& mat4);

    /**
     * @brief Constructor from quaternion (rotation matrix)
     * @param quat Quaternion to convert
     */
    explicit Mat3(const Quat& quat);

    /**
     * @brief Copy constructor
     */
    Mat3(const Mat3& other) = default;

    /**
     * @brief Assignment operator
     */
    Mat3& operator=(const Mat3& other) = default;

    // Element access
    /**
     * @brief Get element at column and row
     * @param col Column index (0-2)
     * @param row Row index (0-2)
     * @return Element value
     */
    f32 Get(i32 col, i32 row) const;

    /**
     * @brief Set element at column and row
     * @param col Column index (0-2)
     * @param row Row index (0-2)
     * @param value Value to set
     */
    void Set(i32 col, i32 row, f32 value);

    /**
     * @brief Array access operator (const)
     * @param index Linear index (0-8)
     * @return Element value
     */
    f32 operator[](i32 index) const;

    /**
     * @brief Array access operator (non-const)
     * @param index Linear index (0-8)
     * @return Reference to element
     */
    f32& operator[](i32 index);

    // Column and row access
    /**
     * @brief Get column as Vec3
     * @param col Column index (0-2)
     * @return Column vector
     */
    Vec3 GetColumn(i32 col) const;

    /**
     * @brief Set column from Vec3
     * @param col Column index (0-2)
     * @param column Column vector
     */
    void SetColumn(i32 col, const Vec3& column);

    /**
     * @brief Get row as Vec3
     * @param row Row index (0-2)
     * @return Row vector
     */
    Vec3 GetRow(i32 row) const;

    /**
     * @brief Set row from Vec3
     * @param row Row index (0-2)
     * @param rowVec Row vector
     */
    void SetRow(i32 row, const Vec3& rowVec);

    // Arithmetic operators
    Mat3 operator+(const Mat3& other) const;
    Mat3 operator-(const Mat3& other) const;
    Mat3 operator*(const Mat3& other) const;
    Mat3 operator*(f32 scalar) const;
    Vec3 operator*(const Vec3& vec) const;
    Mat3 operator-() const;

    // Compound assignment operators
    Mat3& operator+=(const Mat3& other);
    Mat3& operator-=(const Mat3& other);
    Mat3& operator*=(const Mat3& other);
    Mat3& operator*=(f32 scalar);

    // Comparison operators
    bool operator==(const Mat3& other) const;
    bool operator!=(const Mat3& other) const;

    // Matrix operations
    /**
     * @brief Calculate the transpose of the matrix
     */
    Mat3 Transpose() const;

    /**
     * @brief Transpose this matrix in place
     */
    Mat3& TransposeInPlace();

    /**
     * @brief Calculate the determinant of the matrix
     */
    f32 Determinant() const;

    /**
     * @brief Calculate the inverse of the matrix
     */
    Mat3 Inverse() const;

    /**
     * @brief Invert this matrix in place
     */
    Mat3& InvertInPlace();

    /**
     * @brief Check if the matrix is invertible
     */
    bool IsInvertible() const;

    /**
     * @brief Check if the matrix is identity
     */
    bool IsIdentity() const;

    /**
     * @brief Set this matrix to identity
     */
    Mat3& SetIdentity();

    /**
     * @brief Set this matrix to zero
     */
    Mat3& SetZero();

    /**
     * @brief Get pointer to matrix data
     */
    const f32* Data() const;

    /**
     * @brief Get pointer to matrix data
     */
    f32* Data();

    /**
     * @brief Convert to Mat4 (with identity w column/row)
     */
    Mat4 ToMat4() const;

    /**
     * @brief Convert matrix to string representation
     */
    std::string ToString() const;

    // Static utility functions
    static const Mat3 Identity;
    static const Mat3 Zero;

    /**
     * @brief Create identity matrix
     */
    static Mat3 CreateIdentity();

    /**
     * @brief Create zero matrix
     */
    static Mat3 CreateZero();

    /**
     * @brief Create 2D translation matrix
     * @param translation 2D translation vector
     */
    static Mat3 CreateTranslation(const Vec2& translation);

    /**
     * @brief Create 2D translation matrix
     * @param x X translation
     * @param y Y translation
     */
    static Mat3 CreateTranslation(f32 x, f32 y);

    /**
     * @brief Create 2D scale matrix
     * @param scale 2D scale vector
     */
    static Mat3 CreateScale(const Vec2& scale);

    /**
     * @brief Create uniform 2D scale matrix
     * @param scale Uniform scale factor
     */
    static Mat3 CreateScale(f32 scale);

    /**
     * @brief Create 2D scale matrix
     * @param x X scale
     * @param y Y scale
     */
    static Mat3 CreateScale(f32 x, f32 y);

    /**
     * @brief Create 2D rotation matrix
     * @param angle Rotation angle in radians
     */
    static Mat3 CreateRotation(f32 angle);

    /**
     * @brief Create 3D rotation matrix around X axis
     * @param angle Rotation angle in radians
     */
    static Mat3 CreateRotationX(f32 angle);

    /**
     * @brief Create 3D rotation matrix around Y axis
     * @param angle Rotation angle in radians
     */
    static Mat3 CreateRotationY(f32 angle);

    /**
     * @brief Create 3D rotation matrix around Z axis
     * @param angle Rotation angle in radians
     */
    static Mat3 CreateRotationZ(f32 angle);

    /**
     * @brief Create 3D rotation matrix around arbitrary axis
     * @param axis Rotation axis (should be normalized)
     * @param angle Rotation angle in radians
     */
    static Mat3 CreateRotation(const Vec3& axis, f32 angle);

    /**
     * @brief Create rotation matrix from quaternion
     * @param quat Quaternion representing rotation
     */
    static Mat3 CreateRotation(const Quat& quat);

    /**
     * @brief Create normal matrix from model matrix
     * @param modelMatrix Model transformation matrix
     */
    static Mat3 CreateNormalMatrix(const Mat4& modelMatrix);
};

// Non-member operators
Mat3 operator*(f32 scalar, const Mat3& matrix);

} // namespace Math
} // namespace Pyramid
