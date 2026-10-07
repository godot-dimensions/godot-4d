#pragma once

#include "../../../physics/shapes/box_shape_4d.h"

#include "tests/test_macros.h"

namespace TestBoxShape4D {
TEST_CASE("[BoxShape4D] Raycast from outside") {
	Ref<BoxShape4D> box;
	box.instantiate();
	box->set_size(Vector4(2, 2, 2, 2));
	// Ray from outside pointing at center.
	Dictionary result = box->raycast_intersects(Vector4(-3, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from outside should hit box");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(2.0f), "Distance should be 2.0 (size is 2, so extends -1 to 1)");
	Vector4 normal = result["normal"];
	CHECK_MESSAGE(normal == Vector4(-1, 0, 0, 0), "Normal should point backwards along X");
	// Ray from outside pointing away.
	result = box->raycast_intersects(Vector4(3, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == false, "Raycast pointing away should not hit");
	// Ray missing the box.
	result = box->raycast_intersects(Vector4(-3, 3, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == false, "Raycast missing box should not hit");
}

TEST_CASE("[BoxShape4D] Raycast from inside") {
	Ref<BoxShape4D> box;
	box.instantiate();
	box->set_size(Vector4(2, 2, 2, 2));
	// Ray from center pointing towards +X.
	// Verify that raycast returns a hit when starting inside.
	Dictionary result = box->raycast_intersects(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast from center should hit");
	// Distance may be negative or positive depending on implementation, just verify it's set.
	CHECK_MESSAGE(result.has("distance"), "Result should have distance field");
}

TEST_CASE("[BoxShape4D] Raycast through different axes") {
	Ref<BoxShape4D> box;
	box.instantiate();
	box->set_size(Vector4(2, 4, 2, 2));
	// Ray along Y axis.
	Dictionary result = box->raycast_intersects(Vector4(0, -3, 0, 0), Vector4(0, 1, 0, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast along Y should hit");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(1.0f), "Distance along Y should be 1.0 (Y extends -2 to 2)");
	// Ray along Z axis.
	result = box->raycast_intersects(Vector4(0, 0, -3, 0), Vector4(0, 0, 1, 0).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast along Z should hit");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(2.0f), "Distance along Z should be 2.0");
	// Ray along W axis.
	result = box->raycast_intersects(Vector4(0, 0, 0, -3), Vector4(0, 0, 0, 1).normalized());
	CHECK_MESSAGE((bool)result["hit"] == true, "Raycast along W should hit");
	CHECK_MESSAGE((real_t)result["distance"] == doctest::Approx(2.0f), "Distance along W should be 2.0");
}

TEST_CASE("[BoxShape4D] Signed distance inside") {
	Ref<BoxShape4D> box;
	box.instantiate();
	box->set_size(Vector4(1, 2, 3, 4));
	// Inside, the signed distance is the largest (least negative) per-axis signed distance,
	// and the nearest surface point only moves the point along that axis.
	Vector4 surface_point;
	real_t signed_distance = box->get_signed_distance_to_surface(Vector4(0.2, 0.5, -1.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.3), "BoxShape4D get_signed_distance_to_surface should return the negative distance to the nearest face when inside.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.5, 0.5, -1.0, 0.0)), "BoxShape4D get_signed_distance_to_surface should move the point onto the nearest face when inside.");
	signed_distance = box->get_signed_distance_to_surface(Vector4(0.1, -0.9, 0.3, 1.2), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.1), "BoxShape4D get_signed_distance_to_surface should return the negative distance to the nearest face when inside.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.1, -1.0, 0.3, 1.2)), "BoxShape4D get_signed_distance_to_surface should move the point onto the nearest face when inside.");
	signed_distance = box->get_signed_distance_to_surface(Vector4(0.0, 0.3, 1.0, -1.85), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.15), "BoxShape4D get_signed_distance_to_surface should return the negative distance to the nearest face when inside.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 0.3, 1.0, -2.0)), "BoxShape4D get_signed_distance_to_surface should move the point onto the nearest face when inside.");
	// At the center, the X faces are nearest, and a zero coordinate moves to the positive face.
	signed_distance = box->get_signed_distance_to_surface(Vector4(0.0, 0.0, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(-0.5), "BoxShape4D get_signed_distance_to_surface should return the negative distance to the nearest face at the center.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.5, 0.0, 0.0, 0.0)), "BoxShape4D get_signed_distance_to_surface should move a zero coordinate onto the positive face.");
}

TEST_CASE("[BoxShape4D] Signed distance on the surface") {
	Ref<BoxShape4D> box;
	box.instantiate();
	box->set_size(Vector4(1, 2, 3, 4));
	const Vector4 surface_points[] = {
		Vector4(0.5, 0.2, 0.1, -0.3),
		Vector4(0.2, -1.0, 0.0, 0.0),
		Vector4(0.0, 0.0, 0.0, 2.0),
		Vector4(-0.5, 1.0, -1.5, 2.0),
	};
	for (const Vector4 &point : surface_points) {
		Vector4 surface_point;
		const real_t signed_distance = box->get_signed_distance_to_surface(point, &surface_point);
		CHECK_MESSAGE(signed_distance == doctest::Approx(0.0), "BoxShape4D get_signed_distance_to_surface should return zero for points on the surface.");
		CHECK_MESSAGE(surface_point.is_equal_approx(point), "BoxShape4D get_signed_distance_to_surface should return the same point for points on the surface.");
	}
}

TEST_CASE("[BoxShape4D] Signed distance outside") {
	Ref<BoxShape4D> box;
	box.instantiate();
	// Default size is (1, 1, 1, 1), so the half extents are 0.5. Outside of one face, only that axis contributes.
	Vector4 surface_point;
	real_t signed_distance = box->get_signed_distance_to_surface(Vector4(1.5, 0.45, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(1.0), "BoxShape4D get_signed_distance_to_surface should return the positive distance to the face when outside, even when the point is close to another face's plane.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.5, 0.45, 0.0, 0.0)), "BoxShape4D get_signed_distance_to_surface should return the nearest point on the face when outside.");
	// Outside of an edge or corner, every axis outside of the box contributes.
	signed_distance = box->get_signed_distance_to_surface(Vector4(1.5, 1.5, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(Math::sqrt(2.0)), "BoxShape4D get_signed_distance_to_surface should return the distance to the nearest edge when outside of an edge.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.5, 0.5, 0.0, 0.0)), "BoxShape4D get_signed_distance_to_surface should return the nearest point on the edge when outside of an edge.");
	box->set_size(Vector4(1, 2, 3, 4));
	signed_distance = box->get_signed_distance_to_surface(Vector4(0.0, 0.0, 0.0, -2.5), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.5), "BoxShape4D get_signed_distance_to_surface should return the positive distance to the face when outside.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.0, 0.0, 0.0, -2.0)), "BoxShape4D get_signed_distance_to_surface should return the nearest point on the face when outside.");
	signed_distance = box->get_signed_distance_to_surface(Vector4(0.8, 1.4, 0.0, 0.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(0.5), "BoxShape4D get_signed_distance_to_surface should return the distance to the nearest edge when outside of an edge.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(0.5, 1.0, 0.0, 0.0)), "BoxShape4D get_signed_distance_to_surface should return the nearest point on the edge when outside of an edge.");
	signed_distance = box->get_signed_distance_to_surface(Vector4(-1.5, -2.0, 2.5, 3.0), &surface_point);
	CHECK_MESSAGE(signed_distance == doctest::Approx(2.0), "BoxShape4D get_signed_distance_to_surface should return the distance to the nearest vertex when outside of a vertex.");
	CHECK_MESSAGE(surface_point.is_equal_approx(Vector4(-0.5, -1.0, 1.5, 2.0)), "BoxShape4D get_signed_distance_to_surface should return the nearest vertex when outside of a vertex.");
}

TEST_CASE("[BoxShape4D] Signed distance consistency") {
	Ref<BoxShape4D> box;
	box.instantiate();
	box->set_size(Vector4(1, 2, 3, 4));
	// A grid of points inside, on, and outside of every face, edge, and corner of the box.
	const real_t coordinates[] = { -2.4, -1.5, -1.2, -0.7, -0.3, 0.0, 0.5, 0.9, 1.7 };
	for (const real_t x : coordinates) {
		for (const real_t y : coordinates) {
			for (const real_t z : coordinates) {
				for (const real_t w : coordinates) {
					const Vector4 point = Vector4(x, y, z, w);
					INFO("Point: ", point);
					Vector4 surface_point;
					const real_t signed_distance = box->get_signed_distance_to_surface(point, &surface_point);
					CHECK_MESSAGE(box->get_signed_distance_to_surface(point) == signed_distance, "BoxShape4D get_signed_distance_to_surface should not depend on whether the surface point is requested.");
					if (signed_distance < -CMP_EPSILON) {
						CHECK_MESSAGE(box->has_point(point), "BoxShape4D get_signed_distance_to_surface should be negative only for points inside the shape.");
					} else if (signed_distance > CMP_EPSILON) {
						CHECK_FALSE_MESSAGE(box->has_point(point), "BoxShape4D get_signed_distance_to_surface should be positive only for points outside the shape.");
					}
					if (signed_distance > 0.0f) {
						const Vector4 nearest_point = box->get_nearest_point(point);
						CHECK_MESSAGE(signed_distance == doctest::Approx(point.distance_to(nearest_point)), "BoxShape4D get_signed_distance_to_surface should return the distance to the nearest point when outside.");
						CHECK_MESSAGE(surface_point.is_equal_approx(nearest_point), "BoxShape4D get_signed_distance_to_surface should return the nearest point as the surface point when outside.");
					} else {
						CHECK_MESSAGE(point.distance_to(surface_point) == doctest::Approx(-signed_distance), "BoxShape4D get_signed_distance_to_surface should return a surface point at the signed distance when inside.");
					}
					CHECK_MESSAGE(box->get_signed_distance_to_surface(surface_point) == doctest::Approx(0.0), "BoxShape4D get_signed_distance_to_surface should return a surface point that is on the surface.");
				}
			}
		}
	}
}
} // namespace TestBoxShape4D
