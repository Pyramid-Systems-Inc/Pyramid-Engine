#pragma once

#include <Pyramid/Math/MathCommon.hpp>
#include <Pyramid/Math/Vec3.hpp>
#include <Pyramid/Math/Vec4.hpp>
#include <string>

namespace Pyramid
{
    namespace Math
    {

        /**
         * @brief 4x4 Matrix class for 3D transformations
         *
         * Represents a 4x4 matrix stored in column-major order (OpenGL style).
         * Used for 3D transformations, projections, and view matrices.
         */
        class Mat4
        {
        public:
            // Matrix data stored in column-major order
            // m[column][row] or m[column * 4 + row]
            f32 m[16];

            // Constructors
            /**
             * @brief Default constructor - initializes to identity matrix
             */
            Mat4();

            /**
             * @brief Constructor with single value for diagonal elements
             * @param diagonal Value for diagonal elements (others are zero)
             */
            explicit Mat4(f32 diagonal);

            /**
             * @brief Constructor with all 16 values
             * @param m00-m33 Matrix elements in row-major order for readability
             */
            Mat4(f32 m00, f32 m01, f32 m02, f32 m03,
                 f32 m10, f32 m11, f32 m12, f32 m13,
                 f32 m20, f32 m21, f32 m22, f32 m23,
                 f32 m30, f32 m31, f32 m32, f32 m33);

            /**
             * @brief Constructor from array of 16 floats (column-major order)
             * @param data Array of 16 floats
             */
            explicit Mat4(const f32 *data);

            /**
             * @brief Copy constructor
             * @param other Matrix to copy
             */
            Mat4(const Mat4 &other) = default;

            /**
             * @brief Assignment operator
             * @param other Matrix to assign
             * @return Reference to this matrix
             */
            Mat4 &operator=(const Mat4 &other) = default;

            // Element access
            /**
             * @brief Get element at column and row
             * @param col Column index (0-3)
             * @param row Row index (0-3)
             * @return Element value
             */
            f32 Get(i32 col, i32 row) const;

            /**
             * @brief Set element at column and row
             * @param col Column index (0-3)
             * @param row Row index (0-3)
             * @param value Value to set
             */
            void Set(i32 col, i32 row, f32 value);

            /**
             * @brief Array access operator (const)
             * @param index Linear index (0-15)
             * @return Element value
             */
            f32 operator[](i32 index) const;

            /**
             * @brief Array access operator (non-const)
             * @param index Linear index (0-15)
             * @return Reference to element
             */
            f32 &operator[](i32 index);

            // Column and row access
            /**
             * @brief Get column as Vec4
             * @param col Column index (0-3)
             * @return Column vector
             */
            Vec4 GetColumn(i32 col) const;

            /**
             * @brief Set column from Vec4
             * @param col Column index (0-3)
             * @param column Column vector
             */
            void SetColumn(i32 col, const Vec4 &column);

            /**
             * @brief Get row as Vec4
             * @param row Row index (0-3)
             * @return Row vector
             */
            Vec4 GetRow(i32 row) const;

            /**
             * @brief Set row from Vec4
             * @param row Row index (0-3)
             * @param rowVec Row vector
             */
            void SetRow(i32 row, const Vec4 &rowVec);

            // Arithmetic operators
            /**
             * @brief Matrix addition
             * @param other Matrix to add
             * @return Result of addition
             */
            Mat4 operator+(const Mat4 &other) const;

            /**
             * @brief Matrix subtraction
             * @param other Matrix to subtract
             * @return Result of subtraction
             */
            Mat4 operator-(const Mat4 &other) const;

            /**
             * @brief Matrix multiplication
             * @param other Matrix to multiply with
             * @return Result of multiplication
             */
            Mat4 operator*(const Mat4 &other) const;

            /**
             * @brief Scalar multiplication
             * @param scalar Scalar to multiply with
             * @return Result of multiplication
             */
            Mat4 operator*(f32 scalar) const;

            /**
             * @brief Matrix-vector multiplication
             * @param vec Vector to multiply with
             * @return Transformed vector
             */
            Vec4 operator*(const Vec4 &vec) const;

            /**
             * @brief Unary negation
             * @return Negated matrix
             */
            Mat4 operator-() const;

            // Compound assignment operators
            Mat4 &operator+=(const Mat4 &other);
            Mat4 &operator-=(const Mat4 &other);
            Mat4 &operator*=(const Mat4 &other);
            Mat4 &operator*=(f32 scalar);

            // Comparison operators
            bool operator==(const Mat4 &other) const;
            bool operator!=(const Mat4 &other) const;

            // Matrix operations
            /**
             * @brief Calculate the transpose of the matrix
             * @return Transposed matrix
             */
            Mat4 Transpose() const;

            /**
             * @brief Transpose this matrix in place
             * @return Reference to this matrix
             */
            Mat4 &TransposeInPlace();

            /**
             * @brief Calculate the determinant of the matrix
             * @return Determinant value
             */
            f32 Determinant() const;

            /**
             * @brief Calculate the inverse of the matrix
             * @return Inverse matrix (identity if not invertible)
             */
            Mat4 Inverse() const;

            /**
             * @brief Invert this matrix in place
             * @return Reference to this matrix
             */
            Mat4 &InvertInPlace();

            /**
             * @brief Check if the matrix is invertible
             * @return True if invertible
             */
            bool IsInvertible() const;

            /**
             * @brief Check if the matrix is identity
             * @return True if identity matrix
             */
            bool IsIdentity() const;

            /**
             * @brief Set this matrix to identity
             * @return Reference to this matrix
             */
            Mat4 &SetIdentity();

            /**
             * @brief Set this matrix to zero
             * @return Reference to this matrix
             */
            Mat4 &SetZero();

            /**
             * @brief Get pointer to matrix data (for OpenGL)
             * @return Pointer to matrix data
             */
            const f32 *Data() const;

            /**
             * @brief Get pointer to matrix data (for OpenGL)
             * @return Pointer to matrix data
             */
            f32 *Data();

            /**
             * @brief Convert matrix to string representation
             * @return String representation
             */
            std::string ToString() const;

            // Static utility functions
            /**
             * @brief Identity matrix
             */
            static const Mat4 Identity;

            /**
             * @brief Zero matrix
             */
            static const Mat4 Zero;

            /**
             * @brief Create identity matrix
             * @return Identity matrix
             */
            static Mat4 CreateIdentity();

            /**
             * @brief Create zero matrix
             * @return Zero matrix
             */
            static Mat4 CreateZero();

            // Transformation matrix creation functions
            /**
             * @brief Create translation matrix
             * @param translation Translation vector
             * @return Translation matrix
             */
            static Mat4 CreateTranslation(const Vec3 &translation);

            /**
             * @brief Create translation matrix
             * @param x X translation
             * @param y Y translation
             * @param z Z translation
             * @return Translation matrix
             */
            static Mat4 CreateTranslation(f32 x, f32 y, f32 z);

            /**
             * @brief Create scale matrix
             * @param scale Scale vector
             * @return Scale matrix
             */
            static Mat4 CreateScale(const Vec3 &scale);

            /**
             * @brief Create uniform scale matrix
             * @param scale Uniform scale factor
             * @return Scale matrix
             */
            static Mat4 CreateScale(f32 scale);

            /**
             * @brief Create scale matrix
             * @param x X scale
             * @param y Y scale
             * @param z Z scale
             * @return Scale matrix
             */
            static Mat4 CreateScale(f32 x, f32 y, f32 z);

            /**
             * @brief Create rotation matrix around X axis
             * @param angle Rotation angle in radians
             * @return Rotation matrix
             */
            static Mat4 CreateRotationX(f32 angle);

            /**
             * @brief Create rotation matrix around Y axis
             * @param angle Rotation angle in radians
             * @return Rotation matrix
             */
            static Mat4 CreateRotationY(f32 angle);

            /**
             * @brief Create rotation matrix around Z axis
             * @param angle Rotation angle in radians
             * @return Rotation matrix
             */
            static Mat4 CreateRotationZ(f32 angle);

            /**
             * @brief Create rotation matrix around arbitrary axis
             * @param axis Rotation axis (should be normalized)
             * @param angle Rotation angle in radians
             * @return Rotation matrix
             */
            static Mat4 CreateRotation(const Vec3 &axis, f32 angle);

            /**
             * @brief Create perspective projection matrix
             * @param fov Field of view in radians
             * @param aspect Aspect ratio (width/height)
             * @param nearPlane Near clipping plane
             * @param farPlane Far clipping plane
             * @return Perspective projection matrix
             */
            static Mat4 CreatePerspective(f32 fov, f32 aspect, f32 nearPlane, f32 farPlane);

            /**
             * @brief Create orthographic projection matrix
             * @param left Left clipping plane
             * @param right Right clipping plane
             * @param bottom Bottom clipping plane
             * @param top Top clipping plane
             * @param nearPlane Near clipping plane
             * @param farPlane Far clipping plane
             * @return Orthographic projection matrix
             */
            static Mat4 CreateOrthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearPlane, f32 farPlane);

            /**
             * @brief Create look-at view matrix
             * @param eye Camera position
             * @param target Target position to look at
             * @param up Up vector
             * @return View matrix
             */
            static Mat4 CreateLookAt(const Vec3 &eye, const Vec3 &target, const Vec3 &up);
        };

        // Non-member operators
        /**
         * @brief Scalar multiplication (scalar * matrix)
         * @param scalar Scalar value
         * @param matrix Matrix to multiply
         * @return Result of multiplication
         */
        Mat4 operator*(f32 scalar, const Mat4 &matrix);

    } // namespace Math
} // namespace Pyramid
