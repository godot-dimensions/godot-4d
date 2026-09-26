#pragma once

#include "../../math/geometry_4d.h"

#include "tests/test_macros.h"

namespace TestGeometry4D {
// Computes an inverse metric cache holding just this one tetrahedron at index 0.
inline PackedFloat64Array compute_tetrahedron_inverse_metric_cache(const Vector4 &p_vert0, const Vector4 &p_vert1, const Vector4 &p_vert2, const Vector4 &p_vert3) {
	const Vector4 edge1 = p_vert1 - p_vert0;
	const Vector4 edge2 = p_vert2 - p_vert0;
	const Vector4 edge3 = p_vert3 - p_vert0;
	double inv_gram[6];
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
		double inv_gram[6];
		const bool valid = Geometry4D::compute_inverse_metric_3x3(1, 2, 0, 4, 0, 1, inv_gram);
		CHECK_MESSAGE(!valid, "Geometry4D compute_inverse_metric_3x3 should fail for a singular metric.");
	}
	{
		// Degenerate tetrahedron: a zero-length edge has a non-positive diagonal.
		double inv_gram[6];
		const bool valid = Geometry4D::compute_inverse_metric_3x3(0, 0, 0, 4, 0, 4, inv_gram);
		CHECK_MESSAGE(!valid, "Geometry4D compute_inverse_metric_3x3 should fail for a metric with a zero diagonal.");
	}
}

TEST_CASE("[Geometry4D] Inverse metric of a thin tetrahedron") {
	// A tetrahedron from a slab cell 0.00115 thick, 0.87 wide, and 10 long, with the pivot at the far end,
	// so that its three edges are nearly parallel. It is perfectly valid geometry, with a normalized metric
	// determinant around 1e-10, and must not be rejected as degenerate or the slab gets holes in collision.
	const Vector4 vert0 = Vector4(0, 0, 0, 0);
	const Vector4 vert1 = Vector4(10, 0, 0, 0);
	const Vector4 vert2 = Vector4(10, 0.87, 0, 0);
	const Vector4 vert3 = Vector4(10, 0, 0.00115, 0);
	const PackedFloat64Array cache = compute_tetrahedron_inverse_metric_cache(vert0, vert1, vert2, vert3);
	REQUIRE(cache.size() == 6);
	// The inverse must still be accurate: a point inside the tetrahedron lifted along W projects back onto itself.
	const Vector4 inside = Vector4(5, 0.2, 0.0002, 0);
	Vector4 nearest;
	real_t distance_squared = 0.0;
	bool proj_inside = false;
	Geometry4D::get_nearest_point_on_tetrahedron_barycentric(vert0, vert1, vert2, vert3, inside + Vector4(0, 0, 0, 1), cache, 0, nearest, distance_squared, proj_inside);
	CHECK_MESSAGE(proj_inside, "The lifted point must project inside the thin tetrahedron.");
	CHECK_MESSAGE(nearest.is_equal_approx(inside), "The nearest point on the thin tetrahedron must be accurate despite its poor conditioning.");
	CHECK_MESSAGE(Math::is_equal_approx(distance_squared, (real_t)1.0), "The distance to the thin tetrahedron must be accurate despite its poor conditioning.");
}

TEST_CASE("[Geometry4D] Degenerate tetrahedron marked in the cache uses its triangle borders") {
	// A cache entry of NaN marks a tetrahedron whose inverse metric could not be computed. Queries must treat
	// it as having no interior, and still find the nearest point on its triangle borders.
	const Vector4 vert0 = Vector4(0, 0, 0, 0);
	const Vector4 vert1 = Vector4(2, 0, 0, 0);
	const Vector4 vert2 = Vector4(0, 2, 0, 0);
	const Vector4 vert3 = Vector4(1, 1, 0, 0); // Coplanar with the others, so the tetrahedron is flat.
	PackedFloat64Array cache;
	for (int i = 0; i < 6; i++) {
		cache.push_back(NAN);
	}
	CHECK_MESSAGE(!Geometry4D::is_point_inside_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(0.5, 0.5, 0, 0), cache, 0), "A degenerate tetrahedron has no interior.");
	Vector4 nearest;
	real_t distance_squared = 0.0;
	bool proj_inside = true;
	Geometry4D::get_nearest_point_on_tetrahedron_barycentric(vert0, vert1, vert2, vert3, Vector4(0.5, 0.5, 3, 0), cache, 0, nearest, distance_squared, proj_inside);
	CHECK_MESSAGE(!proj_inside, "A degenerate tetrahedron has no interior to project into.");
	CHECK_MESSAGE(nearest.is_equal_approx(Vector4(0.5, 0.5, 0, 0)), "The nearest point must come from the degenerate tetrahedron's triangle borders.");
	CHECK_MESSAGE(Math::is_equal_approx(distance_squared, (real_t)9.0), "The distance must come from the degenerate tetrahedron's triangle borders.");
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
