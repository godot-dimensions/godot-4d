#pragma once

#include "../../../physics/shapes/duocylinder_shape_4d.h"

#include "tests/test_macros.h"

namespace TestDuocylinderShape4D {
TEST_CASE("[DuocylinderShape4D] Raycast") {
	Ref<DuocylinderShape4D> duocylinder;
	duocylinder.instantiate();
	// Default radii are 0.5. Ray from outside, moving only in ZW, hitting the ZW boundary.
	Dictionary result = duocylinder->raycast_intersects(Vector4(0.0, 0.0, -2.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0));
	CHECK_MESSAGE((bool)result["hit"], "DuocylinderShape4D raycast from outside should hit the ZW boundary.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.5), "DuocylinderShape4D raycast from outside should return the distance to the ZW boundary.");
	CHECK_MESSAGE(((Vector4)result["normal"]).is_equal_approx(Vector4(0.0, 0.0, -1.0, 0.0)), "DuocylinderShape4D raycast from outside should return the outward normal of the ZW boundary.");
	CHECK_MESSAGE(((Vector4)result["point"]).is_equal_approx(Vector4(0.0, 0.0, -0.5, 0.0)), "DuocylinderShape4D raycast from outside should return the point on the ZW boundary.");
	// Ray from outside, moving in both planes, entering the XY circle before the ZW circle, so it enters through the ZW boundary.
	result = duocylinder->raycast_intersects(Vector4(-1.0, 0.0, -2.0, 0.0), Vector4(1.0, 0.0, 2.0, 0.0).normalized());
	CHECK_MESSAGE((bool)result["hit"], "DuocylinderShape4D raycast from outside moving in both planes should hit.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(0.75 * Math::sqrt(5.0)), "DuocylinderShape4D raycast from outside moving in both planes should return the distance to the boundary it enters through.");
	CHECK_MESSAGE(((Vector4)result["normal"]).is_equal_approx(Vector4(0.0, 0.0, -1.0, 0.0)), "DuocylinderShape4D raycast from outside moving in both planes should return the outward normal of the boundary it enters through.");
	CHECK_MESSAGE(((Vector4)result["point"]).is_equal_approx(Vector4(-0.25, 0.0, -0.5, 0.0)), "DuocylinderShape4D raycast from outside moving in both planes should return the point on the boundary it enters through.");
	// Ray from outside, missing.
	result = duocylinder->raycast_intersects(Vector4(0.6, 0.0, -2.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0));
	CHECK_FALSE_MESSAGE((bool)result["hit"], "DuocylinderShape4D raycast passing beside the XY boundary should not hit.");
	// Ray from inside, exiting through the XY boundary.
	result = duocylinder->raycast_intersects(Vector4(0.0, 0.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0));
	CHECK_MESSAGE((bool)result["hit"], "DuocylinderShape4D raycast from inside should hit the XY boundary.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(0.5), "DuocylinderShape4D raycast from inside should return the distance to the XY boundary.");
	CHECK_MESSAGE(((Vector4)result["normal"]).is_equal_approx(Vector4(1.0, 0.0, 0.0, 0.0)), "DuocylinderShape4D raycast from inside should return the outward normal of the XY boundary.");
	CHECK_MESSAGE(((Vector4)result["point"]).is_equal_approx(Vector4(0.5, 0.0, 0.0, 0.0)), "DuocylinderShape4D raycast from inside should return the point on the XY boundary.");
	result = duocylinder->raycast_intersects(Vector4(0.0, 0.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), 0.4);
	CHECK_FALSE_MESSAGE((bool)result["hit"], "DuocylinderShape4D raycast from inside should not hit beyond the max distance.");
	result = duocylinder->raycast_intersects(Vector4(0.0, 0.0, 0.0, 0.0), Vector4(1.0, 0.0, 0.0, 0.0), Math_INF, true);
	CHECK_MESSAGE((bool)result["hit"], "DuocylinderShape4D raycast from inside should hit when inside is zero.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(0.0), "DuocylinderShape4D raycast from inside should return zero distance when inside is zero.");
}

TEST_CASE("[DuocylinderShape4D] Raycast along the boundary") {
	Ref<DuocylinderShape4D> duocylinder;
	duocylinder.instantiate();
	// Default radii are 0.5. A ray that only moves in ZW, starting on the XY boundary, stays on the XY boundary,
	// which is part of the shape, so it hits where it enters the ZW circle.
	Dictionary result = duocylinder->raycast_intersects(Vector4(0.5, 0.0, -2.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0));
	CHECK_MESSAGE((bool)result["hit"], "DuocylinderShape4D raycast moving only in ZW along the XY boundary should hit.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.5), "DuocylinderShape4D raycast moving only in ZW along the XY boundary should return the distance to the ZW boundary.");
	CHECK_MESSAGE(((Vector4)result["normal"]).is_equal_approx(Vector4(0.0, 0.0, -1.0, 0.0)), "DuocylinderShape4D raycast moving only in ZW along the XY boundary should return the outward normal of the ZW boundary.");
	CHECK_MESSAGE(((Vector4)result["point"]).is_equal_approx(Vector4(0.5, 0.0, -0.5, 0.0)), "DuocylinderShape4D raycast moving only in ZW along the XY boundary should return the point on the ZW boundary.");
	// Same for a ray that only moves in XY, starting on the ZW boundary.
	result = duocylinder->raycast_intersects(Vector4(-2.0, 0.0, 0.0, 0.5), Vector4(1.0, 0.0, 0.0, 0.0));
	CHECK_MESSAGE((bool)result["hit"], "DuocylinderShape4D raycast moving only in XY along the ZW boundary should hit.");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.5), "DuocylinderShape4D raycast moving only in XY along the ZW boundary should return the distance to the XY boundary.");
	CHECK_MESSAGE(((Vector4)result["normal"]).is_equal_approx(Vector4(-1.0, 0.0, 0.0, 0.0)), "DuocylinderShape4D raycast moving only in XY along the ZW boundary should return the outward normal of the XY boundary.");
	CHECK_MESSAGE(((Vector4)result["point"]).is_equal_approx(Vector4(-0.5, 0.0, 0.0, 0.5)), "DuocylinderShape4D raycast moving only in XY along the ZW boundary should return the point on the XY boundary.");
}

TEST_CASE("[DuocylinderShape4D] Signed distance inside") {
	Ref<DuocylinderShape4D> duocylinder;
	duocylinder.instantiate();
	duocylinder->set_radius_xy(1.0f);
	duocylinder->set_radius_zw(0.25f);
	// Inside, the signed distance is the largest (least negative) of the XY and ZW signed distances,
	// and the nearest surface point only moves the point in that plane.
	Vector4 surface_point;
	real_t signed_distance = duocylinder->get_signed_distance_to_surface(Vector4(0.3, 0.4, 0.1, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.15), "DuocylinderShape4D get_signed_distance_to_surface should return the negative distance to the ZW boundary when it is nearest, even when the point is farther from the center in XY.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.3, 0.4, 0.25, 0.0)), "DuocylinderShape4D get_signed_distance_to_surface should move the point in ZW when the ZW boundary is nearest.");
	signed_distance = duocylinder->get_signed_distance_to_surface(Vector4(0.54, 0.72, 0.0, 0.05), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.1), "DuocylinderShape4D get_signed_distance_to_surface should return the negative distance to the XY boundary when it is nearest.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.6, 0.8, 0.0, 0.05)), "DuocylinderShape4D get_signed_distance_to_surface should move the point in XY when the XY boundary is nearest.");
}

TEST_CASE("[DuocylinderShape4D] Signed distance on the surface") {
	Ref<DuocylinderShape4D> duocylinder;
	duocylinder.instantiate();
	duocylinder->set_radius_xy(1.0f);
	duocylinder->set_radius_zw(0.25f);
	const Vector4 surface_points[] = {
		Vector4(0.6, 0.8, 0.1, 0.0),
		Vector4(0.0, -1.0, 0.0, 0.0),
		Vector4(0.2, -0.3, 0.0, 0.25),
		Vector4(0.0, 0.0, -0.25, 0.0),
		Vector4(0.6, 0.8, 0.15, 0.2),
	};
	for (const Vector4 &point : surface_points) {
		Vector4 surface_point;
		const real_t signed_distance = duocylinder->get_signed_distance_to_surface(point, &surface_point);
		CHECK_MESSAGE(signed_distance == doctest::Approx(0.0), "DuocylinderShape4D get_signed_distance_to_surface should return zero for points on the surface.");
		CHECK_MESSAGE(surface_point.is_equal_approx(point), "DuocylinderShape4D get_signed_distance_to_surface should return the same point for points on the surface.");
	}
}

TEST_CASE("[DuocylinderShape4D] Signed distance outside") {
	Ref<DuocylinderShape4D> duocylinder;
	duocylinder.instantiate();
	// Default radii are 0.5. Outside of one circle but inside the other, only the outside circle contributes.
	Vector4 surface_point;
	real_t signed_distance = duocylinder->get_signed_distance_to_surface(Vector4(0.4, 0.0, 2.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(1.5), "DuocylinderShape4D get_signed_distance_to_surface should return the positive distance when outside, even when the point is close to the other boundary.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.4, 0.0, 0.5, 0.0)), "DuocylinderShape4D get_signed_distance_to_surface should return the nearest point when outside.");
	duocylinder->set_radius_xy(1.0f);
	duocylinder->set_radius_zw(0.25f);
	signed_distance = duocylinder->get_signed_distance_to_surface(Vector4(0.0, 0.0, 0.6, 0.8), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.75), "DuocylinderShape4D get_signed_distance_to_surface should return the positive distance to the ZW boundary when outside of only it.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 0.0, 0.15, 0.2)), "DuocylinderShape4D get_signed_distance_to_surface should return the nearest point on the ZW boundary when outside of only it.");
	// Outside of both circles, both distances contribute, and the nearest point is on the ridge where the boundaries meet.
	signed_distance = duocylinder->get_signed_distance_to_surface(Vector4(0.78, 1.04, 0.39, 0.52), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.5), "DuocylinderShape4D get_signed_distance_to_surface should return the distance to the ridge when outside of both circles.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.6, 0.8, 0.15, 0.2)), "DuocylinderShape4D get_signed_distance_to_surface should return the nearest point on the ridge when outside of both circles.");
}

TEST_CASE("[DuocylinderShape4D] Signed distance on the planes") {
	Ref<DuocylinderShape4D> duocylinder;
	duocylinder.instantiate();
	duocylinder->set_radius_xy(1.0f);
	duocylinder->set_radius_zw(0.25f);
	// On the XY plane, the ZW direction is undefined, so any point on the ZW boundary with the same X and Y is nearest.
	Vector4 surface_point;
	real_t signed_distance = duocylinder->get_signed_distance_to_surface(Vector4(0.0, 0.0, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.25), "DuocylinderShape4D get_signed_distance_to_surface should return the negative ZW radius on the XY plane when the ZW boundary is nearest.");
	CHECK_MESSAGE(Vector2(surface_point.x, surface_point.y).is_zero_approx(), "DuocylinderShape4D get_signed_distance_to_surface should keep X and Y on the XY plane when the ZW boundary is nearest.");
	CHECK_MESSAGE(Vector2(surface_point.z, surface_point.w).length() == doctest::Approx(0.25), "DuocylinderShape4D get_signed_distance_to_surface should return a point on the ZW boundary on the XY plane when the ZW boundary is nearest.");
	// On the ZW plane, outside of the ZW circle, the XY coordinates are inside and stay the same.
	signed_distance = duocylinder->get_signed_distance_to_surface(Vector4(0.0, 0.0, 1.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.75), "DuocylinderShape4D get_signed_distance_to_surface should return the positive distance to the ZW boundary on the ZW plane when outside.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 0.0, 0.25, 0.0)), "DuocylinderShape4D get_signed_distance_to_surface should return the nearest point on the ZW boundary on the ZW plane when outside.");
	// On the ZW plane, the XY direction is undefined, so any point on the XY boundary with the same Z and W is nearest.
	duocylinder->set_radius_xy(0.25f);
	duocylinder->set_radius_zw(1.0f);
	signed_distance = duocylinder->get_signed_distance_to_surface(Vector4(0.0, 0.0, 0.5, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.25), "DuocylinderShape4D get_signed_distance_to_surface should return the negative XY radius on the ZW plane when the XY boundary is nearest.");
	CHECK_MESSAGE(Vector2(surface_point.x, surface_point.y).length() == doctest::Approx(0.25), "DuocylinderShape4D get_signed_distance_to_surface should return a point on the XY boundary on the ZW plane when the XY boundary is nearest.");
	CHECK_MESSAGE(Vector2(surface_point.z, surface_point.w).is_equal_approx(Vector2(0.5, 0.0)), "DuocylinderShape4D get_signed_distance_to_surface should keep Z and W on the ZW plane when the XY boundary is nearest.");
}

TEST_CASE("[DuocylinderShape4D] Signed distance consistency") {
	Ref<DuocylinderShape4D> duocylinder;
	duocylinder.instantiate();
	duocylinder->set_radius_xy(1.0f);
	duocylinder->set_radius_zw(0.25f);
	// A grid of points inside, on, and outside of each boundary and the ridge where they meet.
	const real_t coordinates[] = { -1.3, -0.8, -0.2, 0.0, 0.1, 0.25, 0.6, 1.0, 1.6 };
	for (const real_t x : coordinates) {
		for (const real_t y : coordinates) {
			for (const real_t z : coordinates) {
				for (const real_t w : coordinates) {
					const Vector4 point = Vector4(x, y, z, w);
					INFO("Point: ", point);
					Vector4 surface_point;
					const real_t signed_distance = duocylinder->get_signed_distance_to_surface(point, &surface_point);
					CHECK_MESSAGE(duocylinder->get_signed_distance_to_surface(point) == signed_distance, "DuocylinderShape4D get_signed_distance_to_surface should not depend on whether the surface point is requested.");
					if (signed_distance < -CMP_EPSILON) {
						CHECK_MESSAGE(duocylinder->has_point(point), "DuocylinderShape4D get_signed_distance_to_surface should be negative only for points inside the shape.");
					} else if (signed_distance > CMP_EPSILON) {
						CHECK_FALSE_MESSAGE(duocylinder->has_point(point), "DuocylinderShape4D get_signed_distance_to_surface should be positive only for points outside the shape.");
					}
					if (signed_distance > 0.0f) {
						const Vector4 nearest_point = duocylinder->get_nearest_point(point);
						CHECK_MESSAGE(signed_distance == doctest::Approx(point.distance_to(nearest_point)), "DuocylinderShape4D get_signed_distance_to_surface should return the distance to the nearest point when outside.");
						CHECK_MESSAGE(surface_point.is_equal_approx(nearest_point), "DuocylinderShape4D get_signed_distance_to_surface should return the nearest point as the surface point when outside.");
					} else {
						CHECK_MESSAGE(point.distance_to(surface_point) == doctest::Approx(-signed_distance), "DuocylinderShape4D get_signed_distance_to_surface should return a surface point at the signed distance when inside.");
					}
					CHECK_MESSAGE(duocylinder->get_signed_distance_to_surface(surface_point) == doctest::Approx(0.0), "DuocylinderShape4D get_signed_distance_to_surface should return a surface point that is on the surface.");
				}
			}
		}
	}
}
} // namespace TestDuocylinderShape4D
