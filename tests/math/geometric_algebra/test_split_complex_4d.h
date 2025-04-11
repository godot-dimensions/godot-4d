#pragma once

#include "../../../math/geometric_algebra/split_complex_4d.h"
#include "core/math/random_number_generator.h"
#include "tests/test_macros.h"

namespace TestSplitComplex4D {
TEST_CASE("[SplitComplex4D] Inverse") {
	SplitComplex4D inv = SplitComplex4D(1, 0).inverse();
	SplitComplex4D expected = SplitComplex4D(1, 0);
	CHECK_MESSAGE(inv.is_equal_approx(expected), "SplitComplex4D inverse of the real unit should be itself.");

	inv = SplitComplex4D(2, 0).inverse();
	expected = SplitComplex4D(0.5, 0);
	CHECK_MESSAGE(inv.is_equal_approx(expected), "SplitComplex4D inverse of twice the real unit should be half the real unit.");

	inv = SplitComplex4D(0.5, 0).inverse();
	expected = SplitComplex4D(2, 0);
	CHECK_MESSAGE(inv.is_equal_approx(expected), "SplitComplex4D inverse of half the real unit should be twice the real unit.");

	SplitComplex4D test = SplitComplex4D(1, 0.1);
	inv = test.inverse();
	expected = SplitComplex4D(1, 0);
	CHECK_MESSAGE(expected.is_equal_approx(test * inv), "SplitComplex4D inverse should be the multiplicative inverse.");
	CHECK_MESSAGE(expected.is_equal_approx(inv * test), "SplitComplex4D inverse should be the multiplicative inverse.");

	test = SplitComplex4D(0.5, 0.2);
	inv = test.inverse();
	CHECK_MESSAGE(expected.is_equal_approx(test * inv), "SplitComplex4D inverse should be the multiplicative inverse.");
	CHECK_MESSAGE(expected.is_equal_approx(inv * test), "SplitComplex4D inverse should be the multiplicative inverse.");

	test = SplitComplex4D(0.3, -0.2);
	inv = test.inverse();
	expected = SplitComplex4D(1, 0);
	CHECK_MESSAGE(expected.is_equal_approx(test * inv), "SplitComplex4D inverse should be the multiplicative inverse.");
	CHECK_MESSAGE(expected.is_equal_approx(inv * test), "SplitComplex4D inverse should be the multiplicative inverse.");

	test = SplitComplex4D(-0.7, 0.2);
	inv = test.inverse();
	CHECK_MESSAGE(expected.is_equal_approx(test * inv), "SplitComplex4D inverse should be the multiplicative inverse.");
	CHECK_MESSAGE(expected.is_equal_approx(inv * test), "SplitComplex4D inverse should be the multiplicative inverse.");

	test = SplitComplex4D(-0.8, -0.2);
	inv = test.inverse();
	CHECK_MESSAGE(expected.is_equal_approx(test * inv), "SplitComplex4D inverse should be the multiplicative inverse.");
	CHECK_MESSAGE(expected.is_equal_approx(inv * test), "SplitComplex4D inverse should be the multiplicative inverse.");
}

TEST_CASE("[SplitComplex4D] Inverse Square Root") {
	SplitComplex4D inv_sqrt = SplitComplex4D(1, 0).inverse();
	SplitComplex4D expected = SplitComplex4D(1, 0);
	CHECK_MESSAGE(inv_sqrt.is_equal_approx(expected), "SplitComplex4D inverse square root of the real unit should be itself.");

	inv_sqrt = SplitComplex4D(4, 0).inverse_square_root();
	expected = SplitComplex4D(0.5, 0);
	CHECK_MESSAGE(inv_sqrt.is_equal_approx(expected), "SplitComplex4D inverse square root of four times the real unit should be half the real unit.");

	inv_sqrt = SplitComplex4D(0.25, 0).inverse_square_root();
	expected = SplitComplex4D(2, 0);
	CHECK_MESSAGE(inv_sqrt.is_equal_approx(expected), "SplitComplex4D inverse square root of one quarter the real unit should be twice the real unit.");

	SplitComplex4D test = SplitComplex4D(1, 0.1);
	inv_sqrt = test.inverse_square_root();
	CHECK_MESSAGE(test.inverse().square_root().is_equal_approx(inv_sqrt), "SplitComplex4D inverse square root should be the same as inverse then square root.");
	CHECK_MESSAGE(test.square_root().inverse().is_equal_approx(inv_sqrt), "SplitComplex4D inverse square root should be the same as square root then inverse.");
	expected = SplitComplex4D(1, 0);
	CHECK_MESSAGE(expected.is_equal_approx(test * inv_sqrt * inv_sqrt), "SplitComplex4D inverse square root squared should be the multiplicative inverse.");
	expected = test.inverse();
	CHECK_MESSAGE(expected.is_equal_approx(inv_sqrt * inv_sqrt), "SplitComplex4D inverse square root squared should be the multiplicative inverse.");

	test = SplitComplex4D(0.5, -0.2);
	inv_sqrt = test.inverse_square_root();
	CHECK_MESSAGE(test.inverse().square_root().is_equal_approx(inv_sqrt), "SplitComplex4D inverse square root should be the same as inverse then square root.");
	CHECK_MESSAGE(test.square_root().inverse().is_equal_approx(inv_sqrt), "SplitComplex4D inverse square root should be the same as square root then inverse.");
	expected = SplitComplex4D(1, 0);
	CHECK_MESSAGE(expected.is_equal_approx(test * inv_sqrt * inv_sqrt), "SplitComplex4D inverse square root squared should be the multiplicative inverse.");
	expected = test.inverse();
	CHECK_MESSAGE(expected.is_equal_approx(inv_sqrt * inv_sqrt), "SplitComplex4D inverse square root squared should be the multiplicative inverse.");
}

TEST_CASE("[SplitComplex4D] Square Root") {
	SplitComplex4D square_root = SplitComplex4D(1, 0).inverse();
	SplitComplex4D expected = SplitComplex4D(1, 0);
	CHECK_MESSAGE(square_root.is_equal_approx(expected), "SplitComplex4D inverse square root of the real unit should be itself.");

	square_root = SplitComplex4D(4, 0).square_root();
	expected = SplitComplex4D(2, 0);
	CHECK_MESSAGE(square_root.is_equal_approx(expected), "SplitComplex4D inverse square root of four times the real unit should be twice the real unit.");

	square_root = SplitComplex4D(0.25, 0).square_root();
	expected = SplitComplex4D(0.5, 0);
	CHECK_MESSAGE(square_root.is_equal_approx(expected), "SplitComplex4D inverse square root of one quarter the real unit should be half the real unit.");

	SplitComplex4D test = SplitComplex4D(1, 0.1);
	square_root = test.square_root();
	CHECK_MESSAGE(test.is_equal_approx(square_root * square_root), "SplitComplex4D square root should be the multiplicative square root.");

	test = SplitComplex4D(0.5, -0.2);
	square_root = test.square_root();
	CHECK_MESSAGE(test.is_equal_approx(square_root * square_root), "SplitComplex4D square root should be the multiplicative square root.");
	expected = SplitComplex4D(1, 0);

	test = SplitComplex4D(5, -4);
	expected = SplitComplex4D(2, -1);
	square_root = test.square_root();
	CHECK_MESSAGE(test.is_equal_approx(square_root * square_root), "SplitComplex4D square root should be the multiplicative square root.");
	CHECK_MESSAGE(square_root.is_equal_approx(expected), "SplitComplex4D square root should match known precomputed value.");

	test = SplitComplex4D(1, 1);
	square_root = test.square_root();
	CHECK_MESSAGE(test.is_equal_approx(square_root * square_root), "SplitComplex4D square root should be the multiplicative square root.");
	expected = SplitComplex4D(Math_SQRT12, Math_SQRT12);
	CHECK_MESSAGE(expected.is_equal_approx(square_root), "SplitComplex4D square root should be the multiplicative square root.");

	test = SplitComplex4D(0, 0);
	square_root = test.square_root();
	CHECK_MESSAGE(test.is_equal_approx(square_root), "SplitComplex4D square root of zero should be zero.");
}
} // namespace TestSplitComplex4D
