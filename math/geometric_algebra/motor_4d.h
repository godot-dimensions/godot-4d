#pragma once

#include "rotor_4d.h"

// Motor4D combines together a Rotor4D and a 4D vector, representing a rigid transformation in 4D space.
struct _NO_DISCARD_ Motor4D {
	union {
		struct {
			real_t scalar;
			Vector4 vector;
			Bivector4D bivector;
			real_t pseudoscalar; // A 4-vector in 4D is just a single number representing xyzw, we call it a pseudoscalar.
		};

		struct {
			real_t s;
			real_t x;
			real_t y;
			real_t z;
			real_t w;
			real_t xy;
			real_t xz;
			real_t xw;
			real_t yz;
			real_t yw;
			real_t zw;
			real_t xyzw;
		};

		real_t elements[12] = { 0 };
	};

	real_t get_scalar() const;
	void set_scalar(const real_t p_scalar);

	Vector4 get_vector() const;
	void set_vector(const Vector4 &p_vector);

	Bivector4D get_bivector() const;
	void set_bivector(const Bivector4D &p_bivector);

	real_t get_pseudoscalar() const;
	void set_pseudoscalar(const real_t p_pseudoscalar);

	real_t dot(const Motor4D &p_multivector) const;
	Motor4D dual() const;

	real_t length() const;
	real_t length_squared() const;
	Motor4D normalized() const;

	static Motor4D vector_product(const Vector4 &p_a, const Vector4 &p_b);

	// Operators.
	bool operator==(const Motor4D &p_v) const;
	bool operator!=(const Motor4D &p_v) const;
	Motor4D operator-() const;

	Motor4D &operator+=(const Motor4D &p_v);
	Motor4D operator+(const Motor4D &p_v) const;
	Motor4D &operator-=(const Motor4D &p_v);
	Motor4D operator-(const Motor4D &p_v) const;

	Motor4D &operator*=(const Motor4D &p_child);
	Motor4D operator*(const Motor4D &p_child) const;
	Motor4D &operator/=(const Motor4D &p_child);
	Motor4D operator/(const Motor4D &p_child) const;

	Motor4D &operator*=(const real_t p_scalar);
	Motor4D operator*(const real_t p_scalar) const;
	Motor4D &operator/=(const real_t p_scalar);
	Motor4D operator/(const real_t p_scalar) const;

	// Conversion.
	static Motor4D from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;

	static Motor4D from_rotor(const Rotor4D &p_rotor);
	static Motor4D from_rotor_vector(const Rotor4D &p_rotor, const Vector4 &p_vector);

	static Motor4D from_scalar(const real_t p_scalar);
	static Motor4D from_vector(const Vector4 &p_vector);
	static Motor4D from_bivector(const Bivector4D &p_bivector);
	static Motor4D from_pseudoscalar(const real_t p_pseudoscalar);
	static Motor4D from_scalar_vector(const real_t p_scalar, const Vector4 &p_vector);
	static Motor4D from_scalar_bivector(const real_t p_scalar, const Bivector4D &p_bivector);
	static Motor4D from_scalar_pseudoscalar(const real_t p_scalar, const real_t p_pseudoscalar);
	static Motor4D from_vector_bivector(const Vector4 &p_vector, const Bivector4D &p_bivector);
	static Motor4D from_vector_pseudoscalar(const Vector4 &p_vector, const real_t p_pseudoscalar);
	static Motor4D from_bivector_pseudoscalar(const Bivector4D &p_bivector, const real_t p_pseudoscalar);
	// If you want to construct from 3 components just use the constructor instead of these static functions.

	operator String() const;
	// For binding to Variant, pick an existing Variant data type.
	// AABB has 6 real_t fields so it will work fine.
	operator AABB() const;

	// Constructors.
	Motor4D() {}
	Motor4D(const real_t p_scalar, const Vector4 &p_vector, const Bivector4D &p_bivector, const real_t p_pseudoscalar);
	Motor4D(const real_t p_s, const real_t p_x, const real_t p_y, const real_t p_z, const real_t p_w, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw, const real_t p_xyzw);
};

static_assert(sizeof(Motor4D) == sizeof(real_t) * 12);
