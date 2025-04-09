#pragma once

#include "bivector_4d.h"
#include "rotor_4d.h"
#include "trivector_4d.h"

// Multivector4D combines together a scalar, a 4D vector, a 4D bivector, a 4D trivector, and a pseudoscalar.
// Reference: https://www.youtube.com/watch?v=60z_hpEAtD8
// Reference: https://www.youtube.com/watch?v=htYh-Tq7ZBI
struct _NO_DISCARD_ Multivector4D {
	union {
		struct {
			real_t scalar;
			Vector4 vector;
			Bivector4D bivector;
			Trivector4D trivector;
			real_t pseudoscalar; // A 4-vector in 4D is just a single number representing xyzw, we call it a pseudoscalar.
		} parts;

		struct {
			real_t s; // 0
			real_t x; // 1
			real_t y; // 2
			real_t z; // 3
			real_t w; // 4
			real_t xy; // 5
			real_t xz; // 6
			real_t xw; // 7
			real_t yz; // 8
			real_t yw; // 9
			real_t zw; // 10
			real_t xyz; // 11
			real_t xyw; // 12
			real_t xzw; // 13
			real_t yzw; // 14
			real_t xyzw; // 15
		};

		real_t elements[16] = { 0 };
	};

	// Geometric algebra functions.
	Multivector4D conjugate() const;
	real_t dot(const Multivector4D &p_b) const;
	Multivector4D dual() const;
	Multivector4D inner_product(const Multivector4D &p_b) const;
	Multivector4D inverse() const;
	Multivector4D involute() const;
	Multivector4D regressive_product(const Multivector4D &p_b) const;
	Multivector4D reverse() const;
	real_t scalar_product(const Multivector4D &p_b) const;
	Multivector4D wedge_product(const Multivector4D &p_b) const;

	// Rotation functions.
	Basis4D rotate_basis(const Basis4D &p_basis) const;
	Vector4 rotate_vector(const Vector4 &p_vec) const;

	// Length functions.
	real_t length() const;
	real_t length_squared() const;
	Multivector4D normalized() const;
	bool is_normalized() const;
	bool is_rotor(const bool p_pure = false) const;

	// Static functions for doing math on non-Multivector4D types and returning a Multivector4D.
	static Multivector4D vector_product(const Vector4 &p_a, const Vector4 &p_b);
	static Multivector4D vector_wedge_product(const Vector4 &p_a, const Vector4 &p_b);
	static Multivector4D rotor_vector_product(const Rotor4D &p_rotor, const Vector4 &p_vector);
	static Multivector4D identity();

	// Trivial getters and setters.
	real_t get_scalar() const;
	void set_scalar(const real_t p_scalar);

	Vector4 get_vector() const;
	void set_vector(const Vector4 &p_vector);

	Bivector4D get_bivector() const;
	void set_bivector(const Bivector4D &p_bivector);

	Trivector4D get_trivector() const;
	void set_trivector(const Trivector4D &p_trivector);

	real_t get_pseudoscalar() const;
	void set_pseudoscalar(const real_t p_pseudoscalar);

	Rotor4D get_rotor() const;
	void set_rotor(const Rotor4D &p_rotor);

	// Operators.
	_FORCE_INLINE_ const real_t &operator[](int p_axis) const {
		DEV_ASSERT((unsigned int)p_axis < 16);
		return elements[p_axis];
	}

	_FORCE_INLINE_ real_t &operator[](int p_axis) {
		DEV_ASSERT((unsigned int)p_axis < 16);
		return elements[p_axis];
	}

	bool is_equal_approx(const Multivector4D &p_other) const;
	bool operator==(const Multivector4D &p_v) const;
	bool operator!=(const Multivector4D &p_v) const;
	Multivector4D operator-() const;

	Multivector4D &operator+=(const Multivector4D &p_v);
	Multivector4D operator+(const Multivector4D &p_v) const;
	Multivector4D &operator-=(const Multivector4D &p_v);
	Multivector4D operator-(const Multivector4D &p_v) const;

	Multivector4D &operator*=(const Multivector4D &p_b);
	Multivector4D operator*(const Multivector4D &p_b) const;

	Multivector4D &operator+=(const real_t p_scalar);
	Multivector4D operator+(const real_t p_scalar) const;
	Multivector4D &operator-=(const real_t p_scalar);
	Multivector4D operator-(const real_t p_scalar) const;

	Multivector4D &operator*=(const real_t p_scalar);
	Multivector4D operator*(const real_t p_scalar) const;
	Multivector4D &operator/=(const real_t p_scalar);
	Multivector4D operator/(const real_t p_scalar) const;

	Multivector4D operator+(const Vector4 &p_vector) const;
	Multivector4D operator+(const Bivector4D &p_b) const;
	Multivector4D operator+(const Trivector4D &p_b) const;
	Multivector4D operator+(const Rotor4D &p_b) const;

	Multivector4D operator*(const Vector4 &p_b) const;
	Multivector4D operator*(const Bivector4D &p_b) const;
	Multivector4D operator*(const Trivector4D &p_b) const;
	Multivector4D operator*(const Rotor4D &p_b) const;

	Multivector4D multiply_vector_trivector(const Vector4 &p_vector, const Trivector4D &p_triv) const;
	Multivector4D premultiply_vector_trivector(const Vector4 &p_vector, const Trivector4D &p_trivec) const;

	// Conversion.
	static Multivector4D from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;

	static Multivector4D from_rotor(const Rotor4D &p_rotor);
	static Multivector4D from_rotor_vector(const Rotor4D &p_rotor, const Vector4 &p_vector);

	static Multivector4D from_scalar(const real_t p_scalar);
	static Multivector4D from_vector(const Vector4 &p_vector);
	static Multivector4D from_bivector(const Bivector4D &p_bivector);
	static Multivector4D from_trivector(const Trivector4D &p_trivector);
	static Multivector4D from_pseudoscalar(const real_t p_pseudoscalar);
	static Multivector4D from_scalar_vector(const real_t p_scalar, const Vector4 &p_vector);
	static Multivector4D from_scalar_bivector(const real_t p_scalar, const Bivector4D &p_bivector);
	static Multivector4D from_scalar_trivector(const real_t p_scalar, const Trivector4D &p_trivector);
	static Multivector4D from_scalar_pseudoscalar(const real_t p_scalar, const real_t p_pseudoscalar);
	static Multivector4D from_vector_bivector(const Vector4 &p_vector, const Bivector4D &p_bivector);
	static Multivector4D from_vector_trivector(const Vector4 &p_vector, const Trivector4D &p_trivector);
	static Multivector4D from_vector_pseudoscalar(const Vector4 &p_vector, const real_t p_pseudoscalar);
	static Multivector4D from_bivector_trivector(const Bivector4D &p_bivector, const Trivector4D &p_trivector);
	static Multivector4D from_bivector_pseudoscalar(const Bivector4D &p_bivector, const real_t p_pseudoscalar);
	static Multivector4D from_trivector_pseudoscalar(const Trivector4D &p_trivector, const real_t p_pseudoscalar);
	// If you want to construct from 3, 4, or 5 components just use the constructor instead of these static functions.

	operator String() const;
	Multivector4D &operator=(const Multivector4D &p_other);

	// Constructors.
	Multivector4D() {}
	Multivector4D(const Multivector4D &p_from);
	Multivector4D(const real_t p_scalar, const Vector4 &p_vector, const Bivector4D &p_bivector, const Trivector4D &p_trivector, const real_t p_pseudoscalar);
	Multivector4D(const real_t p_s, const real_t p_x, const real_t p_y, const real_t p_z, const real_t p_w, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw, const real_t p_xyz, const real_t p_xyw, const real_t p_xzw, const real_t p_yzw, const real_t p_xyzw);
};

Multivector4D operator+(const real_t p_scalar, const Multivector4D &p_multivec);
Multivector4D operator+(const Vector4 &p_vector, const Multivector4D &p_multivec);
Multivector4D operator+(const Bivector4D &p_bivector, const Multivector4D &p_multivec);
Multivector4D operator+(const Trivector4D &p_trivector, const Multivector4D &p_multivec);
Multivector4D operator+(const Rotor4D &p_rotor, const Multivector4D &p_multivec);

Multivector4D operator*(const real_t p_scalar, const Multivector4D &p_multivec);
Multivector4D operator*(const Vector4 &p_vector, const Multivector4D &p_multivec);
Multivector4D operator*(const Bivector4D &p_bivector, const Multivector4D &p_multivec);
Multivector4D operator*(const Trivector4D &p_trivector, const Multivector4D &p_multivec);
Multivector4D operator*(const Rotor4D &p_rotor, const Multivector4D &p_multivec);

static_assert(sizeof(Multivector4D) == sizeof(real_t) * 16);
