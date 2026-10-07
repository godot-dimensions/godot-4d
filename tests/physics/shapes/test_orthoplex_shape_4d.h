#pragma once

#include "../../../physics/shapes/orthoplex_shape_4d.h"

#include "tests/test_macros.h"

namespace TestOrthoplexShape4D {
TEST_CASE("[OrthoplexShape4D] Get Nearest Point") {
	Ref<OrthoplexShape4D> orthoplex;
	orthoplex.instantiate();
	// Default size is (1, 1, 1, 1), which means a "radius" of 0.5.
	// This means the adding up the output's absolute values should be less than or equal to 0.5.
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.0, 0.0, 0.0, 0.0)) == Vector4(0.0, 0.0, 0.0, 0.0), "OrthoplexShape4D get_nearest_point should return the same point when the point is inside the shape.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.25, 0.25, 0.25, 0.25)) == Vector4(0.125, 0.125, 0.125, 0.125), "OrthoplexShape4D get_nearest_point should return the nearest point when the point is outside the shape.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.5, 0.5, 0.5, 0.5)) == Vector4(0.125, 0.125, 0.125, 0.125), "OrthoplexShape4D get_nearest_point should return the nearest point when the point is outside the shape.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(1.0, 1.0, 1.0, 0.0)).is_equal_approx(Vector4(1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0, 0.0)), "OrthoplexShape4D get_nearest_point should return a point on the triangle face when one axis is zero.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.6, 0.3, 0.0, 0.0)).is_equal_approx(Vector4(0.4, 0.1, 0.0, 0.0)), "OrthoplexShape4D get_nearest_point should return a point on an edge when two axes are zero.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(1.7, 0.1, 0.2, 0.3)).is_equal_approx(Vector4(0.5, 0.0, 0.0, 0.0)), "OrthoplexShape4D get_nearest_point should return zero for the smallest axes when they make up less than a quarter of the total.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(-0.1, -0.2, -1.2, 1.3)).is_equal_approx(Vector4(0.0, 0.0, -0.2, 0.3)), "OrthoplexShape4D get_nearest_point should work for negative values.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(-0.3, 1.15, -1.35, 0.05)).is_equal_approx(Vector4(0.0, 0.15, -0.35, 0.0)), "OrthoplexShape4D get_nearest_point should work for negative values.");
	// Test when the orthoplex is not at the default size. The nearest point must be the Euclidean nearest point,
	// which is not the same as the nearest point in a space scaled by the size, since the scaling is non-uniform.
	orthoplex->set_size(Vector4(3, 4, 5, 2));
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.0, 0.0, 0.0, 0.0)) == Vector4(0.0, 0.0, 0.0, 0.0), "OrthoplexShape4D get_nearest_point should return the same point when the point is inside the shape.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(0.75, 1.0, 1.25, 0.5)).is_equal_approx(Vector4(1107.0 / 3076.0, 544.0 / 769.0, 3125.0 / 3076.0, 0.0)), "OrthoplexShape4D get_nearest_point should return the Euclidean nearest point, not the nearest point in scaled space.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(1.5, 2.0, 2.5, 1.0)).is_equal_approx(Vector4(0.0, 32.0 / 41.0, 125.0 / 82.0, 0.0)), "OrthoplexShape4D get_nearest_point should return a point on an edge when the smaller axes reach zero.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(3.0, 4.0, 5.0, 0.0)).is_equal_approx(Vector4(0.0, 14.0 / 41.0, 85.0 / 41.0, 0.0)), "OrthoplexShape4D get_nearest_point should return a point on an edge when one axis is zero.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(-4.3, 0.1, -0.2, 0.3)).is_equal_approx(Vector4(-1.5, 0.0, 0.0, 0.0)), "OrthoplexShape4D get_nearest_point should return zero for the smallest axes when they make up less than a quarter of the total.");
	CHECK_MESSAGE(orthoplex->get_nearest_point(Vector4(-6.0, -7.0, -8.0, -9.0)).is_equal_approx(Vector4(0.0, -2.0 / 41.0, -100.0 / 41.0, 0.0)), "OrthoplexShape4D get_nearest_point should work for negative values.");
}

TEST_CASE("[OrthoplexShape4D] Get Support Point") {
	Ref<OrthoplexShape4D> orthoplex;
	orthoplex.instantiate();
	orthoplex->set_size(Vector4(4, 1, 1, 1));
	CHECK_MESSAGE(orthoplex->get_support_point(Vector4(0.5, 1.0, 0.0, 0.0)) == Vector4(2.0, 0.0, 0.0, 0.0), "OrthoplexShape4D get_support_point should account for the size of each axis, not just the direction.");
	orthoplex->set_size(Vector4(3, 4, 5, 2));
	const Vector4 half_extents = orthoplex->get_half_extents();
	const Vector4 directions[] = {
		Vector4(1.0, 1.0, 1.0, 1.0),
		Vector4(-1.0, 0.5, 0.5, -2.0),
		Vector4(-1.0, 0.5, 0.3, 1.2),
		Vector4(0.2, -1.0, 0.5, 1.5),
		Vector4(0.0, 0.0, 0.0, -1.0),
	};
	const Vector4 expected_supports[] = {
		Vector4(0.0, 0.0, 2.5, 0.0),
		Vector4(0.0, 0.0, 0.0, -1.0),
		Vector4(-1.5, 0.0, 0.0, 0.0),
		Vector4(0.0, -2.0, 0.0, 0.0),
		Vector4(0.0, 0.0, 0.0, -1.0),
	};
	for (int i = 0; i < 5; i++) {
		const Vector4 support = orthoplex->get_support_point(directions[i]);
		CHECK_MESSAGE(support == expected_supports[i], "OrthoplexShape4D get_support_point should return the vertex furthest along the direction.");
		for (int axis = 0; axis < 4; axis++) {
			for (int sign = -1; sign <= 1; sign += 2) {
				Vector4 vertex = Vector4();
				vertex[axis] = sign * half_extents[axis];
				CHECK_MESSAGE(support.dot(directions[i]) >= vertex.dot(directions[i]), "OrthoplexShape4D get_support_point should be at least as far along the direction as every vertex.");
			}
		}
	}
}

TEST_CASE("[OrthoplexShape4D] Has Point") {
	Ref<OrthoplexShape4D> orthoplex;
	orthoplex.instantiate();
	// Default size is (1, 1, 1, 1), so the vertices are at plus or minus 0.5 on each axis.
	const Vector4 default_inside_points[] = {
		Vector4(0.0, 0.0, 0.0, 0.0),
		Vector4(0.1, -0.1, 0.1, -0.1),
		Vector4(0.5, 0.0, 0.0, 0.0),
		Vector4(0.0, 0.0, 0.0, -0.5),
		Vector4(0.25, -0.25, 0.0, 0.0),
		Vector4(0.125, 0.125, -0.125, 0.125),
	};
	for (const Vector4 &point : default_inside_points) {
		CHECK_MESSAGE(orthoplex->has_point(point), "OrthoplexShape4D has_point should return true for points inside or on the surface of the shape.");
		CHECK_MESSAGE(orthoplex->get_nearest_point(point) == point, "OrthoplexShape4D has_point should be consistent with get_nearest_point.");
	}
	const Vector4 default_outside_points[] = {
		Vector4(0.75, 0.0, 0.0, 0.0),
		Vector4(0.25, 0.25, 0.25, 0.25),
		Vector4(0.3, 0.3, 0.0, 0.0),
		Vector4(0.0, 0.0, -0.51, 0.0),
	};
	for (const Vector4 &point : default_outside_points) {
		CHECK_MESSAGE(!orthoplex->has_point(point), "OrthoplexShape4D has_point should return false for points outside the shape.");
		CHECK_MESSAGE(orthoplex->get_nearest_point(point) != point, "OrthoplexShape4D has_point should be consistent with get_nearest_point.");
	}
	// Test when the orthoplex is not at the default size.
	orthoplex->set_size(Vector4(3, 4, 5, 2));
	const Vector4 sized_inside_points[] = {
		Vector4(0.0, 0.0, 0.0, 0.0),
		Vector4(0.75, 0.0, 0.0, 0.0),
		Vector4(0.3, -0.4, 0.5, -0.2),
		Vector4(1.5, 0.0, 0.0, 0.0),
		Vector4(0.0, -2.0, 0.0, 0.0),
		Vector4(0.75, 1.0, 0.0, 0.0),
		Vector4(0.375, -0.5, 0.625, -0.25),
	};
	for (const Vector4 &point : sized_inside_points) {
		CHECK_MESSAGE(orthoplex->has_point(point), "OrthoplexShape4D has_point should return true for points inside or on the surface of the shape.");
		CHECK_MESSAGE(orthoplex->get_nearest_point(point) == point, "OrthoplexShape4D has_point should be consistent with get_nearest_point.");
	}
	const Vector4 sized_outside_points[] = {
		Vector4(1.6, 0.0, 0.0, 0.0),
		Vector4(0.0, 0.0, 0.0, 1.01),
		Vector4(0.75, 1.0, 1.25, 0.5),
		Vector4(-6.0, -7.0, -8.0, -9.0),
	};
	for (const Vector4 &point : sized_outside_points) {
		CHECK_MESSAGE(!orthoplex->has_point(point), "OrthoplexShape4D has_point should return false for points outside the shape.");
		CHECK_MESSAGE(orthoplex->get_nearest_point(point) != point, "OrthoplexShape4D has_point should be consistent with get_nearest_point.");
	}
}

TEST_CASE("[OrthoplexShape4D] Get Signed Distance To Surface") {
	Ref<OrthoplexShape4D> orthoplex;
	orthoplex.instantiate();
	Vector4 surface_point;
	// Default size is (1, 1, 1, 1), so each facet is 0.25 away from the center.
	CHECK_MESSAGE(orthoplex->get_signed_distance_to_surface(Vector4(0.0, 0.0, 0.0, 0.0), &surface_point) == doctest::Approx(-0.25), "OrthoplexShape4D get_signed_distance_to_surface should return the negative distance to the nearest facet for the center.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.125, 0.125, 0.125, 0.125)), "OrthoplexShape4D get_signed_distance_to_surface should return the nearest point on a facet for the center.");
	CHECK_MESSAGE(orthoplex->get_signed_distance_to_surface(Vector4(0.1, 0.0, 0.0, 0.0), &surface_point) == doctest::Approx(-0.2), "OrthoplexShape4D get_signed_distance_to_surface should return a negative distance for points inside the shape.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.2, 0.1, 0.1, 0.1)), "OrthoplexShape4D get_signed_distance_to_surface should return the nearest point on the facet for points inside the shape.");
	CHECK_MESSAGE(orthoplex->get_signed_distance_to_surface(Vector4(-0.1, 0.2, 0.0, 0.0), &surface_point) == doctest::Approx(-0.1), "OrthoplexShape4D get_signed_distance_to_surface should work for negative values.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(-0.15, 0.25, 0.05, 0.05)), "OrthoplexShape4D get_signed_distance_to_surface should work for negative values.");
	CHECK_MESSAGE(orthoplex->get_signed_distance_to_surface(Vector4(0.5, 0.5, 0.5, 0.5), &surface_point) == doctest::Approx(0.75), "OrthoplexShape4D get_signed_distance_to_surface should return a positive distance for points outside the shape.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.125, 0.125, 0.125, 0.125)), "OrthoplexShape4D get_signed_distance_to_surface should return the nearest point for points outside the shape.");
	// Test when the orthoplex is not at the default size. All facets are 30 / sqrt(1669) away from the center.
	orthoplex->set_size(Vector4(3, 4, 5, 2));
	CHECK_MESSAGE(orthoplex->get_signed_distance_to_surface(Vector4(0.0, 0.0, 0.0, 0.0), &surface_point) == doctest::Approx(-30.0 / Math::sqrt(1669.0)), "OrthoplexShape4D get_signed_distance_to_surface should return the negative distance to the nearest facet for the center.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(600.0, 450.0, 360.0, 900.0) / 1669.0), "OrthoplexShape4D get_signed_distance_to_surface should return the nearest point on a facet for the center.");
	CHECK_MESSAGE(orthoplex->get_signed_distance_to_surface(Vector4(0.3, -0.4, 0.5, -0.2), &surface_point) == doctest::Approx(-6.0 / Math::sqrt(1669.0)), "OrthoplexShape4D get_signed_distance_to_surface should return a negative distance for points inside the shape.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.3, -0.4, 0.5, -0.2) + Vector4(120.0, -90.0, 72.0, -180.0) / 1669.0), "OrthoplexShape4D get_signed_distance_to_surface should return the nearest point on the facet for points inside the shape.");
	CHECK_MESSAGE(orthoplex->get_signed_distance_to_surface(Vector4(0.75, 1.0, 0.0, 0.0), &surface_point) == doctest::Approx(0.0), "OrthoplexShape4D get_signed_distance_to_surface should return zero for points on the surface.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.75, 1.0, 0.0, 0.0)), "OrthoplexShape4D get_signed_distance_to_surface should return the same point for points on the surface.");
	// Check that the signed distance is consistent with has_point and get_nearest_point for many points, for both sizes.
	const real_t coordinates[] = { -2.6, -0.7, 0.0, 0.3, 1.1 };
	for (int size_index = 0; size_index < 2; size_index++) {
		orthoplex->set_size(size_index == 0 ? Vector4(1, 1, 1, 1) : Vector4(3, 4, 5, 2));
		const Vector4 half_extents = orthoplex->get_half_extents();
		for (const real_t x : coordinates) {
			for (const real_t y : coordinates) {
				for (const real_t z : coordinates) {
					for (const real_t w : coordinates) {
						const Vector4 point = Vector4(x, y, z, w);
						const real_t signed_distance = orthoplex->get_signed_distance_to_surface(point, &surface_point);
						CHECK_MESSAGE((signed_distance <= 0.0f) == orthoplex->has_point(point), "OrthoplexShape4D get_signed_distance_to_surface should be negative or zero exactly when has_point is true.");
						const Vector4 abs_scaled_surface_point = surface_point.abs() / half_extents;
						CHECK_MESSAGE(abs_scaled_surface_point.x + abs_scaled_surface_point.y + abs_scaled_surface_point.z + abs_scaled_surface_point.w == doctest::Approx(1.0), "OrthoplexShape4D get_signed_distance_to_surface should return a point on the surface.");
						CHECK_MESSAGE(Math::abs(signed_distance) == doctest::Approx(point.distance_to(surface_point)), "OrthoplexShape4D get_signed_distance_to_surface should return the distance to the returned surface point.");
						if (signed_distance > 0.0f) {
							const Vector4 nearest_point = orthoplex->get_nearest_point(point);
							CHECK_MESSAGE(signed_distance == doctest::Approx(point.distance_to(nearest_point)), "OrthoplexShape4D get_signed_distance_to_surface should return the distance to the nearest point for points outside the shape.");
							CHECK_MESSAGE(surface_point.is_equal_approx(nearest_point), "OrthoplexShape4D get_signed_distance_to_surface should return the nearest point for points outside the shape.");
						}
					}
				}
			}
		}
	}
}

TEST_CASE("[OrthoplexShape4D] Raycast") {
	Ref<OrthoplexShape4D> orthoplex;
	orthoplex.instantiate();
	// Default size is (1, 1, 1, 1), so the vertices are at plus or minus 0.5 on each axis.
	// This ray hits a vertex, so the normal can be the normal of any facet touching that vertex.
	Dictionary result = orthoplex->raycast_intersects(Vector4(2, 0, 0, 0), Vector4(-1, 0, 0, 0));
	CHECK_MESSAGE((bool)result["hit"], "OrthoplexShape4D raycast_intersects should hit the vertex when pointing at it.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.5), "OrthoplexShape4D raycast_intersects should return the distance to the vertex.");
	CHECK_MESSAGE(Vector4(result["point"]).is_equal_approx(Vector4(0.5, 0, 0, 0)), "OrthoplexShape4D raycast_intersects should return the vertex as the hit point.");
	Vector4 normal = result["normal"];
	CHECK_MESSAGE(normal.x == doctest::Approx(0.5), "OrthoplexShape4D raycast_intersects should return the normal of a facet touching the vertex.");
	CHECK_MESSAGE(normal.abs().is_equal_approx(Vector4(0.5, 0.5, 0.5, 0.5)), "OrthoplexShape4D raycast_intersects should return the normal of a facet touching the vertex.");
	result = orthoplex->raycast_intersects(Vector4(2, 0.1, 0.05, 0.02), Vector4(-1, 0, 0, 0));
	CHECK_MESSAGE((bool)result["hit"], "OrthoplexShape4D raycast_intersects should hit a facet.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.67), "OrthoplexShape4D raycast_intersects should return the distance to the facet.");
	CHECK_MESSAGE(Vector4(result["point"]).is_equal_approx(Vector4(0.33, 0.1, 0.05, 0.02)), "OrthoplexShape4D raycast_intersects should return the hit point on the facet.");
	CHECK_MESSAGE(Vector4(result["normal"]).is_equal_approx(Vector4(0.5, 0.5, 0.5, 0.5)), "OrthoplexShape4D raycast_intersects should return the facet normal.");
	result = orthoplex->raycast_intersects(Vector4(0.1, -2, -0.05, 0.15), Vector4(0, 1, 0, 0));
	CHECK_MESSAGE((bool)result["hit"], "OrthoplexShape4D raycast_intersects should hit a facet.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.8), "OrthoplexShape4D raycast_intersects should return the distance to the facet.");
	CHECK_MESSAGE(Vector4(result["point"]).is_equal_approx(Vector4(0.1, -0.2, -0.05, 0.15)), "OrthoplexShape4D raycast_intersects should return the hit point on the facet.");
	CHECK_MESSAGE(Vector4(result["normal"]).is_equal_approx(Vector4(0.5, -0.5, -0.5, 0.5)), "OrthoplexShape4D raycast_intersects should return the facet normal for negative values.");
	// Rays that miss the shape.
	result = orthoplex->raycast_intersects(Vector4(2, 0, 0, 0), Vector4(1, 0, 0, 0));
	CHECK_MESSAGE(!(bool)result["hit"], "OrthoplexShape4D raycast_intersects should not hit when pointing away.");
	result = orthoplex->raycast_intersects(Vector4(2, 2, 0, 0), Vector4(0, 0, 0, 1));
	CHECK_MESSAGE(!(bool)result["hit"], "OrthoplexShape4D raycast_intersects should not hit when missing the shape.");
	result = orthoplex->raycast_intersects(Vector4(2, 0.1, 0.05, 0.02), Vector4(-1, 0, 0, 0), 1.0);
	CHECK_MESSAGE(!(bool)result["hit"], "OrthoplexShape4D raycast_intersects should not hit when the shape is further than the max distance.");
	// Rays starting inside the shape.
	result = orthoplex->raycast_intersects(Vector4(0, 0.1, -0.05, 0.05), Vector4(1, 0, 0, 0));
	CHECK_MESSAGE((bool)result["hit"], "OrthoplexShape4D raycast_intersects should hit the exit facet when starting inside.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(0.3), "OrthoplexShape4D raycast_intersects should return the distance to the exit facet when starting inside.");
	CHECK_MESSAGE(Vector4(result["normal"]).is_equal_approx(Vector4(0.5, 0.5, -0.5, 0.5)), "OrthoplexShape4D raycast_intersects should return the exit facet normal when starting inside.");
	result = orthoplex->raycast_intersects(Vector4(0.1, 0, 0, 0), Vector4(1, 0, 0, 0), Math_INF, true);
	CHECK_MESSAGE((bool)result["hit"], "OrthoplexShape4D raycast_intersects should hit when starting inside and inside_is_zero is true.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(0.0), "OrthoplexShape4D raycast_intersects should return zero distance when starting inside and inside_is_zero is true.");
	result = orthoplex->raycast_intersects(Vector4(0.75, 0, 0, 0), Vector4(1, 0, 0, 0), Math_INF, true);
	CHECK_MESSAGE(!(bool)result["hit"], "OrthoplexShape4D raycast_intersects should not treat points outside the vertices as inside.");
	// Test when the orthoplex is not at the default size. The facet normals are normalize(signs / half_extents).
	orthoplex->set_size(Vector4(3, 4, 5, 2));
	result = orthoplex->raycast_intersects(Vector4(0.3, -0.4, 0.5, -5), Vector4(0, 0, 0, 1));
	CHECK_MESSAGE((bool)result["hit"], "OrthoplexShape4D raycast_intersects should hit a facet.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(4.6), "OrthoplexShape4D raycast_intersects should return the distance to the facet.");
	CHECK_MESSAGE(Vector4(result["point"]).is_equal_approx(Vector4(0.3, -0.4, 0.5, -0.4)), "OrthoplexShape4D raycast_intersects should return the hit point on the facet.");
	CHECK_MESSAGE(Vector4(result["normal"]).is_equal_approx(Vector4(20, -15, 12, -30) / Math::sqrt(1669.0)), "OrthoplexShape4D raycast_intersects should return the facet normal.");
	result = orthoplex->raycast_intersects(Vector4(1.6, 1.4, -1.5, 1.2), Vector4(-0.5, -0.5, 0.5, -0.5));
	CHECK_MESSAGE((bool)result["hit"], "OrthoplexShape4D raycast_intersects should hit a facet with a diagonal ray.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(2.0), "OrthoplexShape4D raycast_intersects should return the distance to the facet with a diagonal ray.");
	CHECK_MESSAGE(Vector4(result["point"]).is_equal_approx(Vector4(0.6, 0.4, -0.5, 0.2)), "OrthoplexShape4D raycast_intersects should return the hit point on the facet with a diagonal ray.");
	CHECK_MESSAGE(Vector4(result["normal"]).is_equal_approx(Vector4(20, 15, -12, 30) / Math::sqrt(1669.0)), "OrthoplexShape4D raycast_intersects should return the facet normal with a diagonal ray.");
	result = orthoplex->raycast_intersects(Vector4(0.75, 0, 0, 0), Vector4(1, 0, 0, 0), Math_INF, true);
	CHECK_MESSAGE((bool)result["hit"], "OrthoplexShape4D raycast_intersects should hit when starting inside and inside_is_zero is true.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(0.0), "OrthoplexShape4D raycast_intersects should return zero distance when starting inside and inside_is_zero is true.");
}
} // namespace TestOrthoplexShape4D
