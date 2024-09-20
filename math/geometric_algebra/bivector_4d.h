#pragma once

#include "../basis_4d.h"

#if GDEXTENSION
#include <godot_cpp/variant/aabb.hpp>
#elif GODOT_MODULE
#include "core/math/aabb.h"
#endif

struct _NO_DISCARD_ Bivector4D {
	union {
		struct {
			real_t xy; // 0
			real_t xz; // 1
			real_t xw; // 2
			real_t yz; // 3
			real_t yw; // 4
			real_t zw; // 5
		};

		real_t elements[6] = { 0 };
	};

	// Bivector functions.
	Bivector4D conjugate() const;
	real_t dot(const Bivector4D &p_b) const;
	Bivector4D dual() const;
	Bivector4D inverse() const;
	Bivector4D involute() const;
	real_t regressive_product(const Bivector4D &p_b) const;
	Bivector4D reverse() const;
	real_t scalar_product(const Bivector4D &p_b) const;
	real_t wedge_product(const Bivector4D &p_b) const;

	// Length functions.
	real_t length() const;
	real_t length_squared() const;
	Bivector4D normalized() const;
	bool is_normalized() const;

	// Static functions for doing math on non-Bivector4D types and returning a Bivector4D.
	static Bivector4D vector_wedge_product(const Vector4 &p_a, const Vector4 &p_b);

	// Trivial getter and setter.
	Bivector4D get_bivector() const;
	void set_bivector(const Bivector4D &p_bivector);

	// Operators.
	_FORCE_INLINE_ const real_t &operator[](int p_axis) const {
		DEV_ASSERT((unsigned int)p_axis < 6);
		return elements[p_axis];
	}

	_FORCE_INLINE_ real_t &operator[](int p_axis) {
		DEV_ASSERT((unsigned int)p_axis < 6);
		return elements[p_axis];
	}

	bool is_equal_approx(const Bivector4D &p_other) const;
	bool operator==(const Bivector4D &p_v) const;
	bool operator!=(const Bivector4D &p_v) const;
	Bivector4D operator-() const;

	Bivector4D &operator+=(const Bivector4D &p_v);
	Bivector4D operator+(const Bivector4D &p_v) const;
	Bivector4D &operator-=(const Bivector4D &p_v);
	Bivector4D operator-(const Bivector4D &p_v) const;

	Bivector4D &operator*=(const real_t p_scalar);
	Bivector4D operator*(const real_t p_scalar) const;
	Bivector4D &operator/=(const real_t p_scalar);
	Bivector4D operator/(const real_t p_scalar) const;

	// Conversion.
	static Bivector4D from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;
	operator String() const;
	// For binding to Variant, pick an existing Variant data type. AABB has 6 real_t fields so it will work fine.
	operator AABB() const;

	// Constructors.
	Bivector4D() {}
	Bivector4D(const AABB &p_from);
	Bivector4D(const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw);
};

Bivector4D operator+(const real_t p_scalar, const Bivector4D &p_bivec);
Bivector4D operator*(const real_t p_scalar, const Bivector4D &p_bivec);
