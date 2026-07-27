#include <Pyramid/Math/Math.hpp>

#include <cmath>
#include <iostream>

namespace
{
    bool NearlyEqual(float left, float right, float epsilon = 1.0e-4f)
    {
        return std::abs(left - right) <= epsilon;
    }

    bool Expect(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "Math test failed: " << message << '\n';
        }
        return condition;
    }
}

int main()
{
    using namespace Pyramid::Math;

    bool passed = true;
    const Vec3 value(3.0f, 4.0f, 0.0f);
    passed &= Expect(NearlyEqual(value.Length(), 5.0f), "Vec3 length must remain correct");
    const Vec3 normalized = value.Normalized();
    passed &= Expect(
        NearlyEqual(normalized.x, 0.6f) && NearlyEqual(normalized.y, 0.8f) && NearlyEqual(normalized.z, 0.0f),
        "Vec3 normalization must remain correct");

    const Quat rotation = Quat::FromAxisAngle(Vec3::Up, Radians(90.0f));
    const Vec3 rotated = rotation.RotateVector(Vec3::Forward);
    passed &= Expect(
        NearlyEqual(rotated.x, Vec3::Right.x) &&
            NearlyEqual(rotated.y, Vec3::Right.y) &&
            NearlyEqual(rotated.z, Vec3::Right.z),
        "quaternion vector rotation must preserve handedness");

    const Mat4 transform = Mat4::CreateTranslation(2.0f, 3.0f, 4.0f) * Mat4::CreateScale(2.0f);
    const Vec4 transformed = transform * Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    passed &= Expect(
        NearlyEqual(transformed.x, 4.0f) && NearlyEqual(transformed.y, 5.0f) &&
            NearlyEqual(transformed.z, 6.0f) && NearlyEqual(transformed.w, 1.0f),
        "translation-scale point transformation must remain correct");

    const Vec4 recovered = transform.Inverse() * transformed;
    passed &= Expect(
        NearlyEqual(recovered.x, 1.0f) && NearlyEqual(recovered.y, 1.0f) &&
            NearlyEqual(recovered.z, 1.0f) && NearlyEqual(recovered.w, 1.0f),
        "matrix inversion must recover transformed points");
    passed &= Expect(NearlyEqual(transform.Determinant(), 8.0f),
        "translation must not change the determinant of a uniform scale");
    passed &= Expect((transform * transform.Inverse()).IsIdentity(),
        "a matrix multiplied by its inverse must produce identity");

    const Mat4 complexTransform =
        Mat4::CreateTranslation(-7.0f, 2.5f, 11.0f) *
        Mat4::CreateRotationY(Radians(37.0f)) *
        Mat4::CreateScale(2.0f, 3.0f, 4.0f);
    const Vec4 complexPoint(-3.0f, 5.0f, 1.5f, 1.0f);
    const Vec4 complexRecovered = complexTransform.Inverse() * (complexTransform * complexPoint);
    passed &= Expect(
        NearlyEqual(complexRecovered.x, complexPoint.x) &&
            NearlyEqual(complexRecovered.y, complexPoint.y) &&
            NearlyEqual(complexRecovered.z, complexPoint.z) &&
            NearlyEqual(complexRecovered.w, complexPoint.w),
        "pivoted inversion must recover points through rotated non-uniform transforms");
    passed &= Expect(NearlyEqual(complexTransform.Determinant(), 24.0f),
        "a rigid transform must preserve the determinant of its non-uniform scale");
    passed &= Expect(!Mat4::Zero.IsInvertible() && Mat4::Zero.Inverse().IsIdentity(),
        "singular matrices must remain explicitly non-invertible and use the documented identity fallback");

    return passed ? 0 : 1;
}
