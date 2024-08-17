#pragma once

#include "../../math/geometry_4d.h"
#include "tests/test_macros.h"

namespace TestGeometry4D {
TEST_CASE("[Geometry4D] Closest Points Between Lines") {
	{
		Vector4 line1_point = Vector4(1, 2, 3, 4);
		Vector4 line1_direction = Vector4(2, 0, 0, 0);
		Vector4 line2_point = Vector4(5, 6, 7, 8);
		Vector4 line2_direction = Vector4(0, 3, 0, 0);
		PackedVector4Array result = Geometry4D::closest_points_between_lines(line1_point, line1_direction, line2_point, line2_direction);
		CHECK_MESSAGE(result[0].is_equal_approx(Vector4(5, 2, 3, 4)), "Geometry4D closest_points_between_lines should work as expected.");
		CHECK_MESSAGE(result[1].is_equal_approx(Vector4(5, 2, 7, 8)), "Geometry4D closest_points_between_lines should work as expected.");
	}

	{
		Vector4 line1_point = Vector4(1, 2, 3, 4);
		Vector4 line1_direction = Vector4(0, 0, 1, 1);
		Vector4 line2_point = Vector4(5, 6, 7, 9);
		Vector4 line2_direction = Vector4(0, 0, 0, 1);
		PackedVector4Array result = Geometry4D::closest_points_between_lines(line1_point, line1_direction, line2_point, line2_direction);
		CHECK_MESSAGE(result[0].is_equal_approx(Vector4(1, 2, 7, 8)), "Geometry4D closest_points_between_lines should work as expected.");
		CHECK_MESSAGE(result[1].is_equal_approx(Vector4(5, 6, 7, 8)), "Geometry4D closest_points_between_lines should work as expected.");
	}

	{
		Vector4 line1_point = Vector4(0, 0, 0, 0);
		Vector4 line1_direction = Vector4(1, 0, 1, 0);
		Vector4 line2_point = Vector4(0, 0, 0, 0);
		Vector4 line2_direction = Vector4(0, 1, 0, 1);
		PackedVector4Array result = Geometry4D::closest_points_between_lines(line1_point, line1_direction, line2_point, line2_direction);
		CHECK_MESSAGE(result[0].is_equal_approx(Vector4(0, 0, 0, 0)), "Geometry4D closest_points_between_lines should work as expected.");
		CHECK_MESSAGE(result[1].is_equal_approx(Vector4(0, 0, 0, 0)), "Geometry4D closest_points_between_lines should work as expected.");
	}

	{
		Vector4 line1_point = Vector4(0, 0, 0, 0);
		Vector4 line1_direction = Vector4(1, 0, 1, 0);
		Vector4 line2_point = Vector4(1, 1, 1, 1);
		Vector4 line2_direction = Vector4(0, 1, 0, 1);
		PackedVector4Array result = Geometry4D::closest_points_between_lines(line1_point, line1_direction, line2_point, line2_direction);
		CHECK_MESSAGE(result[0].is_equal_approx(Vector4(1, 0, 1, 0)), "Geometry4D closest_points_between_lines should work as expected.");
		CHECK_MESSAGE(result[1].is_equal_approx(Vector4(1, 0, 1, 0)), "Geometry4D closest_points_between_lines should work as expected.");
	}
}
} // namespace TestGeometry4D
