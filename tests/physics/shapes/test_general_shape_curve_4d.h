#pragma once

#include "../../../physics/shapes/general_shape_4d.h"
#include "../../../physics/shapes/general_shape_curve_4d.h"

#include "tests/test_macros.h"

namespace TestGeneralShapeCurve4D {
TEST_CASE("[GeneralShapeCurve4D] Is Equal Exact with tapering") {
	GeneralShapeCurveTaperPoint4D taper_point_a;
	taper_point_a.position = Vector4(0.0, 1.0, 0.0, 0.0);
	taper_point_a.radii = Vector4(0.5, 0.0, 0.5, 0.0);
	GeneralShapeCurveTaperPoint4D taper_point_b;
	taper_point_b.position = Vector4(0.0, -1.0, 0.0, 0.0);
	taper_point_b.radii = Vector4(0.25, 0.0, 0.25, 0.0);
	taper_point_b.exponent = 4.0;
	Vector<GeneralShapeCurveTaperPoint4D> two_taper_points;
	two_taper_points.push_back(taper_point_a);
	two_taper_points.push_back(taper_point_b);
	Vector<GeneralShapeCurveTaperPoint4D> one_taper_point;
	one_taper_point.push_back(taper_point_a);

	Ref<GeneralShapeCurve4D> curve;
	curve.instantiate();
	curve->set_radii(Vector4(1.0, 0.0, 1.0, 0.0));
	Ref<GeneralShapeCurve4D> other_curve;
	other_curve.instantiate();
	other_curve->set_radii(Vector4(1.0, 0.0, 1.0, 0.0));
	CHECK_MESSAGE(curve->is_equal_exact(other_curve), "GeneralShapeCurve4D is_equal_exact should return true for equal curves without tapering.");
	CHECK_FALSE_MESSAGE(curve->is_equal_exact(Ref<GeneralShapeCurve4D>()), "GeneralShapeCurve4D is_equal_exact should return false for a null curve.");

	// Different taper counts must be unequal in both orders, without reading past the end of the shorter taper array.
	curve->set_taper(two_taper_points);
	other_curve->set_taper(one_taper_point);
	CHECK_FALSE_MESSAGE(curve->is_equal_exact(other_curve), "GeneralShapeCurve4D is_equal_exact should return false when this curve has more taper points.");
	CHECK_FALSE_MESSAGE(other_curve->is_equal_exact(curve), "GeneralShapeCurve4D is_equal_exact should return false when the other curve has more taper points.");
	other_curve->set_taper(Vector<GeneralShapeCurveTaperPoint4D>());
	CHECK_FALSE_MESSAGE(curve->is_equal_exact(other_curve), "GeneralShapeCurve4D is_equal_exact should return false when only this curve has tapering.");
	CHECK_FALSE_MESSAGE(other_curve->is_equal_exact(curve), "GeneralShapeCurve4D is_equal_exact should return false when only the other curve has tapering.");

	// Equal taper points are equal, and any difference in a taper point is unequal.
	other_curve->set_taper(two_taper_points);
	CHECK_MESSAGE(curve->is_equal_exact(other_curve), "GeneralShapeCurve4D is_equal_exact should return true for curves with equal tapering.");
	CHECK_MESSAGE(other_curve->is_equal_exact(curve), "GeneralShapeCurve4D is_equal_exact should return true for curves with equal tapering.");
	Vector<GeneralShapeCurveTaperPoint4D> different_taper_points = two_taper_points;
	different_taper_points.write[1].exponent = 2.0;
	other_curve->set_taper(different_taper_points);
	CHECK_FALSE_MESSAGE(curve->is_equal_exact(other_curve), "GeneralShapeCurve4D is_equal_exact should return false when a taper point is different.");
	CHECK_FALSE_MESSAGE(other_curve->is_equal_exact(curve), "GeneralShapeCurve4D is_equal_exact should return false when a taper point is different.");

	// The shape comparison uses the curve comparison, so it must also handle different taper counts.
	other_curve->set_taper(one_taper_point);
	TypedArray<GeneralShapeCurve4D> curves;
	curves.push_back(curve);
	TypedArray<GeneralShapeCurve4D> other_curves;
	other_curves.push_back(other_curve);
	Ref<GeneralShape4D> shape;
	shape.instantiate();
	shape->set_curves(curves);
	Ref<GeneralShape4D> other_shape;
	other_shape.instantiate();
	other_shape->set_curves(other_curves);
	CHECK_FALSE_MESSAGE(shape->is_equal_exact(other_shape), "GeneralShape4D is_equal_exact should return false when a curve has a different taper count.");
	CHECK_FALSE_MESSAGE(other_shape->is_equal_exact(shape), "GeneralShape4D is_equal_exact should return false when a curve has a different taper count.");
}
} // namespace TestGeneralShapeCurve4D
