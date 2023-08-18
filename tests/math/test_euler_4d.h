#pragma once

#include "../../math/euler_4d.h"
#include "core/math/random_number_generator.h"
#include "tests/test_macros.h"

namespace TestEuler4D {
TEST_CASE("[Euler4D] Basis4D to Euler4D round-trip conversions") {
	RandomNumberGenerator rng;

	// Identity basis should have identity rotation.
	Basis4D basis1 = Basis4D();
	Euler4D euler = Euler4D::from_basis(basis1);
	Basis4D basis2 = euler.to_basis();
	CHECK_MESSAGE(basis1.is_equal_approx(basis2), "Basis4D to Euler4D round-trip conversion should give the same output as the input.");
	CHECK_MESSAGE(euler.is_equal_approx(Euler4D()), "Basis4D to Euler4D conversion should give the expected angles.");

	// Rotation of 90 degrees on the YZ plane.
	basis1 = Basis4D();
	basis1.y.y = basis1.z.z = 0;
	basis1.y.z = 1;
	basis1.z.y = -1;
	euler = Euler4D::from_basis(basis1);
	basis2 = euler.to_basis();
	CHECK_MESSAGE(basis1.is_equal_approx(basis2), "Basis4D to Euler4D round-trip conversion should give the same output as the input.");
	CHECK_MESSAGE(euler.is_equal_approx(Euler4D(Math_TAU / 4, 0, 0)), "Basis4D to Euler4D conversion should give the expected angles.");

	// Rotation of 90 degrees on the ZX plane.
	basis1 = Basis4D();
	basis1.z.z = basis1.x.x = 0;
	basis1.z.x = 1;
	basis1.x.z = -1;
	euler = Euler4D::from_basis(basis1);
	basis2 = euler.to_basis();
	CHECK_MESSAGE(basis1.is_equal_approx(basis2), "Basis4D to Euler4D round-trip conversion should give the same output as the input.");
	CHECK_MESSAGE(euler.is_equal_approx(Euler4D(0, Math_TAU / 4, 0)), "Basis4D to Euler4D conversion should give the expected angles.");

	// Rotation of 90 degrees on the XY plane.
	basis1 = Basis4D();
	basis1.x.x = basis1.y.y = 0;
	basis1.x.y = 1;
	basis1.y.x = -1;
	euler = Euler4D::from_basis(basis1);
	basis2 = euler.to_basis();
	CHECK_MESSAGE(basis1.is_equal_approx(basis2), "Basis4D to Euler4D round-trip conversion should give the same output as the input.");
	CHECK_MESSAGE(euler.is_equal_approx(Euler4D(0, 0, Math_TAU / 4)), "Basis4D to Euler4D conversion should give the expected angles.");

	// Rotation of 90 degrees on the XW plane.
	basis1 = Basis4D();
	basis1.x.x = basis1.w.w = 0;
	basis1.x.w = 1;
	basis1.w.x = -1;
	euler = Euler4D::from_basis(basis1);
	basis2 = euler.to_basis();
	CHECK_MESSAGE(basis1.is_equal_approx(basis2), "Basis4D to Euler4D round-trip conversion should give the same output as the input.");
	CHECK_MESSAGE(euler.is_equal_approx(Euler4D(0, 0, 0, Math_TAU / 4, 0, 0)), "Basis4D to Euler4D conversion should give the expected angles.");

	// Rotation of 90 degrees on the WY plane.
	basis1 = Basis4D();
	basis1.w.w = basis1.y.y = 0;
	basis1.w.y = 1;
	basis1.y.w = -1;
	euler = Euler4D::from_basis(basis1);
	basis2 = euler.to_basis();
	CHECK_MESSAGE(basis1.is_equal_approx(basis2), "Basis4D to Euler4D round-trip conversion should give the same output as the input.");
	CHECK_MESSAGE(euler.is_equal_approx(Euler4D(0, 0, 0, 0, Math_TAU / 4, 0)), "Basis4D to Euler4D conversion should give the expected angles.");

	// Rotation of 90 degrees on the ZW plane.
	basis1 = Basis4D();
	basis1.z.z = basis1.w.w = 0;
	basis1.z.w = 1;
	basis1.w.z = -1;
	euler = Euler4D::from_basis(basis1);
	basis2 = euler.to_basis();
	CHECK_MESSAGE(basis1.is_equal_approx(basis2), "Basis4D to Euler4D round-trip conversion should give the same output as the input.");
	CHECK_MESSAGE(euler.is_equal_approx(Euler4D(0, 0, 0, 0, 0, Math_TAU / 4)), "Basis4D to Euler4D conversion should give the expected angles.");
}

TEST_CASE("[Euler4D] Random Euler4D to Basis4D round-trip conversions") {
	RandomNumberGenerator rng;
	for (int i = 0; i < 100; i++) {
		Euler4D euler = Euler4D(
				// We use 3.0 and 1.5 instead of PI or TAU because we want to
				// avoid testing singularities in a random test. Singularities
				// may cause different decompositions even though the rotations
				// are the same, so they should get tested separately.
				rng.randf_range(-1.5, 1.5), // YZ, Pitch
				rng.randf_range(-3.0, 3.0), // XZ, Yaw 1
				rng.randf_range(-1.5, 1.5), // XY, Roll 1
				rng.randf_range(-1.5, 1.5), // XW, Yaw 3
				rng.randf_range(-3.0, 3.0), // WY, Roll 2
				rng.randf_range(-1.5, 1.5)); // ZW, Yaw 2
		Basis4D basis = euler.to_basis();
		// False because we want to avoid testing special cases in a random test.
		Euler4D euler2 = Euler4D::from_basis(basis, false);
		CHECK_MESSAGE(euler.is_equal_approx(euler2), "Euler4D to Basis4D round-trip conversion should work as expected.");
		Basis4D basis2 = euler2.to_basis();
		CHECK_MESSAGE(basis.is_equal_approx(basis2), "Basis4D to Euler4D round-trip conversion should give the same output as the input.");
	}
}

TEST_CASE("[Euler4D] Rotation To") {
	Euler4D euler_zero = Euler4D();
	// Trivial 3D-only cases.
	Euler4D euler_yz_90 = Euler4D(Math_TAU / 4, 0, 0);
	Euler4D euler_zx_90 = Euler4D(0, Math_TAU / 4, 0);
	Euler4D euler_xy_90 = Euler4D(0, 0, Math_TAU / 4);
	CHECK_MESSAGE(euler_zero.rotation_to(euler_zero).is_equal_approx(euler_zero), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_zero.rotation_to(euler_yz_90).is_equal_approx(euler_yz_90), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_zero.rotation_to(euler_zx_90).is_equal_approx(euler_zx_90), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_zero.rotation_to(euler_xy_90).is_equal_approx(euler_xy_90), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_yz_90.rotation_to(euler_zero).is_equal_approx(-euler_yz_90), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_zx_90.rotation_to(euler_zero).is_equal_approx(-euler_zx_90), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_xy_90.rotation_to(euler_zero).is_equal_approx(-euler_xy_90), "Euler4D rotation_to should work as expected.");

	// Trivial cases with the W axis.
	Euler4D euler_xw_90 = Euler4D(0, 0, 0, Math_TAU / 4, 0, 0);
	Euler4D euler_wy_90 = Euler4D(0, 0, 0, 0, Math_TAU / 4, 0);
	Euler4D euler_zw_90 = Euler4D(0, 0, 0, 0, 0, Math_TAU / 4);
	CHECK_MESSAGE(euler_zero.rotation_to(euler_xw_90).is_equal_approx(euler_xw_90), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_zero.rotation_to(euler_wy_90).is_equal_approx(euler_wy_90), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_zero.rotation_to(euler_zw_90).is_equal_approx(euler_zw_90), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_xw_90.rotation_to(euler_zero).is_equal_approx(-euler_xw_90), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_wy_90.rotation_to(euler_zero).is_equal_approx(-euler_wy_90), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_zw_90.rotation_to(euler_zero).is_equal_approx(-euler_zw_90), "Euler4D rotation_to should work as expected.");

	// More tests in the ZX plane.
	Euler4D euler_zx_110 = Euler4D(0, 1.91986217719376253, 0);
	Euler4D euler_zx_200 = Euler4D(0, 3.49065850398865915, 0);
	Euler4D euler_zx_neg160 = Euler4D(0, -2.79252680319092732, 0);
	CHECK_MESSAGE(euler_zx_200.wrapped().is_equal_approx(euler_zx_neg160), "Euler4D wrapped should work as expected.");
	CHECK_MESSAGE(euler_zx_90.rotation_to(euler_zx_200).is_equal_approx(euler_zx_110), "Euler4D rotation_to should work as expected.");
	CHECK_MESSAGE(euler_zx_200.rotation_to(euler_zx_90).is_equal_approx(-euler_zx_110), "Euler4D rotation_to should work as expected.");

	// Rotational holonomy - none.
	Euler4D euler_yz_10 = Euler4D(0.174532925199432957692369, 0, 0, 0, 0, 0);
	Euler4D euler_zx_10 = Euler4D(0, 0.174532925199432957692369, 0, 0, 0, 0);
	Euler4D euler_xy_10 = Euler4D(0, 0, 0.174532925199432957692369, 0, 0, 0);
	Euler4D euler_xw_10 = Euler4D(0, 0, 0, 0.174532925199432957692369, 0, 0);
	Euler4D euler_wy_10 = Euler4D(0, 0, 0, 0, 0.174532925199432957692369, 0);
	Euler4D euler_zw_10 = Euler4D(0, 0, 0, 0, 0, 0.174532925199432957692369);
	CHECK_MESSAGE(euler_xy_10.rotation_to(euler_zw_10).is_equal_approx(euler_zw_10 - euler_xy_10), "Euler4D rotation_to with perpendicular rotations should have no holonomy.");
	CHECK_MESSAGE(euler_zw_10.rotation_to(euler_xy_10).is_equal_approx(euler_xy_10 - euler_zw_10), "Euler4D rotation_to with perpendicular rotations should have no holonomy.");
	CHECK_MESSAGE(euler_yz_10.rotation_to(euler_xw_10).is_equal_approx(euler_xw_10 - euler_yz_10), "Euler4D rotation_to with perpendicular rotations should have no holonomy.");
	CHECK_MESSAGE(euler_xw_10.rotation_to(euler_yz_10).is_equal_approx(euler_yz_10 - euler_xw_10), "Euler4D rotation_to with perpendicular rotations should have no holonomy.");
	CHECK_MESSAGE(euler_zx_10.rotation_to(euler_wy_10).is_equal_approx(euler_wy_10 - euler_zx_10), "Euler4D rotation_to with perpendicular rotations should have no holonomy.");
	CHECK_MESSAGE(euler_wy_10.rotation_to(euler_zx_10).is_equal_approx(euler_zx_10 - euler_wy_10), "Euler4D rotation_to with perpendicular rotations should have no holonomy.");

	// Rotational holonomy - yaw.
	Euler4D euler_xw_to_zw = euler_xw_10.rotation_to(euler_zw_10);
	CHECK_MESSAGE(euler_xw_to_zw.zx == doctest::Approx(0.03060929571074), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_zw_10.is_equal_approx(euler_xw_10.compose(euler_xw_to_zw)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_zw_to_xw = euler_zw_10.rotation_to(euler_xw_10);
	CHECK_MESSAGE(euler_zw_to_xw.zx == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_xw_10.is_equal_approx(euler_zw_10.compose(euler_zw_to_xw)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_xw_to_zx = euler_xw_10.rotation_to(euler_zx_10);
	CHECK_MESSAGE(euler_xw_to_zx.zw == doctest::Approx(-0.03015826099296), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_zx_10.is_equal_approx(euler_xw_10.compose(euler_xw_to_zx)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_zx_to_xw = euler_zx_10.rotation_to(euler_xw_10);
	CHECK_MESSAGE(euler_zx_to_xw.zw == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_xw_10.is_equal_approx(euler_zx_10.compose(euler_zx_to_xw)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_zw_to_zx = euler_zw_10.rotation_to(euler_zx_10);
	CHECK_MESSAGE(euler_zw_to_zx.xw == doctest::Approx(0.03060929571074), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_zx_10.is_equal_approx(euler_zw_10.compose(euler_zw_to_zx)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_zx_to_zw = euler_zx_10.rotation_to(euler_zw_10);
	CHECK_MESSAGE(euler_zx_to_zw.xw == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_zw_10.is_equal_approx(euler_zx_10.compose(euler_zx_to_zw)), "Euler4D rotation_to should rotate between the inputs.");

	// Rotational holonomy - pitch.
	Euler4D euler_yz_to_zx = euler_yz_10.rotation_to(euler_zx_10);
	CHECK_MESSAGE(euler_yz_to_zx.xy == doctest::Approx(-0.03060929571074), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_zx_10.is_equal_approx(euler_yz_10.compose(euler_yz_to_zx)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_zx_to_yz = euler_zx_10.rotation_to(euler_yz_10);
	CHECK_MESSAGE(euler_zx_to_yz.xy == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_yz_10.is_equal_approx(euler_zx_10.compose(euler_zx_to_yz)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_wy_to_yz = euler_wy_10.rotation_to(euler_yz_10);
	CHECK_MESSAGE(euler_wy_to_yz.zw == doctest::Approx(-0.03060929571074), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_yz_10.is_equal_approx(euler_wy_10.compose(euler_wy_to_yz)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_yz_to_wy = euler_yz_10.rotation_to(euler_wy_10);
	CHECK_MESSAGE(euler_yz_to_wy.zw == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_wy_10.is_equal_approx(euler_yz_10.compose(euler_yz_to_wy)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_yz_to_zw = euler_yz_10.rotation_to(euler_zw_10);
	CHECK_MESSAGE(euler_yz_to_zw.wy == doctest::Approx(-0.03060929571074), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_zw_10.is_equal_approx(euler_yz_10.compose(euler_yz_to_zw)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_zw_to_yz = euler_zw_10.rotation_to(euler_yz_10);
	CHECK_MESSAGE(euler_zw_to_yz.wy == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_yz_10.is_equal_approx(euler_zw_10.compose(euler_zw_to_yz)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_xy_to_yz = euler_xy_10.rotation_to(euler_yz_10);
	CHECK_MESSAGE(euler_xy_to_yz.zx == doctest::Approx(-0.03060929571074), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_yz_10.is_equal_approx(euler_xy_10.compose(euler_xy_to_yz)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_yz_to_xy = euler_yz_10.rotation_to(euler_xy_10);
	CHECK_MESSAGE(euler_yz_to_xy.zx == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_xy_10.is_equal_approx(euler_yz_10.compose(euler_yz_to_xy)), "Euler4D rotation_to should rotate between the inputs.");

	// Rotational holonomy - roll.
	Euler4D euler_wy_to_xy = euler_wy_10.rotation_to(euler_xy_10);
	CHECK_MESSAGE(euler_wy_to_xy.xw == doctest::Approx(0.03060929571074), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_xy_10.is_equal_approx(euler_wy_10.compose(euler_wy_to_xy)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_xy_to_wy = euler_xy_10.rotation_to(euler_wy_10);
	CHECK_MESSAGE(euler_xy_to_wy.xw == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_wy_10.is_equal_approx(euler_xy_10.compose(euler_xy_to_wy)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_xy_to_xw = euler_xy_10.rotation_to(euler_xw_10);
	CHECK_MESSAGE(euler_xy_to_xw.wy == doctest::Approx(0.03060929571074), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_xw_10.is_equal_approx(euler_xy_10.compose(euler_xy_to_xw)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_xw_to_xy = euler_xw_10.rotation_to(euler_xy_10);
	CHECK_MESSAGE(euler_xw_to_xy.wy == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_xy_10.is_equal_approx(euler_xw_10.compose(euler_xw_to_xy)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_xy_to_zx = euler_xy_10.rotation_to(euler_zx_10);
	CHECK_MESSAGE(euler_xy_to_zx.yz == doctest::Approx(0.03015826099296), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_zx_10.is_equal_approx(euler_xy_10.compose(euler_xy_to_zx)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_zx_to_xy = euler_zx_10.rotation_to(euler_xy_10);
	CHECK_MESSAGE(euler_zx_to_xy.yz == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_xy_10.is_equal_approx(euler_zx_10.compose(euler_zx_to_xy)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_wy_to_xw = euler_wy_10.rotation_to(euler_xw_10);
	CHECK_MESSAGE(euler_wy_to_xw.xy == doctest::Approx(-0.03015826099296), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_xw_10.is_equal_approx(euler_wy_10.compose(euler_wy_to_xw)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_xw_to_wy = euler_xw_10.rotation_to(euler_wy_10);
	CHECK_MESSAGE(euler_xw_to_wy.xy == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_wy_10.is_equal_approx(euler_xw_10.compose(euler_xw_to_wy)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_wy_to_zw = euler_wy_10.rotation_to(euler_zw_10);
	CHECK_MESSAGE(euler_wy_to_zw.yz == doctest::Approx(0.03015826099296), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_zw_10.is_equal_approx(euler_wy_10.compose(euler_wy_to_zw)), "Euler4D rotation_to should rotate between the inputs.");

	Euler4D euler_zw_to_wy = euler_zw_10.rotation_to(euler_wy_10);
	CHECK_MESSAGE(euler_zw_to_wy.yz == doctest::Approx(0.0), "Euler4D rotation_to should have the expected holonomy.");
	CHECK_MESSAGE(euler_wy_10.is_equal_approx(euler_zw_10.compose(euler_zw_to_wy)), "Euler4D rotation_to should rotate between the inputs.");
}
} // namespace TestEuler4D
