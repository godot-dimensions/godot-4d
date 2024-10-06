#pragma once

#include "../../../math/geometric_algebra/rotor_4d.h"
#include "core/math/random_number_generator.h"
#include "tests/test_macros.h"

namespace TestRotor4D {
TEST_CASE("[Rotor4D] Conversion to Basis4D") {
	Rotor4D rot_xy_rotor = Rotor4D::rotation_xy(Math_TAU / 4);
	CHECK_MESSAGE(rot_xy_rotor.is_equal_approx(Rotor4D(Math_SQRT12, Math_SQRT12, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation should work as expected.");
	CHECK_MESSAGE(rot_xy_rotor.get_rotation_basis().is_equal_approx(Basis4D(0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)), "Rotor4D to_basis should work as expected.");
	Basis4D rot_xy_basis = Basis4D::from_xy(Math_TAU / 4);
	CHECK_MESSAGE(rot_xy_rotor.get_rotation_basis().is_equal_approx(rot_xy_basis), "Rotor4D to_basis should work as expected.");

	Rotor4D rot_zw_rotor = Rotor4D::rotation_zw(Math_TAU / 8);
	Basis4D rot_zw_basis = Basis4D::from_zw(Math_TAU / 8);
	CHECK_MESSAGE(rot_zw_rotor.get_rotation_basis().is_equal_approx(rot_zw_basis), "Rotor4D to_basis should work as expected.");

	Rotor4D rot_yw_rotor = Rotor4D::rotation_yw(Math_TAU / 16);
	Basis4D rot_wy_basis = Basis4D::from_wy(-Math_TAU / 16);
	CHECK_MESSAGE(rot_yw_rotor.get_rotation_basis().is_equal_approx(rot_wy_basis), "Rotor4D to_basis should work as expected.");
}

TEST_CASE("[Rotor4D] Inverse") {
	Rotor4D test = Rotor4D(2, 3, 4, 5, 6, 7, 8, 9);
	Rotor4D inv = test.inverse();
	CHECK_MESSAGE((test * inv).is_equal_approx(Rotor4D::identity()), "Rotor4D inverse should work as expected.");

	test = test.normalized();
	CHECK_MESSAGE(test.is_normalized(), "Rotor4D normalized should return a normalized rotor.");
	inv = test.inverse();
	CHECK_MESSAGE((test * inv).is_equal_approx(Rotor4D::identity()), "Rotor4D inverse of normalized should work as expected.");
}

TEST_CASE("[Rotor4D] Length or Magnitude") {
	Rotor4D test;
	test = Rotor4D::from_scalar_bivector(1, Bivector4D(1, 0, 0, 0, 0, 0));
	CHECK_MESSAGE(test.length_squared() == 2, "Rotor4D scalar-bivector length squared should work as expected.");
	CHECK_MESSAGE(test.length() == doctest::Approx(Math_SQRT2), "Rotor4D scalar-bivector length should be the square root of the length squared.");

	test = Rotor4D(2, 3, 4, 5, 6, 7, 8, 9);
	// 2*2 + 3*3 + 4*4 + 5*5 + 6*6 + 7*7 + 8*8 + 9*9
	CHECK_MESSAGE(test.length_squared() == 284, "Rotor4D full length squared should work as expected.");
	CHECK_MESSAGE(test.length() == doctest::Approx(Math::sqrt(284.0)), "Rotor4D full length should be the square root of the length squared.");
}

TEST_CASE("[Rotor4D] Multiplication") {
	// Note: In the case of integer inputs, the results are expected to be exact, so use == instead of is_equal_approx.
	Rotor4D a, b, expected;
	a = Rotor4D::from_scalar(2);
	b = Rotor4D::from_scalar(3);
	expected = Rotor4D::from_scalar(6);
	CHECK_MESSAGE(a * b == expected, "Rotor4D scalar-scalar multiplication should work as expected.");

	a = Rotor4D::from_bivector(Bivector4D(1, 1, 0, 0, 0, 0));
	b = Rotor4D::from_bivector(Bivector4D(1, 0, 0, 1, 0, 0));
	// Expect (xy + xz)(xy + yz) = xyxy + xyyz + xzxy + xzyz = -1 - xy + xz + yz
	expected = Rotor4D::from_scalar_bivector(-1, Bivector4D(-1, 1, 0, 1, 0, 0));
	CHECK_MESSAGE(a * b == expected, "Rotor4D bivector-bivector multiplication should work as expected.");
	// Expect (xy + yz)(xy + xz) = xyxy + xyxz + yzxy + yzxz = -1 + xy - xz - yz
	expected = Rotor4D::from_scalar_bivector(-1, Bivector4D(1, -1, 0, -1, 0, 0));
	CHECK_MESSAGE(b * a == expected, "Rotor4D bivector-bivector multiplication in reverse should flip the sign of the bivector components.");

	a = Rotor4D::from_scalar(2);
	b = Rotor4D::from_pseudoscalar(3);
	expected = Rotor4D::from_pseudoscalar(6);
	CHECK_MESSAGE(a * b == expected, "Rotor4D scalar-pseudoscalar multiplication should work as expected.");

	a = Rotor4D::from_pseudoscalar(2);
	b = Rotor4D::from_bivector(Bivector4D(3, 4, 5, 6, 7, 8));
	expected = Rotor4D::from_bivector(Bivector4D(-16, 14, -12, -10, 8, -6));
	CHECK_MESSAGE(a * b == expected, "Rotor4D pseudoscalar-bivector multiplication should work as expected.");

	a = Rotor4D(1, Bivector4D(1, 0, 0, 0, 0, 0), 1);
	b = Rotor4D(1, Bivector4D(0, 0, 0, 1, 0, 0), 1);
	expected = Rotor4D(2, Bivector4D(1, 1, -1, 1, 0, -1), 2);
	CHECK_MESSAGE(a * b == expected, "Rotor4D full combination multiplication should work as expected.");

	a = Rotor4D::rotation_yz(Math_TAU / 2);
	b = Rotor4D::rotation_zx(Math_TAU / 2);
	expected = Rotor4D::rotation_xy(-Math_TAU / 2);
	// Analogous to: i * j == k but following Geometric Algebra conventions (xz instead of zx).
	CHECK_MESSAGE((a * b).is_equal_approx(expected), "Rotor4D rotation multiplication should work as expected.");

	a = Rotor4D::rotation_xy(Math_TAU / 2);
	b = Rotor4D::rotation_yz(Math_TAU / 2);
	expected = Rotor4D::rotation_xz(Math_TAU / 2);
	// Analogous to k * i == j but following Geometric Algebra conventions (xz instead of zx).
	CHECK_MESSAGE((a * b).is_equal_approx(expected), "Rotor4D rotation multiplication should work as expected.");
}

TEST_CASE("[Rotor4D] Rotation Bivector-Angle Conversion") {
	real_t angle_radians = Math_TAU / 8.0;
	Rotor4D rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(0.923879532511286756, 0.38268343236508977, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(angle_radians), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(Math_SQRT12, Math_SQRT12, 0, 0)), "Rotor4D rotate_vector should work as expected.");

	angle_radians = -Math_TAU / 8.0;
	rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(0.923879532511286756, -0.38268343236508977, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(-angle_radians), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(-1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(Math_SQRT12, -Math_SQRT12, 0, 0)), "Rotor4D rotate_vector should work as expected.");

	angle_radians = Math_TAU / 8.0;
	rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(-1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(0.923879532511286756, -0.38268343236508977, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(-angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(angle_radians), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(-1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(Math_SQRT12, -Math_SQRT12, 0, 0)), "Rotor4D rotate_vector should work as expected.");

	angle_radians = Math_TAU / 4.0;
	rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(Math_SQRT12, Math_SQRT12, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(angle_radians), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(0, 1, 0, 0)), "Rotor4D rotate_vector should work as expected.");

	angle_radians = -Math_TAU / 4.0;
	rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(Math_SQRT12, -Math_SQRT12, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(Math::abs(angle_radians)), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(-1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(0, -1, 0, 0)), "Rotor4D rotate_vector should work as expected.");

	angle_radians = Math_TAU / 2.0;
	rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(0, 1, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(angle_radians), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(-1, 0, 0, 0)), "Rotor4D rotate_vector should work as expected.");

	angle_radians = Math_TAU / 2.0;
	rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(0, 1, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(angle_radians), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(-1, 0, 0, 0)), "Rotor4D rotate_vector should work as expected.");

	angle_radians = Math_TAU * 3.0 / 4.0;
	rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(-Math_SQRT12, Math_SQRT12, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(angle_radians), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(0, -1, 0, 0)), "Rotor4D rotate_vector should work as expected.");

	angle_radians = -Math_TAU * 3.0 / 4.0;
	rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(-Math_SQRT12, -Math_SQRT12, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(Math::abs(angle_radians)), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(-1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(0, 1, 0, 0)), "Rotor4D rotate_vector should work as expected.");

	angle_radians = Math_TAU * 7.0 / 8.0;
	rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(-0.923879532511286756, 0.38268343236508977, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(Math::fposmod(angle_radians, (real_t)Math_TAU)), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(Math_SQRT12, -Math_SQRT12, 0, 0)), "Rotor4D rotate_vector should work as expected.");

	angle_radians = Math_TAU * 17.0 / 8.0;
	rot_xy = Rotor4D::rotation_bivector_normal_angle(Bivector4D(1, 0, 0, 0, 0, 0), angle_radians);
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D(0.923879532511286756, 0.38268343236508977, 0, 0, 0, 0, 0, 0)), "Rotor4D XY rotation from bivector and angle should work as expected.");
	CHECK_MESSAGE(rot_xy.is_equal_approx(Rotor4D::rotation_xy(angle_radians)), "Rotor4D XY rotation from bivector and angle should match the rotation_xy function.");
	CHECK_MESSAGE(rot_xy.get_rotation_angle() == doctest::Approx(Math::fposmod(angle_radians, (real_t)Math_TAU)), "Rotor4D get_rotation_angle should work as expected.");
	CHECK_MESSAGE(rot_xy.get_rotation_bivector_normal().is_equal_approx(Bivector4D(1, 0, 0, 0, 0, 0)), "Rotor4D get_bivector_rotation should work as expected.");
	CHECK_MESSAGE(rot_xy.rotate_vector(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(Math_SQRT12, Math_SQRT12, 0, 0)), "Rotor4D rotate_vector should work as expected.");
}

TEST_CASE("[Rotor4D] Rotation of Vector4") {
	Vector4 vector, expected;
	{
		Rotor4D rot_xy_90 = Rotor4D::rotation_xy(Math_TAU / 4);
		vector = Vector4(1, 0, 0, 0);
		expected = Vector4(0, 1, 0, 0);
		CHECK_MESSAGE(rot_xy_90.rotate_vector(vector).is_equal_approx(expected), "Rotor4D rotation of Vector4 should work as expected.");
		// Alternate way to create the same rotation.
		rot_xy_90 = Rotor4D::vector_product(Vector4(1, 0, 0, 0), Vector4(Math_SQRT12, Math_SQRT12, 0, 0));
		CHECK_MESSAGE(rot_xy_90.rotate_vector(vector).is_equal_approx(expected), "Rotor4D rotation of Vector4 should work as expected.");
	}
}

TEST_CASE("[Rotor4D] Rotation of Basis4D") {
	Basis4D basis, expected, result;
	{
		Rotor4D rot_xy_zw = Rotor4D::rotation_xy(Math_TAU / 4) * Rotor4D::rotation_zw(Math_TAU / 4);
		basis = Basis4D::from_scale(Vector4(2, 3, 4, 5));
		expected = Basis4D(Vector4(0, 2, 0, 0), Vector4(-3, 0, 0, 0), Vector4(0, 0, 0, 4), Vector4(0, 0, -5, 0));
		result = rot_xy_zw.rotate_basis(basis);
		CHECK_MESSAGE(result.get_scale().is_equal_approx(basis.get_scale()), "Rotor4D rotation should not affect the scale of Basis4D.");
		CHECK_MESSAGE(result.is_equal_approx(expected), "Rotor4D rotation of Basis4D should work as expected.");

		// Alternate way to create the same rotation.
		rot_xy_zw = Rotor4D::vector_product(Vector4(1, 0, 0, 0), Vector4(Math_SQRT12, Math_SQRT12, 0, 0)) * Rotor4D::vector_product(Vector4(0, 0, 1, 0), Vector4(0, 0, Math_SQRT12, Math_SQRT12));
		result = rot_xy_zw.rotate_basis(basis);
		CHECK_MESSAGE(result.get_scale().is_equal_approx(basis.get_scale()), "Rotor4D rotation should not affect the scale of Basis4D.");
		CHECK_MESSAGE(result.is_equal_approx(expected), "Rotor4D rotation of Basis4D should work as expected.");

		// Another alternate way to create the same rotation.
		rot_xy_zw = Rotor4D::vector_product(Vector4(Math_SQRT12, Math_SQRT12, 0, 0), Vector4(0, 1, 0, 0)) * Rotor4D::vector_product(Vector4(0, 0, Math_SQRT12, Math_SQRT12), Vector4(0, 0, 0, 1));
		result = rot_xy_zw.rotate_basis(basis);
		CHECK_MESSAGE(result.get_scale().is_equal_approx(basis.get_scale()), "Rotor4D rotation should not affect the scale of Basis4D.");
		CHECK_MESSAGE(result.is_equal_approx(expected), "Rotor4D rotation of Basis4D should work as expected.");

		// Reverse the rotation.
		rot_xy_zw = rot_xy_zw.reverse();
		result = rot_xy_zw.rotate_basis(basis);
		CHECK_MESSAGE(result.get_scale().is_equal_approx(basis.get_scale()), "Rotor4D rotation should not affect the scale of Basis4D.");
		CHECK_MESSAGE(result.is_equal_approx(-expected), "Rotor4D rotation by 90 degrees in opposite direction should give the opposite / negative basis.");
	}
}

TEST_CASE("[Rotor4D] Slerp between rotations") {
	{
		// Rotation in the XY plane by 45 degrees.
		Rotor4D a = Rotor4D::rotation_xy(Math_TAU / 8.0);
		Rotor4D b = Rotor4D::rotation_xy(Math_TAU / 4.0);
		CHECK_MESSAGE(a.slerp(b, 0.0).is_equal_approx(a), "Rotor4D slerp should give the start rotor when the weight is zero.");
		CHECK_MESSAGE(a.slerp(b, 1.0).is_equal_approx(b), "Rotor4D slerp should give the target rotor when the weight is one.");
		CHECK_MESSAGE(a.slerp(b, 0.50).is_equal_approx(Rotor4D::rotation_xy(Math_TAU * 3.0 / 16.0)), "Rotor4D slerp should work as expected.");
		CHECK_MESSAGE(a.slerp(b, 0.25).is_equal_approx(Rotor4D::rotation_xy(Math_TAU * 5.0 / 32.0)), "Rotor4D slerp should work as expected.");
		CHECK_MESSAGE(a.slerp(b, 0.75).is_equal_approx(Rotor4D::rotation_xy(Math_TAU * 7.0 / 32.0)), "Rotor4D slerp should work as expected.");
	}
	{
		// Rotation in the XY plane by 90 degrees.
		Rotor4D a = Rotor4D::rotation_xy(Math_TAU / 4.0);
		Rotor4D b = Rotor4D::rotation_xy(Math_TAU / 2.0);
		CHECK_MESSAGE(a.slerp(b, 0.0).is_equal_approx(a), "Rotor4D slerp should give the start rotor when the weight is zero.");
		CHECK_MESSAGE(a.slerp(b, 1.0).is_equal_approx(b), "Rotor4D slerp should give the target rotor when the weight is one.");
		CHECK_MESSAGE(a.slerp(b, 0.50).is_equal_approx(Rotor4D::rotation_xy(Math_TAU * 3.0 / 8.0)), "Rotor4D slerp should work as expected.");
		CHECK_MESSAGE(a.slerp(b, 0.25).is_equal_approx(Rotor4D::rotation_xy(Math_TAU * 5.0 / 16.0)), "Rotor4D slerp should work as expected.");
		CHECK_MESSAGE(a.slerp(b, 0.75).is_equal_approx(Rotor4D::rotation_xy(Math_TAU * 7.0 / 16.0)), "Rotor4D slerp should work as expected.");
	}
	{
		// Rotation in the XY and XZ planes by 180 degrees.
		Rotor4D a = Rotor4D::rotation_xy(Math_TAU / 2.0);
		Rotor4D b = Rotor4D::rotation_xz(Math_TAU / 2.0);
		CHECK_MESSAGE(a.slerp(b, 0.0).is_equal_approx(a), "Rotor4D slerp should give the start rotor when the weight is zero.");
		CHECK_MESSAGE(a.slerp(b, 1.0).is_equal_approx(b), "Rotor4D slerp should give the target rotor when the weight is one.");
		CHECK_MESSAGE(a.slerp(b, 0.50).is_equal_approx(Rotor4D(0.0, Bivector4D(Math_SQRT12, Math_SQRT12, 0, 0, 0, 0))), "Rotor4D slerp should work as expected.");
		CHECK_MESSAGE(a.slerp(b, 0.5).get_rotation_angle() == doctest::Approx(Math_TAU / 2.0), "Rotor4D slerp between perpendicular 180 degree rotations should give a 180 degree rotation.");
	}
	{
		// Rotation in the XW and ZW planes by -90 degrees.
		Rotor4D a = Rotor4D::rotation_xw(-Math_TAU / 4.0);
		Rotor4D b = Rotor4D::rotation_zw(-Math_TAU / 4.0);
		CHECK_MESSAGE(a.slerp(b, 0.0).is_equal_approx(a), "Rotor4D slerp should give the start rotor when the weight is zero.");
		CHECK_MESSAGE(a.slerp(b, 1.0).is_equal_approx(b), "Rotor4D slerp should give the target rotor when the weight is one.");
		CHECK_MESSAGE(a.slerp(b, 0.50).is_equal_approx(Rotor4D(0.81649658092773, Bivector4D(0, 0, -0.40824829046386, 0, 0, -0.40824829046386))), "Rotor4D slerp should work as expected.");
	}
}

TEST_CASE("[Rotor4D] Slerp fraction") {
	{
		Rotor4D rot = Rotor4D::rotation_xy(Math_TAU / 4.0);
		CHECK_MESSAGE(rot.slerp_fraction(0.0).is_equal_approx(Rotor4D::identity()), "Rotor4D slerp_fraction should give the identity rotor when the weight is zero.");
		CHECK_MESSAGE(rot.slerp_fraction(1.0).is_equal_approx(rot), "Rotor4D slerp_fraction should give the same rotor when the weight is one.");
		CHECK_MESSAGE(rot.slerp_fraction(0.50).is_equal_approx(Rotor4D::rotation_xy(Math_TAU / 8.0)), "Rotor4D slerp_fraction should work as expected.");
		CHECK_MESSAGE(rot.slerp_fraction(0.25).is_equal_approx(Rotor4D::rotation_xy(Math_TAU / 16.0)), "Rotor4D slerp_fraction should work as expected.");
		CHECK_MESSAGE(rot.slerp_fraction(0.75).is_equal_approx(Rotor4D::rotation_xy(Math_TAU * 3.0 / 16.0)), "Rotor4D slerp_fraction should work as expected.");
	}
}
} // namespace TestRotor4D
