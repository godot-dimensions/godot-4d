#pragma once

#include "../../math/vector_4d.h"
#include "tests/test_macros.h"

namespace TestVector4D {
TEST_CASE("[Vector4D] Angle To method") {
	const Vector4 vector_x = Vector4(1, 0, 0, 0);
	const Vector4 vector_y = Vector4(0, 1, 0, 0);
	const Vector4 vector_yz = Vector4(0, 1, 1, 0);
	const Vector4 vector1 = Vector4(1.7, 2.3, 1, 9.1);
	const Vector4 vector2 = Vector4(-8.2, -16, 3, 2.4);
	CHECK_MESSAGE(
			Vector4D::angle_to(vector_x, vector_y) == doctest::Approx(Math_TAU / 4),
			"Vector4 angle_to should work as expected.");
	CHECK_MESSAGE(
			Vector4D::angle_to(vector_x, vector_yz) == doctest::Approx(Math_TAU / 4),
			"Vector4 angle_to should work as expected.");
	CHECK_MESSAGE(
			Vector4D::angle_to(vector_yz, vector_x) == doctest::Approx(Math_TAU / 4),
			"Vector4 angle_to should work as expected.");
	CHECK_MESSAGE(
			Vector4D::angle_to(vector_y, vector_yz) == doctest::Approx(Math_TAU / 8),
			"Vector4 angle_to should work as expected.");
	CHECK_MESSAGE(
			Vector4D::angle_to(vector1, vector2) == doctest::Approx(1.718212531758478876075),
			"Vector4 angle_to should work as expected.");
	CHECK_MESSAGE(
			Vector4D::angle_to(vector2, vector1) == doctest::Approx(1.718212531758478876075),
			"Vector4 angle_to reversed should give the same result.");
}

TEST_CASE("[Vector4D] Cross method") {
	const Vector4 vector_x = Vector4(1, 0, 0, 0);
	const Vector4 vector_y = Vector4(0, 1, 0, 0);
	const Vector4 vector1 = Vector4(1.7, 2.3, 1, 9.1);
	const Vector4 vector2 = Vector4(-8.2, -16, 3, 2.4);

	CHECK_MESSAGE(
			Math::is_equal_approx(Vector4D::cross(vector_x, vector_x), (real_t)0.0),
			"Vector4 cross product of parallel vectors should be zero.");
	CHECK_MESSAGE(
			Math::is_equal_approx(Vector4D::cross(vector_x, vector_y), (real_t)1.0),
			"Vector4 cross product of perpendicular unit vectors should be one.");
	CHECK_MESSAGE(
			Math::is_equal_approx(Vector4D::cross(vector1, vector2), (real_t)Math::sqrt(30421.81)),
			"Vector4 cross product of two vectors should give the expected result.");
	CHECK_MESSAGE(
			Math::is_equal_approx(Vector4D::cross(vector2, vector1), (real_t)Math::sqrt(30421.81)),
			"Vector4 cross product of two vectors reversed should give the same result.");
}

TEST_CASE("[Vector4D] Limit Length") {
	const Vector4 vector = Vector4(10, 10, 10, 10);
	CHECK_MESSAGE(
			Vector4D::limit_length(vector).is_equal_approx(Vector4(0.5, 0.5, 0.5, 0.5)),
			"Vector4 limit_length should work as expected.");
	CHECK_MESSAGE(
			Vector4D::limit_length(vector, 5).is_equal_approx(5 * Vector4(0.5, 0.5, 0.5, 0.5)),
			"Vector4 limit_length should work as expected.");
}

TEST_CASE("[Vector4] Plane methods") {
	const Vector4 vector = Vector4(1.2, 3.4, 5.6, 1.6);
	const Vector4 vector_y = Vector4(0, 1, 0, 0);
	CHECK_MESSAGE(
			Vector4D::bounce(vector, vector_y) == Vector4(1.2, -3.4, 5.6, 1.6),
			"Vector4 bounce on a plane with normal of the Y axis should.");
	CHECK_MESSAGE(
			Vector4D::reflect(vector, vector_y) == Vector4(-1.2, 3.4, -5.6, -1.6),
			"Vector4 reflect on a plane with normal of the Y axis should.");
	CHECK_MESSAGE(
			Vector4D::project(vector, vector_y) == Vector4(0, 3.4, 0, 0),
			"Vector4 projected on the X axis should only give the Y component.");
}
} // namespace TestVector4D
