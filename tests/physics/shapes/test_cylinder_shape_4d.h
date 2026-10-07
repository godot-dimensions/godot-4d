#pragma once

#include "../../../physics/shapes/cylinder_shape_4d.h"

#include "tests/test_macros.h"

namespace TestCylinderShape4D {
TEST_CASE("[CylinderShape4D] Signed distance inside") {
	Ref<CylinderShape4D> cylinder;
	cylinder.instantiate();
	cylinder->set_radius(0.5f);
	cylinder->set_height(2.0f);
	// Inside, the signed distance is the largest (least negative) of the radial and vertical signed distances,
	// and the nearest surface point only moves the point in that part of the shape.
	Vector4 surface_point;
	real_t signed_distance = cylinder->get_signed_distance_to_surface(Vector4(0.1, 0.2, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.4), "CylinderShape4D get_signed_distance_to_surface should return the negative distance to the curved surface when it is nearest.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.5, 0.2, 0.0, 0.0)), "CylinderShape4D get_signed_distance_to_surface should move the point radially when the curved surface is nearest.");
	signed_distance = cylinder->get_signed_distance_to_surface(Vector4(0.1, 0.9, 0.1, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.1), "CylinderShape4D get_signed_distance_to_surface should return the negative distance to the cap when it is nearest.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.1, 1.0, 0.1, 0.0)), "CylinderShape4D get_signed_distance_to_surface should move the point vertically when the cap is nearest.");
}

TEST_CASE("[CylinderShape4D] Signed distance on the surface") {
	Ref<CylinderShape4D> cylinder;
	cylinder.instantiate();
	cylinder->set_radius(0.5f);
	cylinder->set_height(2.0f);
	const Vector4 surface_points[] = {
		Vector4(0.3, 0.5, 0.4, 0.0),
		Vector4(0.0, -0.2, 0.0, 0.5),
		Vector4(0.0, 1.0, 0.0, 0.0),
		Vector4(0.1, -1.0, 0.2, -0.2),
		Vector4(0.0, 1.0, -0.3, 0.4),
	};
	for (const Vector4 &point : surface_points) {
		Vector4 surface_point;
		const real_t signed_distance = cylinder->get_signed_distance_to_surface(point, &surface_point);
		CHECK_MESSAGE(signed_distance == doctest::Approx(0.0), "CylinderShape4D get_signed_distance_to_surface should return zero for points on the surface.");
		CHECK_MESSAGE(surface_point.is_equal_approx(point), "CylinderShape4D get_signed_distance_to_surface should return the same point for points on the surface.");
	}
}

TEST_CASE("[CylinderShape4D] Signed distance outside") {
	Ref<CylinderShape4D> cylinder;
	cylinder.instantiate();
	cylinder->set_radius(0.5f);
	cylinder->set_height(2.0f);
	// Outside of the curved surface but inside the cap planes, only the radial distance contributes.
	Vector4 surface_point;
	real_t signed_distance = cylinder->get_signed_distance_to_surface(Vector4(0.6, 0.95, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.1), "CylinderShape4D get_signed_distance_to_surface should return the positive distance to the curved surface when outside, even when the point is close to a cap's plane.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.5, 0.95, 0.0, 0.0)), "CylinderShape4D get_signed_distance_to_surface should return the nearest point on the curved surface when outside.");
	// Outside of a cap but inside the curved surface, only the vertical distance contributes.
	signed_distance = cylinder->get_signed_distance_to_surface(Vector4(0.0, 1.5, 0.2, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.5), "CylinderShape4D get_signed_distance_to_surface should return the positive distance to the cap when outside.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 1.0, 0.2, 0.0)), "CylinderShape4D get_signed_distance_to_surface should return the nearest point on the cap when outside.");
	// Outside of the rim where the cap meets the curved surface, both distances contribute.
	signed_distance = cylinder->get_signed_distance_to_surface(Vector4(0.0, 1.4, 0.8, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.5), "CylinderShape4D get_signed_distance_to_surface should return the distance to the rim when outside of the rim.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 1.0, 0.5, 0.0)), "CylinderShape4D get_signed_distance_to_surface should return the nearest point on the rim when outside of the rim.");
	signed_distance = cylinder->get_signed_distance_to_surface(Vector4(0.0, -1.4, 0.48, 0.64), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.5), "CylinderShape4D get_signed_distance_to_surface should return the distance to the rim when outside of the rim.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, -1.0, 0.3, 0.4)), "CylinderShape4D get_signed_distance_to_surface should return the nearest point on the rim when outside of the rim.");
}

TEST_CASE("[CylinderShape4D] Signed distance on the axis") {
	Ref<CylinderShape4D> cylinder;
	cylinder.instantiate();
	cylinder->set_radius(0.5f);
	cylinder->set_height(2.0f);
	// On the Y axis, the radial direction is undefined, so any point on the curved surface at the same height is nearest.
	Vector4 surface_point;
	real_t signed_distance = cylinder->get_signed_distance_to_surface(Vector4(0.0, 0.2, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.5), "CylinderShape4D get_signed_distance_to_surface should return the negative radius on the axis when the curved surface is nearest.");
	CHECK_MESSAGE(surface_point.y == doctest::Approx(0.2), "CylinderShape4D get_signed_distance_to_surface should keep the height on the axis when the curved surface is nearest.");
	CHECK_MESSAGE(Vector4(surface_point.x, 0.0, surface_point.z, surface_point.w).length() == doctest::Approx(0.5), "CylinderShape4D get_signed_distance_to_surface should return a point on the curved surface on the axis when the curved surface is nearest.");
	signed_distance = cylinder->get_signed_distance_to_surface(Vector4(0.0, -0.7, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.3), "CylinderShape4D get_signed_distance_to_surface should return the negative distance to the cap on the axis when the cap is nearest.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, -1.0, 0.0, 0.0)), "CylinderShape4D get_signed_distance_to_surface should return the center of the cap on the axis when the cap is nearest.");
	signed_distance = cylinder->get_signed_distance_to_surface(Vector4(0.0, 3.0, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(2.0), "CylinderShape4D get_signed_distance_to_surface should return the positive distance to the cap on the axis when outside.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 1.0, 0.0, 0.0)), "CylinderShape4D get_signed_distance_to_surface should return the center of the cap on the axis when outside.");
}

TEST_CASE("[CylinderShape4D] Signed distance consistency") {
	Ref<CylinderShape4D> cylinder;
	cylinder.instantiate();
	cylinder->set_radius(0.5f);
	cylinder->set_height(2.0f);
	// A grid of points inside, on, and outside of the curved surface, the caps, and the rims.
	const real_t coordinates[] = { -1.6, -1.0, -0.6, -0.3, 0.0, 0.2, 0.4, 0.8, 1.3 };
	for (const real_t x : coordinates) {
		for (const real_t y : coordinates) {
			for (const real_t z : coordinates) {
				for (const real_t w : coordinates) {
					const Vector4 point = Vector4(x, y, z, w);
					INFO("Point: ", point);
					Vector4 surface_point;
					const real_t signed_distance = cylinder->get_signed_distance_to_surface(point, &surface_point);
					CHECK_MESSAGE(cylinder->get_signed_distance_to_surface(point) == signed_distance, "CylinderShape4D get_signed_distance_to_surface should not depend on whether the surface point is requested.");
					if (signed_distance < -CMP_EPSILON) {
						CHECK_MESSAGE(cylinder->has_point(point), "CylinderShape4D get_signed_distance_to_surface should be negative only for points inside the shape.");
					} else if (signed_distance > CMP_EPSILON) {
						CHECK_FALSE_MESSAGE(cylinder->has_point(point), "CylinderShape4D get_signed_distance_to_surface should be positive only for points outside the shape.");
					}
					if (signed_distance > 0.0f) {
						const Vector4 nearest_point = cylinder->get_nearest_point(point);
						CHECK_MESSAGE(signed_distance == doctest::Approx(point.distance_to(nearest_point)), "CylinderShape4D get_signed_distance_to_surface should return the distance to the nearest point when outside.");
						CHECK_MESSAGE(surface_point.is_equal_approx(nearest_point), "CylinderShape4D get_signed_distance_to_surface should return the nearest point as the surface point when outside.");
					} else {
						CHECK_MESSAGE(point.distance_to(surface_point) == doctest::Approx(-signed_distance), "CylinderShape4D get_signed_distance_to_surface should return a surface point at the signed distance when inside.");
					}
					CHECK_MESSAGE(cylinder->get_signed_distance_to_surface(surface_point) == doctest::Approx(0.0), "CylinderShape4D get_signed_distance_to_surface should return a surface point that is on the surface.");
				}
			}
		}
	}
}
} // namespace TestCylinderShape4D
