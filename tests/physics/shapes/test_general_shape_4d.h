#pragma once

#include "../../../physics/shapes/general_shape_4d.h"

#include "tests/test_macros.h"

namespace TestGeneralShape4D {
TEST_CASE("[GeneralShape4D] Get Support Point with a direction perpendicular to the curve") {
	// A 4D cylinder, which is a base box along Y with a 3D sphere curve on the XZW axes.
	Ref<GeneralShapeCurve4D> sphere_curve;
	sphere_curve.instantiate();
	sphere_curve->set_radii(Vector4(0.5, 0.0, 0.5, 0.5));
	TypedArray<GeneralShapeCurve4D> curves;
	curves.push_back(sphere_curve);
	Ref<GeneralShape4D> cylinder;
	cylinder.instantiate();
	cylinder->set_base_half_extents(Vector4(0.0, 1.0, 0.0, 0.0));
	cylinder->set_curves(curves);
	// The direction has no component on the curve's axes, so the curve adds nothing, and only the base box remains.
	Vector4 support = cylinder->get_support_point(Vector4(0.0, 1.0, 0.0, 0.0));
	CHECK_MESSAGE(support.is_finite(), "GeneralShape4D get_support_point should not return NaN when the direction is perpendicular to a curve.");
	CHECK_MESSAGE(support == Vector4(0.0, 1.0, 0.0, 0.0), "GeneralShape4D get_support_point should return the base box's cap center when the direction is perpendicular to a curve.");
	support = cylinder->get_support_point(Vector4(0.0, -3.0, 0.0, 0.0));
	CHECK_MESSAGE(support.is_finite(), "GeneralShape4D get_support_point should not return NaN when the direction is perpendicular to a curve.");
	CHECK_MESSAGE(support == Vector4(0.0, -1.0, 0.0, 0.0), "GeneralShape4D get_support_point should return the base box's cap center when the direction is perpendicular to a curve.");
	// Directions on the curve's axes still use the curve.
	support = cylinder->get_support_point(Vector4(0.0, 1.0, -2.0, 0.0));
	CHECK_MESSAGE(support.is_equal_approx(Vector4(0.0, 1.0, -0.5, 0.0)), "GeneralShape4D get_support_point should combine the base box and the curve.");
	// A zero direction has no meaningful support point, but it should still be finite.
	CHECK_MESSAGE(cylinder->get_support_point(Vector4()).is_finite(), "GeneralShape4D get_support_point should not return NaN for a zero direction.");

	// A rounded box, which is a base box with a 4D sphere curve on all axes.
	Ref<GeneralShapeCurve4D> hypersphere_curve;
	hypersphere_curve.instantiate();
	hypersphere_curve->set_radii(Vector4(0.5, 0.5, 0.5, 0.5));
	curves.clear();
	curves.push_back(hypersphere_curve);
	Ref<GeneralShape4D> rounded_box;
	rounded_box.instantiate();
	rounded_box->set_base_half_extents(Vector4(1.0, 2.0, 3.0, 4.0));
	rounded_box->set_curves(curves);
	// Along a pure axis, the whole face of the base box is equally far, so check the absolute values.
	support = rounded_box->get_support_point(Vector4(0.0, 0.0, 1.0, 0.0));
	CHECK_MESSAGE(support.is_finite(), "GeneralShape4D get_support_point should not return NaN along a pure axis.");
	CHECK_MESSAGE(support.z == doctest::Approx(3.5), "GeneralShape4D get_support_point should add the curve's radius along a pure axis.");
	CHECK_MESSAGE(support.abs().is_equal_approx(Vector4(1.0, 2.0, 3.5, 4.0)), "GeneralShape4D get_support_point should return a point on the base box's face along a pure axis, with no curve offset on the other axes.");
	support = rounded_box->get_support_point(Vector4(0.0, -1.0, 0.0, 0.0));
	CHECK_MESSAGE(support.y == doctest::Approx(-2.5), "GeneralShape4D get_support_point should add the curve's radius along a pure negative axis.");
	CHECK_MESSAGE(support.abs().is_equal_approx(Vector4(1.0, 2.5, 3.0, 4.0)), "GeneralShape4D get_support_point should return a point on the base box's face along a pure axis, with no curve offset on the other axes.");
	support = rounded_box->get_support_point(Vector4(1.0, -1.0, 1.0, -1.0));
	CHECK_MESSAGE(support.is_equal_approx(Vector4(1.25, -2.25, 3.25, -4.25)), "GeneralShape4D get_support_point should return the rounded corner along a diagonal.");
}

TEST_CASE("[GeneralShape4D] Get Support Point with custom exponents") {
	Ref<GeneralShapeCurve4D> curve;
	curve.instantiate();
	TypedArray<GeneralShapeCurve4D> curves;
	curves.push_back(curve);
	Ref<GeneralShape4D> shape;
	shape.instantiate();
	shape->set_curves(curves);
	// A superellipse with an exponent of 4. These values were verified with a brute force search of the surface.
	curve->set_radii(Vector4(1.0, 1.0, 0.0, 0.0));
	curve->set_exponent(4.0);
	Vector4 support = shape->get_support_point(Vector4(1.0, 0.5, 0.0, 0.0));
	CHECK_MESSAGE(support.is_equal_approx(Vector4(0.919840519, 0.730077904, 0.0, 0.0)), "GeneralShape4D get_support_point should be exact for an exponent of 4.");
	CHECK_MESSAGE(support.dot(Vector4(1.0, 0.5, 0.0, 0.0)) == doctest::Approx(1.284879471), "GeneralShape4D get_support_point should be exact for an exponent of 4.");
	support = shape->get_support_point(Vector4(-1.0, 0.5, 0.0, 0.0));
	CHECK_MESSAGE(support.is_equal_approx(Vector4(-0.919840519, 0.730077904, 0.0, 0.0)), "GeneralShape4D get_support_point should be exact for negative direction components.");
	// Ellipsoidal superellipses on non-adjacent axes, including a direction component on an unused axis.
	curve->set_radii(Vector4(2.0, 0.0, 0.5, 0.0));
	support = shape->get_support_point(Vector4(-0.3, 0.7, -1.2, 0.0));
	CHECK_MESSAGE(support.is_equal_approx(Vector4(-1.681792831, 0.0, -0.420448208, 0.0)), "GeneralShape4D get_support_point should be exact for ellipsoidal curves with an exponent of 4.");
	curve->set_radii(Vector4(0.0, 1.0, 0.0, 2.0));
	curve->set_exponent(3.0);
	support = shape->get_support_point(Vector4(0.0, 0.4, 0.0, -0.7));
	CHECK_MESSAGE(support.is_equal_approx(Vector4(0.0, 0.509789961, 0.0, -1.907459375)), "GeneralShape4D get_support_point should be exact for odd exponents.");
	curve->set_radii(Vector4(1.0, 1.0, 0.0, 0.0));
	curve->set_exponent(1.5);
	support = shape->get_support_point(Vector4(1.0, 0.5, 0.0, 0.0));
	CHECK_MESSAGE(support.is_equal_approx(Vector4(0.924481699, 0.231120425, 0.0, 0.0)), "GeneralShape4D get_support_point should be exact for exponents between 1 and 2.");
	// The support point of the curve is added to the support point of the base box.
	shape->set_base_half_extents(Vector4(0.5, 0.25, 0.0, 0.0));
	curve->set_exponent(4.0);
	support = shape->get_support_point(Vector4(1.0, 0.5, 0.0, 0.0));
	CHECK_MESSAGE(support.is_equal_approx(Vector4(1.419840519, 0.980077904, 0.0, 0.0)), "GeneralShape4D get_support_point should add the curve to the base box for custom exponents.");
	shape->set_base_half_extents(Vector4());
	// An exponent of 1 is a diamond, so the support point is the vertex on the axis with the largest radius times direction.
	curve->set_radii(Vector4(1.0, 2.0, 0.0, 0.0));
	curve->set_exponent(1.0);
	CHECK_MESSAGE(shape->get_support_point(Vector4(1.0, 0.6, 0.0, 0.0)) == Vector4(0.0, 2.0, 0.0, 0.0), "GeneralShape4D get_support_point should return a vertex for an exponent of 1.");
	CHECK_MESSAGE(shape->get_support_point(Vector4(-1.0, -0.4, 0.0, 0.0)) == Vector4(-1.0, 0.0, 0.0, 0.0), "GeneralShape4D get_support_point should return a vertex for an exponent of 1.");
	// An exponent below 1 is not convex, so the support point is the vertex of its convex hull, the same as an exponent of 1.
	curve->set_exponent(0.5);
	CHECK_MESSAGE(shape->get_support_point(Vector4(1.0, 0.6, 0.0, 0.0)) == Vector4(0.0, 2.0, 0.0, 0.0), "GeneralShape4D get_support_point should return a vertex of the convex hull for exponents below 1.");
	CHECK_MESSAGE(shape->get_support_point(Vector4(-1.0, -0.4, 0.0, 0.0)) == Vector4(-1.0, 0.0, 0.0, 0.0), "GeneralShape4D get_support_point should return a vertex of the convex hull for exponents below 1.");
	// An exponent slightly above 1 is almost a diamond, and should not overflow or underflow into NaN or infinity.
	curve->set_exponent(1.0001);
	support = shape->get_support_point(Vector4(1.0, 0.6, 0.0, 0.0));
	CHECK_MESSAGE(support.is_finite(), "GeneralShape4D get_support_point should not return NaN or infinity for exponents close to 1.");
	CHECK_MESSAGE(support.is_equal_approx(Vector4(0.0, 2.0, 0.0, 0.0)), "GeneralShape4D get_support_point should be close to the vertex for exponents close to 1.");
	// When two axes are tied, the support point of a diamond is anywhere on the edge between them, and an exponent
	// slightly above 1 is close to the edge's midpoint.
	support = shape->get_support_point(Vector4(1.0, 0.5, 0.0, 0.0));
	CHECK_MESSAGE(support.is_finite(), "GeneralShape4D get_support_point should not return NaN or infinity for exponents close to 1.");
	CHECK_MESSAGE(support.distance_to(Vector4(0.5, 1.0, 0.0, 0.0)) < 0.001, "GeneralShape4D get_support_point should be close to the edge's midpoint for exponents close to 1.");
	// Large exponents are almost a box, and huge exponents should not overflow or underflow into NaN or infinity.
	curve->set_radii(Vector4(1.0, 2.0, 3.0, 0.5));
	curve->set_exponent(1000.0);
	support = shape->get_support_point(Vector4(1.0, -1.0, 0.5, -0.2));
	CHECK_MESSAGE(support.is_equal_approx(Vector4(0.998474724, -1.998335494, 2.996640175, -0.498088000)), "GeneralShape4D get_support_point should be exact for large exponents.");
	curve->set_exponent(1.0e300);
	support = shape->get_support_point(Vector4(1.0, -1.0, 0.5, -0.2));
	CHECK_MESSAGE(support.is_finite(), "GeneralShape4D get_support_point should not return NaN or infinity for huge exponents.");
	CHECK_MESSAGE(support.is_equal_approx(Vector4(1.0, -2.0, 3.0, -0.5)), "GeneralShape4D get_support_point should return the box corner for huge exponents.");
	// An infinite exponent is not allowed, since it can't be stored in a G4MF file.
	ERR_PRINT_OFF;
	curve->set_exponent(Math_INF);
	ERR_PRINT_ON;
	CHECK_MESSAGE(curve->get_exponent() == 1.0e300, "GeneralShapeCurve4D set_exponent should reject an infinite exponent.");
}

TEST_CASE("[GeneralShape4D] Get Support Point properties") {
	Ref<GeneralShapeCurve4D> curve;
	curve.instantiate();
	curve->set_radii(Vector4(1.0, 0.5, 2.0, 0.75));
	TypedArray<GeneralShapeCurve4D> curves;
	curves.push_back(curve);
	Ref<GeneralShape4D> shape;
	shape.instantiate();
	shape->set_curves(curves);
	Ref<GeneralShape4D> shape_with_base;
	shape_with_base.instantiate();
	shape_with_base->set_base_half_extents(Vector4(0.5, 1.0, 0.25, 0.0));
	shape_with_base->set_curves(curves);
	const Vector4 radii = curve->get_radii();
	const Vector4 base_half_extents = shape_with_base->get_base_half_extents();
	const real_t grid_values[] = { -1.0, -0.4, 0.0, 0.6, 1.0 };
	PackedVector4Array grid_directions;
	for (const real_t x : grid_values) {
		for (const real_t y : grid_values) {
			for (const real_t z : grid_values) {
				for (const real_t w : grid_values) {
					const Vector4 direction = Vector4(x, y, z, w);
					if (direction != Vector4()) {
						grid_directions.push_back(direction);
					}
				}
			}
		}
	}
	const double exponents[] = { 0.5, 1.0, 1.25, 1.5, 2.0, 3.0, 4.0, 8.0, 16.0 };
	for (const double exponent : exponents) {
		curve->set_exponent(exponent);
		// Points on the curve's surface, using the signed power parametrization: for any unit vector u,
		// the point with s_i = r_i * sign(u_i) * |u_i|^(2 / p) satisfies sum(|s_i / r_i|^p) = sum(u_i^2) = 1.
		PackedVector4Array surface_points;
		for (const Vector4 &grid_direction : grid_directions) {
			const Vector4 unit = grid_direction.normalized();
			Vector4 surface_point;
			for (int axis = 0; axis < 4; axis++) {
				surface_point[axis] = radii[axis] * SIGN(unit[axis]) * Math::pow((double)Math::abs(unit[axis]), 2.0 / exponent);
			}
			surface_points.push_back(surface_point);
		}
		for (const Vector4 &direction : grid_directions) {
			INFO("Exponent: ", exponent, ", direction: ", direction);
			const Vector4 support = shape->get_support_point(direction);
			CHECK_MESSAGE(support.is_finite(), "GeneralShape4D get_support_point should always be finite.");
			// The support point is on the curve's surface. For exponents of 1 or less, it is a vertex, which is also on the surface.
			double surface_sum = 0.0;
			for (int axis = 0; axis < 4; axis++) {
				surface_sum += Math::pow(Math::abs((double)support[axis] / radii[axis]), exponent);
			}
			CHECK_MESSAGE(surface_sum == doctest::Approx(1.0), "GeneralShape4D get_support_point should be on the curve's surface.");
			// The support point is at least as far along the direction as every point on the surface.
			real_t max_surface_distance = -Math_INF;
			for (const Vector4 &surface_point : surface_points) {
				max_surface_distance = MAX(max_surface_distance, surface_point.dot(direction));
			}
			CHECK_MESSAGE(support.dot(direction) >= max_surface_distance - (real_t)CMP_EPSILON, "GeneralShape4D get_support_point should be at least as far along the direction as every point on the surface.");
			// The base box is added to the curve. When a direction component is zero, any value on that axis is valid.
			const Vector4 base_offset = shape_with_base->get_support_point(direction) - support;
			for (int axis = 0; axis < 4; axis++) {
				if (direction[axis] != 0.0f) {
					CHECK_MESSAGE(base_offset[axis] == doctest::Approx(SIGN(direction[axis]) * base_half_extents[axis]), "GeneralShape4D get_support_point should add the base box's corner to the curve's support point.");
				}
			}
		}
	}
}

TEST_CASE("[GeneralShape4D] Get Support Point with an exponent of 2 is unchanged") {
	Ref<GeneralShapeCurve4D> curve;
	curve.instantiate();
	curve->set_radii(Vector4(0.5, 1.0, 2.0, 0.0));
	TypedArray<GeneralShapeCurve4D> curves;
	curves.push_back(curve);
	Ref<GeneralShape4D> shape;
	shape.instantiate();
	shape->set_curves(curves);
	CHECK_MESSAGE(shape->get_support_point(Vector4(1.0, 2.0, -3.0, 4.0)).is_equal_approx(Vector4(0.039405520, 0.315244162, -1.891464975, 0.0)), "GeneralShape4D get_support_point should be unchanged for ellipsoids.");
	const Vector4 radii_squared = curve->get_radii() * curve->get_radii();
	const real_t grid_values[] = { -1.0, -0.3, 0.2, 1.0 };
	for (const real_t x : grid_values) {
		for (const real_t y : grid_values) {
			for (const real_t z : grid_values) {
				for (const real_t w : grid_values) {
					const Vector4 direction = Vector4(x, y, z, w);
					INFO("Direction: ", direction);
					// The previous implementation, which is correct for an exponent of 2 when the direction is not perpendicular to the curve.
					const Vector4 expected = direction * radii_squared / Math::sqrt((direction * direction).dot(radii_squared));
					CHECK_MESSAGE(shape->get_support_point(direction).is_equal_approx(expected), "GeneralShape4D get_support_point should be unchanged for ellipsoids.");
				}
			}
		}
	}
}

TEST_CASE("[GeneralShape4D] Has Point with custom exponents") {
	Ref<GeneralShapeCurve4D> curve;
	curve.instantiate();
	curve->set_radii(Vector4(1.0, 1.0, 0.0, 0.0));
	TypedArray<GeneralShapeCurve4D> curves;
	curves.push_back(curve);
	Ref<GeneralShape4D> shape;
	shape.instantiate();
	shape->set_base_half_extents(Vector4(0.5, 0.0, 0.0, 0.25));
	shape->set_curves(curves);
	// The offset (0.75, 0.75) from the base box is outside a circle, but inside a squircle with an exponent of 4.
	curve->set_exponent(4.0);
	CHECK_MESSAGE(shape->has_point(Vector4(1.25, 0.75, 0.0, 0.0)), "GeneralShape4D has_point should use the curve's exponent.");
	CHECK_MESSAGE(shape->has_point(Vector4(-1.25, 0.75, 0.0, -0.25)), "GeneralShape4D has_point should use the curve's exponent.");
	curve->set_exponent(2.0);
	CHECK_FALSE_MESSAGE(shape->has_point(Vector4(1.25, 0.75, 0.0, 0.0)), "GeneralShape4D has_point should use the curve's exponent.");
	// A non-convex curve, where the offset (0.3, 0.3) is outside, but the offsets (0.2, 0.2) and (0.9, 0.0) are inside.
	curve->set_exponent(0.5);
	CHECK_FALSE_MESSAGE(shape->has_point(Vector4(0.8, 0.3, 0.0, 0.0)), "GeneralShape4D has_point should be exact for non-convex curves.");
	CHECK_MESSAGE(shape->has_point(Vector4(0.7, 0.2, 0.0, 0.0)), "GeneralShape4D has_point should be exact for non-convex curves.");
	CHECK_MESSAGE(shape->has_point(Vector4(1.4, 0.0, 0.0, 0.0)), "GeneralShape4D has_point should be exact for non-convex curves.");
	// Large exponents with large or small radii must not overflow or underflow into NaN, which would be treated as inside.
	shape->set_base_half_extents(Vector4());
	curve->set_radii(Vector4(2.0, 2.0, 0.0, 0.0));
	curve->set_exponent(1100.0);
	CHECK_FALSE_MESSAGE(shape->has_point(Vector4(2.1, 0.0, 0.0, 0.0)), "GeneralShape4D has_point should not overflow with large exponents.");
	CHECK_MESSAGE(shape->has_point(Vector4(1.9, -1.9, 0.0, 0.0)), "GeneralShape4D has_point should not overflow with large exponents.");
	curve->set_radii(Vector4(0.01, 0.01, 0.0, 0.0));
	curve->set_exponent(200.0);
	CHECK_FALSE_MESSAGE(shape->has_point(Vector4(0.02, 0.0, 0.0, 0.0)), "GeneralShape4D has_point should not underflow with small radii.");
	CHECK_MESSAGE(shape->has_point(Vector4(0.009, -0.009, 0.0, 0.0)), "GeneralShape4D has_point should not underflow with small radii.");
	CHECK_FALSE_MESSAGE(shape->has_point(Vector4(0.0, 0.0, 0.001, 0.0)), "GeneralShape4D has_point should return false for points off of the curve's axes.");
}

TEST_CASE("[GeneralShape4D] Get Nearest Point with custom exponents") {
	// The nearest point is an approximation for custom exponents, so it prints warnings.
	GeneralShape4D::set_warnings_enabled(false);
	Ref<GeneralShapeCurve4D> curve;
	curve.instantiate();
	curve->set_radii(Vector4(1.0, 1.0, 0.0, 0.0));
	TypedArray<GeneralShapeCurve4D> curves;
	curves.push_back(curve);
	Ref<GeneralShape4D> shape;
	shape.instantiate();
	shape->set_curves(curves);
	const double exponents[] = { 1.5, 4.0 };
	const Vector4 points[] = {
		Vector4(2.0, 1.5, 0.0, 0.0),
		Vector4(-2.0, 1.5, 0.0, 0.0),
		Vector4(0.3, -4.0, 0.0, 0.0),
		Vector4(-1.0, -1.0, 0.0, 0.0),
	};
	for (const double exponent : exponents) {
		curve->set_exponent(exponent);
		for (const Vector4 &point : points) {
			INFO("Exponent: ", exponent, ", point: ", point);
			const Vector4 nearest_point = shape->get_nearest_point(point);
			const double surface_sum = Math::pow(Math::abs((double)nearest_point.x), exponent) + Math::pow(Math::abs((double)nearest_point.y), exponent);
			CHECK_MESSAGE(surface_sum == doctest::Approx(1.0), "GeneralShape4D get_nearest_point should return a point on the curve's surface for custom exponents.");
			// Rounding may put a point exactly on the surface on either side of it, so check points slightly inside and outside instead.
			CHECK_MESSAGE(shape->has_point(nearest_point * (real_t)0.9999), "GeneralShape4D get_nearest_point should return a point on the surface, so a point slightly closer to the center is inside the shape.");
			CHECK_FALSE_MESSAGE(shape->has_point(nearest_point * (real_t)1.0001), "GeneralShape4D get_nearest_point should return a point on the surface, so a point slightly farther from the center is outside the shape.");
			CHECK_MESSAGE(point.sign() == nearest_point.sign(), "GeneralShape4D get_nearest_point should stay in the same orthant as the point.");
		}
	}
	// These values are the point scaled to have a length of 1 with the curve's exponent.
	curve->set_exponent(4.0);
	CHECK_MESSAGE(shape->get_nearest_point(Vector4(2.0, 1.5, 0.0, 0.0)).is_equal_approx(Vector4(0.933582100, 0.700186575, 0.0, 0.0)), "GeneralShape4D get_nearest_point should return a point on the curve's surface for an exponent of 4.");
	curve->set_exponent(1.5);
	CHECK_MESSAGE(shape->get_nearest_point(Vector4(2.0, 1.5, 0.0, 0.0)).is_equal_approx(Vector4(0.716300261, 0.537225195, 0.0, 0.0)), "GeneralShape4D get_nearest_point should return a point on the curve's surface for an exponent of 1.5.");
	// With a base box, the curve is offset by the box, and points inside the box's extents on an axis keep their value.
	shape->set_base_half_extents(Vector4(1.0, 0.0, 0.5, 0.0));
	curve->set_exponent(4.0);
	const Vector4 nearest_point = shape->get_nearest_point(Vector4(3.0, 1.5, 0.2, 0.0));
	CHECK_MESSAGE(nearest_point.is_equal_approx(Vector4(1.933582100, 0.700186575, 0.2, 0.0)), "GeneralShape4D get_nearest_point should add the base box to the curve for custom exponents.");
	CHECK_MESSAGE(shape->has_point(nearest_point * (real_t)0.9999), "GeneralShape4D get_nearest_point should return a point on the surface, so a point slightly closer to the center is inside the shape.");
	CHECK_FALSE_MESSAGE(shape->has_point(nearest_point * (real_t)1.0001), "GeneralShape4D get_nearest_point should return a point on the surface, so a point slightly farther from the center is outside the shape.");
	// Small radii with a large exponent must not underflow into NaN, which would be treated as inside.
	shape->set_base_half_extents(Vector4());
	curve->set_radii(Vector4(0.01, 0.01, 0.0, 0.0));
	curve->set_exponent(200.0);
	CHECK_MESSAGE(shape->get_nearest_point(Vector4(0.02, 0.0, 0.0, 0.0)).is_equal_approx(Vector4(0.01, 0.0, 0.0, 0.0)), "GeneralShape4D get_nearest_point should not underflow with small radii.");
	GeneralShape4D::set_warnings_enabled(true);
}
} // namespace TestGeneralShape4D
