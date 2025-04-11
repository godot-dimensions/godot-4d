#pragma once

#include "../../godot_4d_defines.h"

// SplitComplex4D represents a split-complex number. https://en.wikipedia.org/wiki/Split-complex_number
// Split-complex numbers are like complex numbers, but the imaginary unit squares to +1 instead of -1.
// This class is tailored for Geometric Algebra, a scalar and a pseudoscalar. In 4D, this is a combination of a scalar and a 4-vector (xyzw).
struct _NO_DISCARD_ SplitComplex4D {
	union {
		struct {
			real_t scalar;
			real_t pseudoscalar;
		} parts;

		struct {
			real_t s; // 0
			real_t xyzw; // 1
		};

		real_t elements[2] = { 0 };
	};

	// Geometric algebra functions.
	real_t dot(const SplitComplex4D &p_b) const;
	SplitComplex4D inverse() const;
	SplitComplex4D inverse_square_root() const;
	SplitComplex4D square_root() const;

	// Trivial getters and setters.
	real_t get_scalar() const;
	void set_scalar(const real_t p_scalar);

	real_t get_pseudoscalar() const;
	void set_pseudoscalar(const real_t p_pseudoscalar);

	// Operators.
	_FORCE_INLINE_ const real_t &operator[](int p_axis) const {
		DEV_ASSERT((unsigned int)p_axis < 2);
		return elements[p_axis];
	}

	_FORCE_INLINE_ real_t &operator[](int p_axis) {
		DEV_ASSERT((unsigned int)p_axis < 2);
		return elements[p_axis];
	}

	bool is_equal_approx(const SplitComplex4D &p_other) const;
	bool operator==(const SplitComplex4D &p_v) const;
	bool operator!=(const SplitComplex4D &p_v) const;
	SplitComplex4D operator-() const;

	SplitComplex4D &operator+=(const SplitComplex4D &p_v);
	SplitComplex4D operator+(const SplitComplex4D &p_v) const;
	SplitComplex4D &operator-=(const SplitComplex4D &p_v);
	SplitComplex4D operator-(const SplitComplex4D &p_v) const;

	SplitComplex4D &operator*=(const SplitComplex4D &p_b);
	SplitComplex4D operator*(const SplitComplex4D &p_b) const;

	SplitComplex4D &operator+=(const real_t p_scalar);
	SplitComplex4D operator+(const real_t p_scalar) const;
	SplitComplex4D &operator-=(const real_t p_scalar);
	SplitComplex4D operator-(const real_t p_scalar) const;

	SplitComplex4D &operator*=(const real_t p_scalar);
	SplitComplex4D operator*(const real_t p_scalar) const;
	SplitComplex4D &operator/=(const real_t p_scalar);
	SplitComplex4D operator/(const real_t p_scalar) const;

	// Conversion.
	static SplitComplex4D from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;
	static SplitComplex4D identity();

	operator String() const;
	// For binding to Variant, pick an existing Variant data type. Vector2 has 2 real_t fields so it will work fine.
	operator Vector2() const;

	// Constructors.
	SplitComplex4D() {}
	SplitComplex4D(const Vector2 &p_from);
	SplitComplex4D(const real_t p_scalar, const real_t p_pseudoscalar = 0.0);
};

SplitComplex4D operator+(const real_t p_scalar, const SplitComplex4D &p_rotor);
SplitComplex4D operator*(const real_t p_scalar, const SplitComplex4D &p_rotor);

static_assert(sizeof(SplitComplex4D) == sizeof(real_t) * 2);
