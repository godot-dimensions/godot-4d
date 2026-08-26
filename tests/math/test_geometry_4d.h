#pragma once

#include "../../math/geometry_4d.h"

#include "tests/test_macros.h"

namespace TestGeometry4D {
// Computes an inverse metric cache holding just this one tetrahedron at index 0.
inline PackedFloat64Array compute_tetrahedron_inverse_metric_cache(const Vector4 &p_vert0, const Vector4 &p_vert1, const Vector4 &p_vert2, const Vector4 &p_vert3) {
	const Vector4 edge1 = p_vert1 - p_vert0;
	const Vector4 edge2 = p_vert2 - p_vert0;
	const Vector4 edge3 = p_vert3 - p_vert0;
	real_t inv_gram[6];
	const bool valid = Geometry4D::compute_inverse_metric_3x3(edge1.dot(edge1), edge1.dot(edge2), edge1.dot(edge3), edge2.dot(edge2), edge2.dot(edge3), edge3.dot(edge3), inv_gram);
	CHECK_MESSAGE(valid, "Geometry4D compute_inverse_metric_3x3 should succeed for a non-degenerate tetrahedron.");
	PackedFloat64Array cache;
	for (int64_t gram_index = 0; gram_index < 6; gram_index++) {
		cache.push_back(inv_gram[gram_index]);
	}
	return cache;
}

TEST_CASE("[Geometry4D] Compute Inverse Metric 3x3") {
	{
		// Degenerate tetrahedron: collinear edges have a singular metric.
		real_t inv_gram[6];
		const bool valid = Geometry4D::compute_inverse_metric_3x3(1, 2, 0, 4, 0, 1, inv_gram);
		CHECK_MESSAGE(!valid, "Geometry4D compute_inverse_metric_3x3 should fail for a singular metric.");
	}
	{
		// Degenerate tetrahedron: a zero-length edge has a non-positive diagonal.
		real_t inv_gram[6];
		const bool valid = Geometry4D::compute_inverse_metric_3x3(0, 0, 0, 4, 0, 4, inv_gram);
		CHECK_MESSAGE(!valid, "Geometry4D compute_inverse_metric_3x3 should fail for a metric with a zero diagonal.");
	}
}

TEST_CASE("[Geometry4D] Is Point Inside Tetrahedron Barycentric") {
	const Vector4 vert0 = Vector4(0, 0, 0, 0);
	const Vector4 vert1 = Vector4(2, 0, 0, 0);
	const Vector4 vert2 = Vector4(0, 2, 0, 0);
	const Vector4 vert3 = Vector4(0, 0, 2, 0);
	const PackedFloat64Array cache = compute_tetrahedron_inverse_metric_cache(vert0, vert1, vert2, vert3);
	CHECK_MESSAGE(Geometry4D::is_point_inside_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(0.5, 0.5, 0.5, 0), cache, 0), "Geometry4D is_point_inside_tetrahedron_barycentric should return true for a point inside the tetrahedron.");
	CHECK_MESSAGE(Geometry4D::is_point_inside_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(0.5, 0.5, 0.5, 9), cache, 0), "Geometry4D is_point_inside_tetrahedron_barycentric should return true when the projection lands inside the tetrahedron.");
	CHECK_MESSAGE(!Geometry4D::is_point_inside_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(-0.1, 0.5, 0.5, 0), cache, 0), "Geometry4D is_point_inside_tetrahedron_barycentric should return false for a point outside the tetrahedron.");
	CHECK_MESSAGE(!Geometry4D::is_point_inside_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(2, 2, 2, 0), cache, 0), "Geometry4D is_point_inside_tetrahedron_barycentric should return false for a point beyond the far cell of the tetrahedron.");
}

TEST_CASE("[Geometry4D] Get Nearest Point On Tetrahedron Barycentric") {
	const Vector4 vert0 = Vector4(0, 0, 0, 0);
	const Vector4 vert1 = Vector4(2, 0, 0, 0);
	const Vector4 vert2 = Vector4(0, 2, 0, 0);
	const Vector4 vert3 = Vector4(0, 0, 2, 0);
	const PackedFloat64Array cache = compute_tetrahedron_inverse_metric_cache(vert0, vert1, vert2, vert3);
	Vector4 nearest;
	real_t distance_squared = 0.0;
	bool proj_inside = false;
	Geometry4D::get_nearest_point_on_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(0.5, 0.5, 0.5, 0), cache, 0, nearest, distance_squared, proj_inside);
	CHECK_MESSAGE(nearest.is_equal_approx(Vector4(0.5, 0.5, 0.5, 0)), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return the point itself when inside the tetrahedron.");
	CHECK_MESSAGE(Math::is_zero_approx(distance_squared), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return zero distance for a point inside the tetrahedron.");
	CHECK_MESSAGE(proj_inside, "Geometry4D get_nearest_point_on_tetrahedron_barycentric should report the projection as inside the tetrahedron.");
	Geometry4D::get_nearest_point_on_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(0.5, 0.5, 0.5, 1), cache, 0, nearest, distance_squared, proj_inside);
	CHECK_MESSAGE(nearest.is_equal_approx(Vector4(0.5, 0.5, 0.5, 0)), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should project onto the inside of the tetrahedron.");
	CHECK_MESSAGE(distance_squared == doctest::Approx(1.0), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return the correct squared distance.");
	CHECK_MESSAGE(proj_inside, "Geometry4D get_nearest_point_on_tetrahedron_barycentric should report the projection as inside the tetrahedron.");
	Geometry4D::get_nearest_point_on_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(2, 2, 2, 0), cache, 0, nearest, distance_squared, proj_inside);
	CHECK_MESSAGE(nearest.is_equal_approx(Vector4(2.0 / 3.0, 2.0 / 3.0, 2.0 / 3.0, 0)), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return the nearest point on the far face.");
	CHECK_MESSAGE(distance_squared == doctest::Approx(16.0 / 3.0), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return the correct squared distance.");
	CHECK_MESSAGE(!proj_inside, "Geometry4D get_nearest_point_on_tetrahedron_barycentric should report the projection as outside the tetrahedron.");
	Geometry4D::get_nearest_point_on_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(2, 2, 0, 0), cache, 0, nearest, distance_squared, proj_inside);
	CHECK_MESSAGE(nearest.is_equal_approx(Vector4(1, 1, 0, 0)), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return the nearest point on the border edge.");
	CHECK_MESSAGE(distance_squared == doctest::Approx(2.0), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return the correct squared distance.");
	Geometry4D::get_nearest_point_on_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(4, 0, 0, 0), cache, 0, nearest, distance_squared, proj_inside);
	CHECK_MESSAGE(nearest.is_equal_approx(Vector4(2, 0, 0, 0)), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return the nearest vertex.");
	CHECK_MESSAGE(distance_squared == doctest::Approx(4.0), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return the correct squared distance.");
	Geometry4D::get_nearest_point_on_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(-1, -1, -1, 0), cache, 0, nearest, distance_squared, proj_inside);
	CHECK_MESSAGE(nearest.is_equal_approx(Vector4(0, 0, 0, 0)), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return the nearest vertex.");
	CHECK_MESSAGE(distance_squared == doctest::Approx(3.0), "Geometry4D get_nearest_point_on_tetrahedron_barycentric should return the correct squared distance.");
}

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

TEST_CASE("[Geometry4D] Closest Points Between Line And Segment") {
	{
		Vector4 line_point = Vector4(1, 2, 3, 4);
		Vector4 line_direction = Vector4(2, 0, 0, 0);
		Vector4 segment_a = Vector4(5, 6, 7, 5);
		Vector4 segment_b = Vector4(5, 6, 7, 9);
		PackedVector4Array result = Geometry4D::closest_points_between_line_and_segment(line_point, line_direction, segment_a, segment_b);
		CHECK_MESSAGE(result[0].is_equal_approx(Vector4(5, 2, 3, 4)), "Geometry4D closest_points_between_line_and_segment should work as expected.");
		CHECK_MESSAGE(result[1].is_equal_approx(Vector4(5, 6, 7, 5)), "Geometry4D closest_points_between_line_and_segment should work as expected.");
	}
}
} // namespace TestGeometry4D
