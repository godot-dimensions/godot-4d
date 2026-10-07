#pragma once

#include "../../../physics/shapes/box_shape_4d.h"
#include "../../../physics/shapes/convex_hull_shape_4d.h"

#include "tests/test_macros.h"

namespace TestConvexHullShape4D {
TEST_CASE("[ConvexHullShape4D] Get Support Point") {
	Ref<ConvexHullShape4D> hull;
	hull.instantiate();
	// An interior point first, so that it would be returned if the support point did not search all points,
	// followed by the 16 corners of a tesseract.
	PackedVector4Array points;
	points.push_back(Vector4(0.25, -0.5, 0.75, 0.0));
	for (int i = 0; i < 16; i++) {
		points.push_back(Vector4((i & 1) ? 1.0 : -1.0, (i & 2) ? 1.0 : -1.0, (i & 4) ? 1.0 : -1.0, (i & 8) ? 1.0 : -1.0));
	}
	hull->set_points(points);
	const Vector4 directions[] = {
		Vector4(1.0, 1.0, 1.0, 1.0),
		Vector4(-1.0, -1.0, -1.0, -1.0),
		Vector4(-1.0, 0.5, 0.3, -2.0),
		Vector4(0.2, -1.0, -0.5, 1.5),
		Vector4(-0.1, -0.2, 3.0, 0.4),
	};
	for (const Vector4 &direction : directions) {
		const Vector4 support = hull->get_support_point(direction);
		CHECK_MESSAGE(support == direction.sign(), "ConvexHullShape4D get_support_point should return the tesseract corner furthest along the direction.");
		for (const Vector4 &point : points) {
			CHECK_MESSAGE(support.dot(direction) >= point.dot(direction), "ConvexHullShape4D get_support_point should be at least as far along the direction as every point.");
		}
	}
	// When the direction is zero on some axes, multiple corners are equally far along it, so any of them is valid.
	const Vector4 axis_direction = Vector4(0.0, 0.0, 0.0, -1.0);
	const Vector4 axis_support = hull->get_support_point(axis_direction);
	CHECK_MESSAGE(axis_support.w == doctest::Approx(-1.0), "ConvexHullShape4D get_support_point should return a corner on the face furthest along an axis direction.");
	CHECK_MESSAGE(points.has(axis_support), "ConvexHullShape4D get_support_point should return one of the hull's points.");
	CHECK_MESSAGE(axis_support != points[0], "ConvexHullShape4D get_support_point should never return an interior point for a non-zero direction.");
}

TEST_CASE("[ConvexHullShape4D] Empty hull") {
	Ref<ConvexHullShape4D> hull;
	hull.instantiate();
	CHECK_MESSAGE(hull->get_support_point(Vector4(1.0, -2.0, 3.0, -4.0)) == Vector4(), "ConvexHullShape4D get_support_point should return the origin for an empty hull.");
	CHECK_MESSAGE(hull->get_rect_bounds() == Rect4(), "ConvexHullShape4D get_rect_bounds should return a zero-size rect at the origin for an empty hull.");
	const Transform4D transform = Transform4D(Basis4D::from_xy(0.5), Vector4(5.0, 6.0, 7.0, 8.0));
	CHECK_MESSAGE(hull->get_rect_bounds(transform) == Rect4(Vector4(5.0, 6.0, 7.0, 8.0), Vector4()), "ConvexHullShape4D get_rect_bounds should return a zero-size rect at the target origin for an empty hull.");
}

TEST_CASE("[ConvexHullShape4D] Get Rect Bounds") {
	Ref<ConvexHullShape4D> hull;
	hull.instantiate();
	// The corners of a box that does not surround the hull's origin, with all X values in [2, 3], plus an interior point.
	const Vector4 box_position = Vector4(2.0, 1.0, -1.0, 0.25);
	const Vector4 box_size = Vector4(1.0, 1.0, 0.5, 0.25);
	PackedVector4Array points;
	for (int i = 0; i < 16; i++) {
		const Vector4 corner_offset = Vector4((i & 1) ? 1.0 : 0.0, (i & 2) ? 1.0 : 0.0, (i & 4) ? 1.0 : 0.0, (i & 8) ? 1.0 : 0.0);
		points.push_back(box_position + corner_offset * box_size);
	}
	points.push_back(box_position + box_size * 0.5);
	hull->set_points(points);
	// With the identity transform, the bounds are exactly the box, and do not include the origin.
	const Rect4 identity_bounds = hull->get_rect_bounds();
	CHECK_MESSAGE(identity_bounds.is_equal_approx(Rect4(box_position, box_size)), "ConvexHullShape4D get_rect_bounds should return the exact bounds of the points.");
	CHECK_FALSE_MESSAGE(identity_bounds.has_point(Vector4()), "ConvexHullShape4D get_rect_bounds should not include the origin when the points do not surround it.");
	// Rotate 90 degrees from +X to +Y, which maps (x, y, z, w) to (-y, x, z, w), and then translate.
	const Transform4D swap_transform = Transform4D(Vector4(0.0, 1.0, 0.0, 0.0), Vector4(-1.0, 0.0, 0.0, 0.0), Vector4(0.0, 0.0, 1.0, 0.0), Vector4(0.0, 0.0, 0.0, 1.0), Vector4(10.0, 20.0, 30.0, 40.0));
	const Rect4 swap_bounds = hull->get_rect_bounds(swap_transform);
	CHECK_MESSAGE(swap_bounds.is_equal_approx(Rect4(Vector4(8.0, 22.0, 29.0, 40.25), Vector4(1.0, 1.0, 0.5, 0.25))), "ConvexHullShape4D get_rect_bounds should return the exact bounds of the transformed points.");
	CHECK_FALSE_MESSAGE(swap_bounds.has_point(swap_transform.origin), "ConvexHullShape4D get_rect_bounds should not include the target origin when the points do not surround it.");
	// With an arbitrary rotation, the bounds must match the bounds of an equivalent box shape.
	Ref<BoxShape4D> box;
	box.instantiate();
	box->set_size(box_size);
	const Transform4D rotated_transform = Transform4D(Basis4D::from_xy(0.3) * Basis4D::from_zw(-0.7) * Basis4D::from_xw(1.1), Vector4(-3.0, 4.0, -5.0, 6.0));
	const Rect4 expected_rotated_bounds = box->get_rect_bounds(rotated_transform * Transform4D(Basis4D(), box_position + box_size * 0.5));
	const Rect4 rotated_bounds = hull->get_rect_bounds(rotated_transform);
	CHECK_MESSAGE(rotated_bounds.is_equal_approx(expected_rotated_bounds), "ConvexHullShape4D get_rect_bounds should return the exact bounds of the rotated points.");
	CHECK_FALSE_MESSAGE(rotated_bounds.has_point(rotated_transform.origin), "ConvexHullShape4D get_rect_bounds should not include the target origin when the points do not surround it.");
	// The default raycast falls back to the rect bounds, which are exact for this hull.
	const Dictionary raycast = hull->raycast_intersects(Vector4(0.0, 1.5, -0.75, 0.375), Vector4(1.0, 0.0, 0.0, 0.0));
	CHECK_MESSAGE(bool(raycast["hit"]), "ConvexHullShape4D raycast_intersects should hit the hull when the ray points at it.");
	CHECK_MESSAGE(real_t(raycast["distance"]) == doctest::Approx(2.0), "ConvexHullShape4D raycast_intersects should hit the hull's points, not its origin.");
}
} // namespace TestConvexHullShape4D
