#pragma once

#include "../../math/transform_4d.h"
#include "tests/test_macros.h"

namespace TestTransform4D {
TEST_CASE("[Transform4D] Conversion") {
	Transform4D transform_4d = Transform4D(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21);
	Projection basis_4d_projection = transform_4d.basis;
	CHECK_MESSAGE(Basis4D(basis_4d_projection) == transform_4d.basis, "Transform4D's Basis4D should be implicitly convertible to/from Projection.");

	Transform3D transform_3d = transform_4d.to_3d();
	// Remember that Godot's Transform stores rows for low-level optimization reasons.
	CHECK_MESSAGE(transform_3d == Transform3D(Basis(2, 6, 10, 3, 7, 11, 4, 8, 12), Vector3(18, 19, 20)), "Transform4D to_3d should work as expected.");
	Transform4D transform_no_w = Transform4D(2, 3, 4, 0, 6, 7, 8, 0, 10, 11, 12, 0, 0, 0, 0, 1, 18, 19, 20, 0);
	CHECK_MESSAGE(Transform4D::from_3d(transform_3d) == transform_no_w, "Transform4D from_3d should work as expected.");
}

TEST_CASE("[Transform4D] Precomputed Inversion") {
	Transform4D ones = Transform4D(1, 1, 1, -1, 1, 1, -1, 1, 1, -1, 1, 1, -1, 1, 1, 1);
	Transform4D ones_inverse = Transform4D(0.25, 0.25, 0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, 0.25);
	CHECK_MESSAGE(ones_inverse.is_equal_approx(ones.inverse()), "Transform4D inverse should work as expected.");

	const Transform4D unique = Transform4D(1.0, 0.41, 0.31, 0.21, 0.11, 1.1, 0.12, 0.22, 0.32, 0.42, 1.2, 0.43, 0.33, 0.23, 0.13, 1.3, 5, 6, 7, 8);
	const Transform4D unique_inverse = Transform4D(1.131305786134016, -0.31456736720963524, -0.2559375714833072, -0.04485864504938581, -0.03978939804196893, 0.9801066566654912, -0.07307806077708652, -0.13526471134111334, -0.19433619846736885, -0.23401553809120415, 0.9326678341313813, -0.23750242201406277, -0.2607043400567804, -0.07015021530925138, -0.015368819899121764, 0.8282996548742938, -1.97180443269244, -2.10849261483233, -4.68756805764764, -3.9279987916023);
	CHECK_MESSAGE(unique_inverse.origin.is_equal_approx(unique.inverse().origin), "Transform4D inverse should work as expected.");
	CHECK_MESSAGE(unique_inverse.is_equal_approx(unique.inverse()), "Transform4D inverse should work as expected.");
	CHECK_MESSAGE(unique.is_equal_approx(unique_inverse.inverse()), "Transform4D inverse of inverse should be the original transform.");
}

TEST_CASE("[Transform4D] Random Inversion") {
	RandomNumberGenerator rng;
	for (int i = 0; i < 100; i++) {
		Transform4D transform = Transform4D();
		for (int j = 0; j < 20; j++) {
			transform[j / 4][j % 4] = rng.randf_range(-1.0, 1.0);
		}
		// Non-orthogonalized transforms may be inverted, but the result will
		// have a high amount of floating-point error. The pre-computed tests
		// have an example of a non-orthogonal transform, for these random tests
		// we only want to test orthogonalized transforms. If you try commenting
		// out this line many test runs will fail by a slight amount.
		transform.basis.orthogonalize();
		const real_t determinant = transform.determinant();
		if (-CMP_EPSILON < determinant && determinant < CMP_EPSILON) {
			// For a near-zero determinant, the transform is not invertible.
			continue;
		}
		Transform4D inverted = transform.inverse();
		CHECK_MESSAGE(inverted.determinant() == doctest::Approx(1.0f / determinant), "Transform4D inverse should have an inverse determinant.");
		CHECK_MESSAGE(transform.is_equal_approx(inverted.inverse()), "Transform4D inverse of inverse should be the original transform.");
		CHECK_MESSAGE(transform.is_equal_approx(inverted.inverse()), String(rtos(transform.determinant()) + String(" != ") + rtos(inverted.determinant())).utf8());
		CHECK_MESSAGE(Transform4D().is_equal_approx(transform * inverted), "Transform4D transform times inverted should be the identity transform.");
		CHECK_MESSAGE(Transform4D().is_equal_approx(inverted * transform), "Transform4D inverted times transform should be the identity transform.");
	}
}
} // namespace TestTransform4D
