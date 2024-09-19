#pragma once

#include "../../math/basis_4d.h"
#include "tests/test_macros.h"

namespace TestBasis4D {
TEST_CASE("[Basis4D] Determinant") {
	const Basis4D identity = Basis4D();
	const Basis4D basis2345 = Basis4D::from_scale(Vector4(2, 3, 4, 5));
	const Basis4D rotation = Basis4D::from_xy(0.5f);
	const Basis4D flip = Basis4D::from_scale(Vector4(1, 1, 1, -1));
	CHECK_MESSAGE(identity.determinant() == 1.0f, "Basis4D determinant of identity should be 1.");
	CHECK_MESSAGE(basis2345.determinant() == 120.0f, "Basis4D determinant should work as expected.");
	CHECK_MESSAGE(rotation.determinant() == doctest::Approx(1.0f), "Basis4D determinant of only rotation should be 1.");
	CHECK_MESSAGE(flip.determinant() == -1.0f, "Basis4D determinant of only flip should be -1.");
}

TEST_CASE("[Basis4D] Conversion") {
	Basis4D basis_4d = Basis4D(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
	Projection basis_4d_projection = basis_4d;
	CHECK_MESSAGE(Basis4D(basis_4d_projection) == basis_4d, "Basis4D should be implicitly convertible to/from Projection.");

	Basis basis_3d = basis_4d.to_3d();
	// Remember that Godot's Basis stores rows for low-level optimization reasons.
	CHECK_MESSAGE(basis_3d == Basis(2, 6, 10, 3, 7, 11, 4, 8, 12), "Basis4D to_3d should work as expected.");
	Basis4D basis_no_w = Basis4D(2, 3, 4, 0, 6, 7, 8, 0, 10, 11, 12, 0, 0, 0, 0, 1);
	CHECK_MESSAGE(Basis4D::from_3d(basis_3d) == basis_no_w, "Basis4D from_3d should work as expected.");
}

TEST_CASE("[Basis4D] Rotate Bivector") {
	// Rotate by 90 degrees in the XY plane (X to Y).
	const Basis4D basis = Basis4D::from_xy(Math_TAU / 4.0);
	const Bivector4D bivector = Bivector4D(2, 3, 4, 5, 6, 7);
	const Bivector4D rotated_bivector = basis.rotate_bivector(bivector);
	// XY should not change because that's within the plane of rotation.
	// ZW should not change because that's orthogonal to the plane of rotation.
	// XZ and YZ are swapped because of the 90 degree rotation in the XY plane.
	// Similarly, XW and YW are swapped because of the 90 degree rotation in the XY plane.
	// For XY, X goes to Y when positive, so the items in the X spots (numbers 3 and 4) are just moved to the Y spots,
	// while the items in the Y spots (numbers 5 and 6) are negated and moved to the X spots (to become -5 and -6).
	CHECK_MESSAGE(rotated_bivector.is_equal_approx(Bivector4D(2, -5, -6, 3, 4, 7)), "Basis4D rotate_bivector should work as expected.");
}

TEST_CASE("[Basis4D] Scale") {
	const Basis4D basis_uniform = Basis4D::from_scale_uniform(7);
	CHECK_MESSAGE(basis_uniform.get_scale() == Vector4(7, 7, 7, 7), "Basis4D from_scale_uniform and get_scale should work as expected.");

	const Basis4D basis2345 = Basis4D::from_scale(Vector4(2, 3, 4, 5));
	const Basis4D basis2345_numbers = Basis4D::from_scale(2, 3, 4, 5);
	CHECK_MESSAGE(basis2345_numbers == basis2345, "Basis4D from_scale should work as expected.");
	CHECK_MESSAGE(basis2345.get_scale() == Vector4(2, 3, 4, 5), "Basis4D get_scale should work as expected.");

	const Basis4D flip = Basis4D::from_scale(Vector4(1, 1, 1, -1));
	CHECK_MESSAGE(flip.get_scale() == Vector4(1, 1, 1, -1), "Basis4D get_scale should work as expected for a flip.");
	CHECK_MESSAGE(flip.get_scale_abs() == Vector4(1, 1, 1, 1), "Basis4D get_scale_abs should work as expected for a flip.");

	const Basis4D basis46810 = basis2345.scaled_global(Vector4(2, 2, 2, 2));
	const Basis4D basis8985 = basis2345.scaled_local(Vector4(4, 3, 2, 1));
	CHECK_MESSAGE(basis46810 == Basis4D::from_scale(Vector4(4, 6, 8, 10)), "Basis4D scaled should work as expected.");
	CHECK_MESSAGE(basis46810.get_scale() == Vector4(4, 6, 8, 10), "Basis4D scaled should work as expected.");
	CHECK_MESSAGE(basis8985.get_scale() == Vector4(8, 9, 8, 5), "Basis4D scaled_local should work as expected.");

	const Basis4D basis_from_neg_uniform = Basis4D::from_scale_uniform(-7);
	Basis4D basis_set_neg_uniform = Basis4D();
	basis_set_neg_uniform.set_uniform_scale(-7);
	CHECK_MESSAGE(basis_from_neg_uniform.get_uniform_scale() == doctest::Approx(-7.0), "Basis4D from_scale_uniform should work as expected.");
	CHECK_MESSAGE(basis_set_neg_uniform.get_uniform_scale() == doctest::Approx(-7.0), "Basis4D set_uniform_scale should work as expected.");
	CHECK_MESSAGE(basis_from_neg_uniform.get_scale().is_equal_approx(Vector4(7, 7, 7, -7)), "Basis4D from_scale_uniform should work as expected.");
	CHECK_MESSAGE(basis_set_neg_uniform.get_scale().is_equal_approx(Vector4(7, 7, 7, -7)), "Basis4D set_uniform_scale should work as expected.");
	CHECK_MESSAGE(basis_from_neg_uniform.is_equal_approx(basis_set_neg_uniform), "Basis4D from_scale_uniform and set_uniform_scale should produce the same results.");
	basis_set_neg_uniform.set_uniform_scale(-7);
	CHECK_MESSAGE(basis_set_neg_uniform.get_uniform_scale() == doctest::Approx(-7.0), "Basis4D set_uniform_scale should preserve an existing uniform scale.");
	CHECK_MESSAGE(basis_from_neg_uniform.is_equal_approx(basis_set_neg_uniform), "Basis4D set_uniform_scale should preserve an existing uniform scale.");
	basis_set_neg_uniform.set_uniform_scale(5);
	CHECK_MESSAGE(basis_set_neg_uniform.get_uniform_scale() == doctest::Approx(5.0), "Basis4D set_uniform_scale should be able to flip a negative scale to positive.");
	CHECK_MESSAGE(basis_set_neg_uniform.get_scale() == Vector4(5, 5, 5, 5), "Basis4D set_uniform_scale should be able to flip a negative scale to positive.");
}

TEST_CASE("[Basis4D] Validation") {
	const Basis4D non_ortho = Basis4D(1.0, 0.4, 0.3, 0.2, 0.1, 1.0, 0.1, 0.2, 0.3, 0.4, 1.0, 0.4, 0.3, 0.2, 0.1, 1.0);
	CHECK_MESSAGE(non_ortho.determinant() > 0.1f, "Sanity check: Test data should not have a determinant near zero.");
	CHECK_MESSAGE(non_ortho.determinant() < 2.0f, "Sanity check: Test data should not have a big determinant or else ortho*ization will have errors too big to pass the checks.");
	CHECK_MESSAGE(non_ortho.is_orthogonal() == false, "Basis4D is_orthogonal should be false for a non-ortho basis.");
	CHECK_MESSAGE(non_ortho.is_orthonormal() == false, "Basis4D is_rotation should be false for a non-ortho basis.");
	CHECK_MESSAGE(non_ortho.is_rotation() == false, "Basis4D is_rotation should be false for a non-ortho basis.");

	const Basis4D orthogonalized = non_ortho.orthogonalized();
	CHECK_MESSAGE(orthogonalized.is_orthogonal(), "Basis4D orthogonalized should be orthogonal.");
	CHECK_MESSAGE(orthogonalized.is_orthonormal() == false, "Basis4D orthogonalized should not be orthonormal.");
	CHECK_MESSAGE(orthogonalized.is_rotation() == false, "Basis4D orthogonalized should not be a rotation.");

	const Basis4D orthonormalized = non_ortho.orthonormalized();
	CHECK_MESSAGE(orthonormalized.is_orthogonal(), "Basis4D orthonormalized should be orthogonal.");
	CHECK_MESSAGE(orthonormalized.is_orthonormal(), "Basis4D orthonormalized should be orthonormal.");
	CHECK_MESSAGE(orthonormalized.is_rotation(), "Non-flipped Basis4D orthonormalized should be a rotation.");
	CHECK_MESSAGE(orthonormalized.is_equal_approx(orthogonalized.orthonormalized()), "Orthonormalizing orthogonalized basis should be the same as orthonormalizing the original basis.");

	const Basis4D flip_skew_w = Basis4D(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0.7, -0.7);
	CHECK_MESSAGE(flip_skew_w.determinant() < 0.0f, "Basis4D determinant should be negative for a flip-rotation basis.");
	CHECK_MESSAGE(flip_skew_w.is_orthogonal() == false, "Basis4D is_orthogonal should be false for a flip-rotation basis.");
	CHECK_MESSAGE(flip_skew_w.is_orthonormal() == false, "Basis4D is_orthonormal should be false for a flip-rotation basis.");
	CHECK_MESSAGE(flip_skew_w.is_rotation() == false, "Basis4D is_rotation should be false for a flip-rotation basis.");

	const Basis4D flip_skew_orthogonal = flip_skew_w.orthogonalized();
	CHECK_MESSAGE(flip_skew_orthogonal.determinant() < 0.0f, "Basis4D determinant should be negative for a flip-rotation basis.");
	CHECK_MESSAGE(flip_skew_orthogonal.is_orthogonal(), "Basis4D is_orthogonal should be true for a flip-rotation basis.");
	CHECK_MESSAGE(flip_skew_orthogonal.is_orthonormal() == false, "Basis4D is_orthonormal should be false for a flip-rotation basis.");
	CHECK_MESSAGE(flip_skew_orthogonal.is_rotation() == false, "Basis4D is_rotation should be false for a flip-rotation basis.");

	const Basis4D flip_skew_orthonormal = flip_skew_w.orthonormalized();
	CHECK_MESSAGE(flip_skew_orthonormal.determinant() < 0.0f, "Basis4D determinant should be negative for a flip-rotation basis.");
	CHECK_MESSAGE(flip_skew_orthonormal.is_orthogonal(), "Basis4D is_orthogonal should be true for a flip-rotation basis.");
	CHECK_MESSAGE(flip_skew_orthonormal.is_orthonormal(), "Basis4D is_orthonormal should be true for a flip-rotation basis.");
	CHECK_MESSAGE(flip_skew_orthonormal.is_rotation() == false, "Flipped Basis4D is_rotation should not be a rotation.");
}

TEST_CASE("[Basis4D] Precomputed Inversion") {
	Basis4D ones = Basis4D(1, 1, 1, -1, 1, 1, -1, 1, 1, -1, 1, 1, -1, 1, 1, 1);
	Basis4D ones_adjugate = Basis4D(-4, -4, -4, 4, -4, -4, 4, -4, -4, 4, -4, -4, 4, -4, -4, -4);
	Basis4D ones_inverse = Basis4D(0.25, 0.25, 0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, -0.25, 0.25, 0.25, 0.25);
	CHECK_MESSAGE(ones_adjugate.is_equal_approx(ones.adjugate()), "Basis4D adjugate should work as expected.");
	CHECK_MESSAGE(ones_inverse.is_equal_approx(ones.inverse()), "Basis4D inverse should work as expected.");

	const Basis4D unique = Basis4D(1.0, 0.41, 0.31, 0.21, 0.11, 1.1, 0.12, 0.22, 0.32, 0.42, 1.2, 0.43, 0.33, 0.23, 0.13, 1.3);
	const Basis4D unique_inverse = Basis4D(1.131305786134016, -0.31456736720963524, -0.2559375714833072, -0.04485864504938581, -0.03978939804196893, 0.9801066566654912, -0.07307806077708652, -0.13526471134111334, -0.19433619846736885, -0.23401553809120415, 0.9326678341313813, -0.23750242201406277, -0.2607043400567804, -0.07015021530925138, -0.015368819899121764, 0.8282996548742938);
	CHECK_MESSAGE(unique_inverse.is_equal_approx(unique.inverse()), "Basis4D inverse should work as expected.");
	CHECK_MESSAGE(unique.is_equal_approx(unique_inverse.inverse()), "Basis4D inverse of inverse should be the original basis.");
}

TEST_CASE("[Basis4D] Random Inversion") {
	RandomNumberGenerator rng;
	for (int i = 0; i < 100; i++) {
		Basis4D basis = Basis4D();
		for (int j = 0; j < 16; j++) {
			basis[j / 4][j % 4] = rng.randf_range(-1.0, 1.0);
		}
		// Non-orthogonalized bases may be inverted, but the result will
		// have a high amount of floating-point error. The pre-computed tests
		// have an example of a non-orthogonal basis, for these random tests
		// we only want to test orthogonalized bases. If you try commenting
		// out this line many test runs will fail by a slight amount.
		basis.orthogonalize();
		const real_t determinant = basis.determinant();
		if (-CMP_EPSILON < determinant && determinant < CMP_EPSILON) {
			// For a near-zero determinant, the basis is not invertible.
			continue;
		}
		Basis4D inverted = basis.inverse();
		CHECK_MESSAGE(inverted.determinant() == doctest::Approx(1.0f / determinant), "Basis4D inverse should have an inverse determinant.");
		CHECK_MESSAGE(basis.is_equal_approx(inverted.inverse()), "Basis4D inverse of inverse should be the original basis.");
		CHECK_MESSAGE(basis.is_equal_approx(inverted.inverse()), String(rtos(basis.determinant()) + String(" != ") + rtos(inverted.determinant())).utf8());
		CHECK_MESSAGE(Basis4D().is_equal_approx(basis * inverted), "Basis4D basis times inverted should be the identity basis.");
		CHECK_MESSAGE(Basis4D().is_equal_approx(inverted * basis), "Basis4D inverted times basis should be the identity basis.");
	}
}
} // namespace TestBasis4D
