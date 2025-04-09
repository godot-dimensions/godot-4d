#pragma once

#include "../../../math/geometric_algebra/multivector_4d.h"
#include "core/math/random_number_generator.h"
#include "tests/test_macros.h"

namespace TestMultivector4D {
TEST_CASE("[Multivector4D] Inverse") {
	Multivector4D test = Multivector4D(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
	Multivector4D inv = test.inverse();
	CHECK_MESSAGE((test * inv).is_equal_approx(Multivector4D::identity()), "Multivector4D inverse should work as expected.");

	test = test.normalized();
	CHECK_MESSAGE(test.is_normalized(), "Multivector4D normalized should return a normalized multivector.");
	inv = test.inverse();
	CHECK_MESSAGE((test * inv).is_equal_approx(Multivector4D::identity()), "Multivector4D inverse of normalized should work as expected.");
}

TEST_CASE("[Multivector4D] Length or Magnitude") {
	Multivector4D test;
	test = Multivector4D::from_scalar_vector(1, Vector4(1, 0, 0, 0));
	CHECK_MESSAGE(test.length_squared() == 2, "Multivector4D scalar-vector length squared should work as expected.");
	CHECK_MESSAGE(test.length() == doctest::Approx(Math_SQRT2), "Multivector4D scalar-vector length should be the square root of the length squared.");

	test = Multivector4D(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
	// 2*2 + 3*3 + 4*4 + 5*5 + 6*6 + 7*7 + 8*8 + 9*9 + 10*10 + 11*11 + 12*12 + 13*13 + 14*14 + 15*15 + 16*16 + 17*17
	CHECK_MESSAGE(test.length_squared() == 1784, "Multivector4D full length squared should work as expected.");
	CHECK_MESSAGE(test.length() == doctest::Approx(Math::sqrt(1784.0)), "Multivector4D full length should be the square root of the length squared.");
}

TEST_CASE("[Multivector4D] Multiplication") {
	// Note: In the case of integer inputs, the results are expected to be exact, so use == instead of is_equal_approx.
	Multivector4D a, b, expected;
	a = Multivector4D::from_scalar(2);
	b = Multivector4D::from_scalar(3);
	expected = Multivector4D::from_scalar(6);
	CHECK_MESSAGE(a * b == expected, "Multivector4D scalar-scalar multiplication should work as expected.");

	a = Multivector4D::from_scalar(2);
	b = Multivector4D::from_vector(Vector4(3, 4, 0, 0));
	expected = Multivector4D::from_vector(Vector4(6, 8, 0, 0));
	CHECK_MESSAGE(a * b == expected, "Multivector4D scalar-vector multiplication should work as expected.");

	a = Multivector4D::from_vector(Vector4(5, 0, 0, 0));
	b = Multivector4D::from_vector(Vector4(6, 7, 0, 0));
	expected = Multivector4D::from_scalar_bivector(30, Bivector4D(35, 0, 0, 0, 0, 0));
	CHECK_MESSAGE(a * b == expected, "Multivector4D vector-vector multiplication should work as expected.");
	expected.xy = -expected.xy;
	CHECK_MESSAGE(b * a == expected, "Multivector4D vector-vector multiplication should have anti-commutative bivector components.");

	a = Multivector4D::from_vector(Vector4(1, 0, 0, 1));
	b = Multivector4D::from_vector(Vector4(0, 0, 0, 1));
	expected = Multivector4D::from_scalar_bivector(1, Bivector4D(0, 0, 1, 0, 0, 0));
	CHECK_MESSAGE(a * b == expected, "Multivector4D vector-vector multiplication should work as expected.");
	expected.xw = -expected.xw;
	CHECK_MESSAGE(b * a == expected, "Multivector4D vector-vector multiplication should have anti-commutative bivector components.");

	a = Multivector4D::from_vector(Vector4(1, 0, 0, 0));
	b = Multivector4D::from_bivector(Bivector4D(1, 0, 0, 0, 0, 1));
	expected = Multivector4D::from_vector_trivector(Vector4(0, 1, 0, 0), Trivector4D(0, 0, 1, 0));
	CHECK_MESSAGE(a * b == expected, "Multivector4D vector-bivector multiplication should work as expected.");

	a = Multivector4D::from_vector_bivector(Vector4(1, 0, 0, 0), Bivector4D(0, 0, 0, 0, 1, 0));
	b = Multivector4D::from_bivector(Bivector4D(0, 0, 1, 1, 0, 0));
	expected = Multivector4D(0, Vector4(0, 0, 0, 1), Bivector4D(1, 0, 0, 0, 0, 1), Trivector4D(1, 0, 0, 0), 0);
	CHECK_MESSAGE(a * b == expected, "Multivector4D vector+bivector-bivector multiplication should work as expected.");

	a = Multivector4D::from_bivector(Bivector4D(0, 0, 1, 0, 0, 0));
	b = Multivector4D::from_vector_bivector(Vector4(0, 0, 0, 1), Bivector4D(0, 0, 0, 1, 0, 0));
	expected = Multivector4D::from_vector_pseudoscalar(Vector4(1, 0, 0, 0), 1);
	CHECK_MESSAGE(a * b == expected, "Multivector4D bivector-vector+bivector multiplication should work as expected.");

	a = Multivector4D::from_bivector(Bivector4D(1, 1, 0, 0, 0, 0));
	b = Multivector4D::from_bivector(Bivector4D(1, 0, 0, 1, 0, 0));
	// Expect (xy + xz)(xy + yz) = xyxy + xyyz + xzxy + xzyz = -1 - xy + xz + yz
	expected = Multivector4D::from_scalar_bivector(-1, Bivector4D(-1, 1, 0, 1, 0, 0));
	CHECK_MESSAGE(a * b == expected, "Multivector4D bivector-bivector multiplication should work as expected.");
	// Expect (xy + yz)(xy + xz) = xyxy + xyxz + yzxy + yzxz = -1 + xy - xz - yz
	expected = Multivector4D::from_scalar_bivector(-1, Bivector4D(1, -1, 0, -1, 0, 0));
	CHECK_MESSAGE(b * a == expected, "Multivector4D bivector-bivector multiplication in reverse should flip the sign of the bivector components.");

	a = Multivector4D::from_trivector(Trivector4D(1, 0, 0, 0));
	b = Multivector4D::from_vector(Vector4(1, 1, 1, 0));
	expected = Multivector4D::from_bivector(Bivector4D(1, -1, 0, 1, 0, 0));
	CHECK_MESSAGE(a * b == expected, "Multivector4D trivector-vector multiplication should work as expected.");

	a = Multivector4D::from_trivector(Trivector4D(0, 1, 0, 0));
	b = Multivector4D::from_vector_bivector(Vector4(1, 0, 0, 0), Bivector4D(0, 0, 0, 0, 1, 0));
	expected = Multivector4D::from_vector_bivector(Vector4(-1, 0, 0, 0), Bivector4D(0, 0, 0, 0, 1, 0));
	CHECK_MESSAGE(a * b == expected, "Multivector4D trivector-vector+bivector multiplication should work as expected.");

	a = Multivector4D::from_scalar(2);
	b = Multivector4D::from_pseudoscalar(3);
	expected = Multivector4D::from_pseudoscalar(6);
	CHECK_MESSAGE(a * b == expected, "Multivector4D scalar-pseudoscalar multiplication should work as expected.");

	a = Multivector4D::from_pseudoscalar(2);
	b = Multivector4D::from_vector(Vector4(3, 4, 5, 6));
	expected = Multivector4D::from_trivector(Trivector4D(12, -10, 8, -6));
	CHECK_MESSAGE(a * b == expected, "Multivector4D pseudoscalar-vector multiplication should work as expected.");

	a = Multivector4D::from_scalar(2);
	b = Multivector4D::from_vector_pseudoscalar(Vector4(0, 0, 0, 3), 4);
	expected = Multivector4D::from_vector_pseudoscalar(Vector4(0, 0, 0, 6), 8);
	CHECK_MESSAGE(a * b == expected, "Multivector4D scalar-vector+pseudoscalar multiplication should work as expected.");

	a = Multivector4D::from_pseudoscalar(2);
	b = Multivector4D::from_bivector(Bivector4D(3, 4, 5, 6, 7, 8));
	expected = Multivector4D::from_bivector(Bivector4D(-16, 14, -12, -10, 8, -6));
	CHECK_MESSAGE(a * b == expected, "Multivector4D pseudoscalar-bivector multiplication should work as expected.");

	a = Multivector4D(1, Vector4(1, 0, 0, 0), Bivector4D(1, 0, 0, 0, 0, 0), Trivector4D(1, 0, 0, 0), 1);
	b = Multivector4D(1, Vector4(0, 1, 0, 0), Bivector4D(0, 0, 0, 1, 0, 0), Trivector4D(0, 0, 0, 1), 1);
	expected = Multivector4D(2, Vector4(0, 1, 0, -1), Bivector4D(2, 0, -2, 1, 0, -1), Trivector4D(2, 0, 2, 2), 3);
	CHECK_MESSAGE(a * b == expected, "Multivector4D full combination multiplication should work as expected.");
}

TEST_CASE("[Multivector4D] Rotation of Vector4") {
	Vector4 vector, expected;
	{
		Multivector4D rot_xy_90 = Multivector4D::vector_product(Vector4(1, 0, 0, 0), Vector4(Math_SQRT12, Math_SQRT12, 0, 0));
		vector = Vector4(1, 0, 0, 0);
		expected = Vector4(0, 1, 0, 0);
		CHECK_MESSAGE(rot_xy_90.rotate_vector(vector).is_equal_approx(expected), "Multivector4D rotation of Vector4 should work as expected.");

		// Alternate way to create the same rotation.
		rot_xy_90 = Multivector4D::from_rotor(Rotor4D::from_xy(Math_TAU / 2));
		CHECK_MESSAGE(rot_xy_90.is_equal_approx(rot_xy_90), "Multivector4D rotation of Vector4 should work as expected.");
	}
}
} // namespace TestMultivector4D
