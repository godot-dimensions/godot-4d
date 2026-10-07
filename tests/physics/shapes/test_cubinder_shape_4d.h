#pragma once

#include "../../../physics/shapes/cubinder_shape_4d.h"

#include "tests/test_macros.h"

namespace TestCubinderShape4D {
TEST_CASE("[CubinderShape4D] Signed distance inside") {
	Ref<CubinderShape4D> cubinder;
	cubinder.instantiate();
	cubinder->set_radius(0.5f);
	cubinder->set_height(2.0f);
	cubinder->set_thickness(3.0f);
	// Inside, the signed distance is the largest (least negative) of the radial, vertical, and thickness signed distances,
	// and the nearest surface point only moves the point in that part of the shape.
	Vector4 surface_point;
	real_t signed_distance = cubinder->get_signed_distance_to_surface(Vector4(0.1, 0.2, 0.0, -0.3), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.4), "CubinderShape4D get_signed_distance_to_surface should return the negative distance to the curved surface when it is nearest.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.5, 0.2, 0.0, -0.3)), "CubinderShape4D get_signed_distance_to_surface should move the point radially when the curved surface is nearest.");
	signed_distance = cubinder->get_signed_distance_to_surface(Vector4(0.0, 0.8, 0.2, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.2), "CubinderShape4D get_signed_distance_to_surface should return the negative distance to the Y cap when it is nearest.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 1.0, 0.2, 0.0)), "CubinderShape4D get_signed_distance_to_surface should move the point along Y when the Y cap is nearest.");
	signed_distance = cubinder->get_signed_distance_to_surface(Vector4(0.0, 0.0, 0.2, -1.4), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.1), "CubinderShape4D get_signed_distance_to_surface should return the negative distance to the W cap when it is nearest.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 0.0, 0.2, -1.5)), "CubinderShape4D get_signed_distance_to_surface should move the point along W when the W cap is nearest.");
}

TEST_CASE("[CubinderShape4D] Signed distance on the surface") {
	Ref<CubinderShape4D> cubinder;
	cubinder.instantiate();
	cubinder->set_radius(0.5f);
	cubinder->set_height(2.0f);
	cubinder->set_thickness(3.0f);
	const Vector4 surface_points[] = {
		Vector4(0.3, 0.5, 0.4, 0.0),
		Vector4(0.0, -1.0, 0.2, 0.7),
		Vector4(0.1, 0.0, 0.0, -1.5),
		Vector4(0.0, 1.0, 0.0, 1.5),
		Vector4(-0.3, -1.0, -0.4, 1.5),
	};
	for (const Vector4 &point : surface_points) {
		Vector4 surface_point;
		const real_t signed_distance = cubinder->get_signed_distance_to_surface(point, &surface_point);
		CHECK_MESSAGE(signed_distance == doctest::Approx(0.0), "CubinderShape4D get_signed_distance_to_surface should return zero for points on the surface.");
		CHECK_MESSAGE(surface_point.is_equal_approx(point), "CubinderShape4D get_signed_distance_to_surface should return the same point for points on the surface.");
	}
}

TEST_CASE("[CubinderShape4D] Signed distance outside") {
	Ref<CubinderShape4D> cubinder;
	cubinder.instantiate();
	cubinder->set_radius(0.5f);
	cubinder->set_height(2.0f);
	cubinder->set_thickness(3.0f);
	// Outside of the curved surface but inside the cap planes, only the radial distance contributes.
	Vector4 surface_point;
	real_t signed_distance = cubinder->get_signed_distance_to_surface(Vector4(0.6, 0.95, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.1), "CubinderShape4D get_signed_distance_to_surface should return the positive distance to the curved surface when outside, even when the point is close to a cap's plane.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.5, 0.95, 0.0, 0.0)), "CubinderShape4D get_signed_distance_to_surface should return the nearest point on the curved surface when outside.");
	// Outside of one cap, only that cap's distance contributes.
	signed_distance = cubinder->get_signed_distance_to_surface(Vector4(0.1, 0.0, 0.0, 2.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.5), "CubinderShape4D get_signed_distance_to_surface should return the positive distance to the W cap when outside.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.1, 0.0, 0.0, 1.5)), "CubinderShape4D get_signed_distance_to_surface should return the nearest point on the W cap when outside.");
	// Outside of where two or three parts of the surface meet, each of their distances contributes.
	signed_distance = cubinder->get_signed_distance_to_surface(Vector4(0.0, 1.3, 0.0, -1.9), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.5), "CubinderShape4D get_signed_distance_to_surface should return the distance to the edge where the caps meet when outside of it.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 1.0, 0.0, -1.5)), "CubinderShape4D get_signed_distance_to_surface should return the nearest point on the edge where the caps meet when outside of it.");
	signed_distance = cubinder->get_signed_distance_to_surface(Vector4(0.42, 1.4, 0.56, -1.9), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.6), "CubinderShape4D get_signed_distance_to_surface should return the distance to the circle where all three parts meet when outside of it.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.3, 1.0, 0.4, -1.5)), "CubinderShape4D get_signed_distance_to_surface should return the nearest point on the circle where all three parts meet when outside of it.");
	// Same as the CylinderShape4D counterexample, with the default thickness.
	cubinder->set_thickness(2.0f);
	signed_distance = cubinder->get_signed_distance_to_surface(Vector4(0.6, 0.95, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.1), "CubinderShape4D get_signed_distance_to_surface should return the positive distance to the curved surface when outside, even when the point is close to a cap's plane.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.5, 0.95, 0.0, 0.0)), "CubinderShape4D get_signed_distance_to_surface should return the nearest point on the curved surface when outside.");
}

TEST_CASE("[CubinderShape4D] Signed distance on the axes") {
	Ref<CubinderShape4D> cubinder;
	cubinder.instantiate();
	cubinder->set_radius(0.5f);
	cubinder->set_height(2.0f);
	cubinder->set_thickness(3.0f);
	// On the YW plane, the radial direction is undefined, so any point on the curved surface with the same Y and W is nearest.
	Vector4 surface_point;
	real_t signed_distance = cubinder->get_signed_distance_to_surface(Vector4(0.0, 0.2, 0.0, 0.3), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.5), "CubinderShape4D get_signed_distance_to_surface should return the negative radius on the YW plane when the curved surface is nearest.");
	CHECK_MESSAGE(surface_point.y == doctest::Approx(0.2), "CubinderShape4D get_signed_distance_to_surface should keep Y on the YW plane when the curved surface is nearest.");
	CHECK_MESSAGE(surface_point.w == doctest::Approx(0.3), "CubinderShape4D get_signed_distance_to_surface should keep W on the YW plane when the curved surface is nearest.");
	CHECK_MESSAGE(Vector2(surface_point.x, surface_point.z).length() == doctest::Approx(0.5), "CubinderShape4D get_signed_distance_to_surface should return a point on the curved surface on the YW plane when the curved surface is nearest.");
	signed_distance = cubinder->get_signed_distance_to_surface(Vector4(0.0, 0.0, 0.0, 3.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(1.5), "CubinderShape4D get_signed_distance_to_surface should return the positive distance to the W cap on the W axis when outside.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 0.0, 0.0, 1.5)), "CubinderShape4D get_signed_distance_to_surface should return the center of the W cap on the W axis when outside.");
}

TEST_CASE("[CubinderShape4D] Signed distance consistency") {
	Ref<CubinderShape4D> cubinder;
	cubinder.instantiate();
	cubinder->set_radius(0.5f);
	cubinder->set_height(2.0f);
	cubinder->set_thickness(3.0f);
	// A grid of points inside, on, and outside of the curved surface, the caps, and where they meet.
	const real_t coordinates[] = { -2.0, -1.5, -0.9, -0.4, 0.0, 0.3, 0.6, 1.2, 1.7 };
	for (const real_t x : coordinates) {
		for (const real_t y : coordinates) {
			for (const real_t z : coordinates) {
				for (const real_t w : coordinates) {
					const Vector4 point = Vector4(x, y, z, w);
					INFO("Point: ", point);
					Vector4 surface_point;
					const real_t signed_distance = cubinder->get_signed_distance_to_surface(point, &surface_point);
					CHECK_MESSAGE(cubinder->get_signed_distance_to_surface(point) == signed_distance, "CubinderShape4D get_signed_distance_to_surface should not depend on whether the surface point is requested.");
					if (signed_distance < -CMP_EPSILON) {
						CHECK_MESSAGE(cubinder->has_point(point), "CubinderShape4D get_signed_distance_to_surface should be negative only for points inside the shape.");
					} else if (signed_distance > CMP_EPSILON) {
						CHECK_FALSE_MESSAGE(cubinder->has_point(point), "CubinderShape4D get_signed_distance_to_surface should be positive only for points outside the shape.");
					}
					if (signed_distance > 0.0f) {
						const Vector4 nearest_point = cubinder->get_nearest_point(point);
						CHECK_MESSAGE(signed_distance == doctest::Approx(point.distance_to(nearest_point)), "CubinderShape4D get_signed_distance_to_surface should return the distance to the nearest point when outside.");
						CHECK_MESSAGE(surface_point.is_equal_approx(nearest_point), "CubinderShape4D get_signed_distance_to_surface should return the nearest point as the surface point when outside.");
					} else {
						CHECK_MESSAGE(point.distance_to(surface_point) == doctest::Approx(-signed_distance), "CubinderShape4D get_signed_distance_to_surface should return a surface point at the signed distance when inside.");
					}
					CHECK_MESSAGE(cubinder->get_signed_distance_to_surface(surface_point) == doctest::Approx(0.0), "CubinderShape4D get_signed_distance_to_surface should return a surface point that is on the surface.");
				}
			}
		}
	}
}
} // namespace TestCubinderShape4D
