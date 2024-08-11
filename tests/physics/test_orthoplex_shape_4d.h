#pragma once

#include "../../physics/shapes/orthoplex_shape_4d.h"
#include "tests/test_macros.h"

namespace TestOrthoplexShape4D {
TEST_CASE("[OrthoplexShape4D] Get Nearest Point") {
	Ref<OrthoplexShape4D> orthoplex;
	orthoplex.instantiate();
	// Default size is (1, 1, 1, 1), which means a "radius" of 0.5.
	// This means the adding up the output's absolute values should be less than or equal to 0.5.
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.0, 0.0, 0.0, 0.0)) == Vector4(0.0, 0.0, 0.0, 0.0), "OrthoplexShape4D get_nearest_point should return the same point when the point is inside the shape.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.25, 0.25, 0.25, 0.25)) == Vector4(0.125, 0.125, 0.125, 0.125), "OrthoplexShape4D get_nearest_point should return the same point when the point is on the surface of the shape.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.5, 0.5, 0.5, 0.5)) == Vector4(0.125, 0.125, 0.125, 0.125), "OrthoplexShape4D get_nearest_point should return the nearest point when the point is outside the shape.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(1.0, 1.0, 1.0, 0.0)).is_equal_approx(Vector4(1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0, 0.0)), "OrthoplexShape4D get_nearest_point should return a point on the triangle face when one axis is zero.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.6, 0.3, 0.0, 0.0)).is_equal_approx(Vector4(0.4, 0.1, 0.0, 0.0)), "OrthoplexShape4D get_nearest_point should return a point on an edge when two axes are zero.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(1.7, 0.1, 0.2, 0.3)).is_equal_approx(Vector4(0.5, 0.0, 0.0, 0.0)), "OrthoplexShape4D get_nearest_point should return zero for the smallest axes when they make up less than a quarter of the total.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(-0.1, -0.2, -1.2, 1.3)).is_equal_approx(Vector4(0.0, 0.0, -0.2, 0.3)), "OrthoplexShape4D get_nearest_point should work for negative values.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(-0.3, 1.15, -1.35, 0.05)).is_equal_approx(Vector4(0.0, 0.15, -0.35, 0.0)), "OrthoplexShape4D get_nearest_point should work for negative values.");
	// Test when the orthoplex is not at the default size.
	orthoplex->set_size(Vector4(3, 4, 5, 2));
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.0, 0.0, 0.0, 0.0)) == Vector4(0.0, 0.0, 0.0, 0.0), "OrthoplexShape4D get_nearest_point should return the same point when the point is inside the shape.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.75, 1.0, 1.25, 0.5)) == Vector4(0.375, 0.5, 0.625, 0.25), "OrthoplexShape4D get_nearest_point should return the same point when the point is on the surface of the shape.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(1.5, 2.0, 2.5, 1.0)) == Vector4(0.375, 0.5, 0.625, 0.25), "OrthoplexShape4D get_nearest_point should return the nearest point when the point is outside the shape.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(3.0, 4.0, 5.0, 0.0)).is_equal_approx(Vector4(0.5, 2.0 / 3.0, 5.0 / 6.0, 0.0)), "OrthoplexShape4D get_nearest_point should return a point on the triangle face when one axis is zero.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(-4.3, 0.1, -0.2, 0.3)).is_equal_approx(Vector4(-1.5, 0.0, 0.0, 0.0)), "OrthoplexShape4D get_nearest_point should return zero for the smallest axes when they make up less than a quarter of the total.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(-6.0, -7.0, -8.0, -9.0)).is_equal_approx(Vector4(0.0, 0.0, 0.0, -1.0)), "OrthoplexShape4D get_nearest_point should work for negative values.");
}
} // namespace TestOrthoplexShape4D
