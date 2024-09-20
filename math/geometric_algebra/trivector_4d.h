#pragma once

#include "bivector_4d.h"

struct _NO_DISCARD_ Trivector4D {
	union {
		struct {
			real_t xyz; // 0
			real_t xyw; // 1
			real_t xzw; // 2
			real_t yzw; // 3
		};

		real_t elements[4] = { 0 };
	};

	// Trivector functions.
	Trivector4D conjugate() const;
	Vector4 dual() const;
	Trivector4D inverse() const;
	Trivector4D involute() const;
	Bivector4D regressive_product(const Trivector4D &p_b) const;
	Trivector4D reverse() const;
	real_t scalar_product(const Trivector4D &p_b) const;

	// Length functions.
	real_t length() const;
	real_t length_squared() const;
	Trivector4D normalized() const;
	bool is_normalized() const;

	// Operators.
	_FORCE_INLINE_ const real_t &operator[](int p_axis) const {
		DEV_ASSERT((unsigned int)p_axis < 4);
		return elements[p_axis];
	}

	_FORCE_INLINE_ real_t &operator[](int p_axis) {
		DEV_ASSERT((unsigned int)p_axis < 4);
		return elements[p_axis];
	}

	bool is_equal_approx(const Trivector4D &p_other) const;
	bool operator==(const Trivector4D &p_v) const;
	bool operator!=(const Trivector4D &p_v) const;
	Trivector4D operator-() const;

	Trivector4D &operator+=(const Trivector4D &p_v);
	Trivector4D operator+(const Trivector4D &p_v) const;
	Trivector4D &operator-=(const Trivector4D &p_v);
	Trivector4D operator-(const Trivector4D &p_v) const;

	Trivector4D &operator*=(const real_t p_scalar);
	Trivector4D operator*(const real_t p_scalar) const;
	Trivector4D &operator/=(const real_t p_scalar);
	Trivector4D operator/(const real_t p_scalar) const;

	// Conversion.
	static Trivector4D from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;
	operator String() const;
	// For binding to Variant, pick an existing Variant data type. Vector4 has 4 real_t fields so it will work fine.
	// Note that this is just for binding. The order of the fields is not meaningful,
	// therefore this MUST NOT be used to convert Trivector4D to Vector4 in calculations.
	// Consider using dual() instead, which multiplies by xyzw to get x, y, z, and w fields for a Vector4.
	operator Vector4() const;

	// Constructors.
	Trivector4D() {}
	Trivector4D(const real_t p_xyz, const real_t p_xyw, const real_t p_xzw, const real_t p_yzw);
};

Trivector4D operator*(const real_t p_scalar, const Trivector4D &p_trivec);
