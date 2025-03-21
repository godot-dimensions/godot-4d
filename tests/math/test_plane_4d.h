#pragma once

#include "../../math/plane_4d.h"
#include "tests/test_macros.h"

namespace TestPlane4D {
TEST_CASE("[Plane4D] Intersect Ray") {
	const Plane4D plane = Plane4D(Vector4(0, 1, 0, 0), 5);
	const Vector4 ray_origin = Vector4(1, 2, 3, 4);
	const Vector4 ray_down = Vector4(0, -1, 0, 0);
	Variant result = plane.intersect_ray(ray_origin, ray_down);
	CHECK_MESSAGE(result.get_type() == Variant::NIL, "Plane4D intersect_ray should return NIL for invalid hits.");

	const Vector4 ray_up = Vector4(0, 1, 0, 0);
	result = plane.intersect_ray(ray_origin, ray_up);
	CHECK_MESSAGE(result.get_type() == Variant::VECTOR4, "Plane4D intersect_ray should return a Vector4 for valid hits.");
	CHECK_MESSAGE(result == Vector4(1, 5, 3, 4), "Plane4D intersect_ray should work as expected.");
}

TEST_CASE("[Plane4D] Intersect Line Segment") {
	const Plane4D plane = Plane4D(Vector4(0, 1, 0, 0), -3);
	const Vector4 begin = Vector4(1, 2, 3, 4);
	const Vector4 end = Vector4(1, -4, 9, 4);
	Variant result = plane.intersect_line_segment(begin, end);
	CHECK_MESSAGE(result.get_type() == Variant::VECTOR4, "Plane4D intersect_line_segment should return a Vector4 for valid hits.");
	CHECK_MESSAGE(result == Vector4(1, -3, 8, 4), "Plane4D intersect_line_segment should work as expected.");
	result = plane.intersect_line_segment(end, begin);
	CHECK_MESSAGE(result.get_type() == Variant::VECTOR4, "Plane4D intersect_line_segment should return a Vector4 for valid hits.");
	CHECK_MESSAGE(result == Vector4(1, -3, 8, 4), "Plane4D intersect_line_segment should work as expected.");
}

TEST_CASE("[Plane4D] From Coplanar Directions") {
	const Plane4D xyz = Plane4D::from_coplanar_directions(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 0));
	CHECK_MESSAGE(xyz.get_normal() == Vector4(0, 0, 0, 1), "Plane4D from_coplanar_directions should work as expected.");
	const Plane4D xwy = Plane4D::from_coplanar_directions(Vector4(1, 0, 0, 0), Vector4(0, 0, 0, 1), Vector4(0, 1, 0, 0), Vector4(0, 0, 0, 0));
	CHECK_MESSAGE(xwy.get_normal() == Vector4(0, 0, 1, 0), "Plane4D from_coplanar_directions should work as expected.");
	const Plane4D xzw = Plane4D::from_coplanar_directions(Vector4(1, 0, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 0));
	CHECK_MESSAGE(xzw.get_normal() == Vector4(0, 1, 0, 0), "Plane4D from_coplanar_directions should work as expected.");
	const Plane4D ywz = Plane4D::from_coplanar_directions(Vector4(0, 1, 0, 0), Vector4(0, 0, 0, 1), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 0));
	CHECK_MESSAGE(ywz.get_normal() == Vector4(1, 0, 0, 0), "Plane4D from_coplanar_directions should work as expected.");

	const Plane4D test = Plane4D::from_coplanar_directions(Vector4(1, 2, 3, 4), Vector4(5, 6, 7, 8), Vector4(9, 10, 11, -12), Vector4(13, 14, 15, 16));
	CHECK_MESSAGE(test.get_normal().dot(Vector4(1, 2, 3, 4)) == doctest::Approx(0), "Plane4D from_coplanar_directions normal should be perpendicular to the input directions.");
	CHECK_MESSAGE(test.get_normal().dot(Vector4(5, 6, 7, 8)) == doctest::Approx(0), "Plane4D from_coplanar_directions normal should be perpendicular to the input directions.");
	CHECK_MESSAGE(test.get_normal().dot(Vector4(9, 10, 11, -12)) == doctest::Approx(0), "Plane4D from_coplanar_directions normal should be perpendicular to the input directions.");
	CHECK_MESSAGE(test.has_point(Vector4(13, 14, 15, 16)), "Plane4D from_coplanar_directions should contain the input point.");
}
} // namespace TestPlane4D
