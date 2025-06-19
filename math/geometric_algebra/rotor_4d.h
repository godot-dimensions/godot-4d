#pragma once

#include "../basis_4d.h"
#include "bivector_4d.h"
#include "split_complex_4d.h"
#include "trivector_4d.h"

// Rotor4D represents a 4D Geometric Algebra rotor. It is a combination of a scalar and a bivector.
// The pseudoscalar is also included to allow the rotor to be closed under multiplication, but is not needed in some cases.
// Reference: https://marctenbosch.com/news/2011/05/4d-rotations-and-the-4d-equivalent-of-quaternions/
struct _NO_DISCARD_ Rotor4D {
	union {
		struct {
			real_t scalar;
			Bivector4D bivector;
			real_t pseudoscalar;
		} parts;

		struct {
			real_t s; // 0
			real_t xy; // 1
			real_t xz; // 2
			real_t xw; // 3
			real_t yz; // 4
			real_t yw; // 5
			real_t zw; // 6
			real_t xyzw; // 7
		};

		real_t elements[8] = { 0 };
	};

	// Geometric algebra functions.
	Rotor4D conjugate() const;
	real_t dot(const Rotor4D &p_b) const;
	Rotor4D dual() const;
	Rotor4D inner_product(const Rotor4D &p_b) const;
	Rotor4D inverse() const;
	Rotor4D involute() const;
	Rotor4D regressive_product(const Rotor4D &p_b) const;
	Rotor4D reverse() const;
	real_t scalar_product(const Rotor4D &p_b) const;
	Rotor4D wedge_product(const Rotor4D &p_b) const;

	// Rotation functions.
	Basis4D to_basis() const;
	// real_t get_rotation_angle() const;
	real_t get_simple_rotation_angle() const;
	// Bivector4D get_rotation_bivector_magnitude() const;
	// Bivector4D get_rotation_bivector_normal() const;
	Basis4D rotate_basis(const Basis4D &p_basis) const;
	Rotor4D rotate_rotor(const Rotor4D &p_rotor) const;
	Vector4 rotate_vector(const Vector4 &p_vec) const;
	Vector4 sandwich(const Vector4 &p_vec, const Rotor4D &p_right) const;
	Rotor4D slerp(Rotor4D p_to, const real_t p_weight) const;
	Rotor4D slerp_fraction(const real_t p_weight = 0.5f) const;

	// Length functions.
	real_t length() const;
	real_t length_squared() const;
	SplitComplex4D split_magnitude_squared() const;
	Rotor4D normalized() const;
	bool is_normalized() const;
	bool is_simple_rotation() const;

	// Static functions for doing math on non-Rotor4D types and returning a Rotor4D.
	static Rotor4D vector_product(const Vector4 &p_a, const Vector4 &p_b);
	static Rotor4D from_basis(const Basis4D &p_basis);
	static Rotor4D from_bivector_magnitude(const Bivector4D &p_bivector);
	static Rotor4D from_bivector_normal_angle(const Bivector4D &p_bivector_normal, const real_t p_angle);
	static Rotor4D from_xy(const real_t p_angle);
	static Rotor4D from_xz(const real_t p_angle);
	static Rotor4D from_zx(const real_t p_angle);
	static Rotor4D from_xw(const real_t p_angle);
	static Rotor4D from_yz(const real_t p_angle);
	static Rotor4D from_yw(const real_t p_angle);
	static Rotor4D from_wy(const real_t p_angle);
	static Rotor4D from_zw(const real_t p_angle);
	static Rotor4D identity();

	// Component with-style setters.
	Rotor4D with_s(const real_t p_s) const;
	Rotor4D with_xy(const real_t p_xy) const;
	Rotor4D with_xz(const real_t p_xz) const;
	Rotor4D with_xw(const real_t p_xw) const;
	Rotor4D with_yz(const real_t p_yz) const;
	Rotor4D with_yw(const real_t p_yw) const;
	Rotor4D with_zw(const real_t p_zw) const;
	Rotor4D with_xyzw(const real_t p_xyzw) const;

	// Trivial getters and setters.
	real_t get_scalar() const;
	void set_scalar(const real_t p_scalar);

	Bivector4D get_bivector() const;
	void set_bivector(const Bivector4D &p_bivector);

	real_t get_pseudoscalar() const;
	void set_pseudoscalar(const real_t p_pseudoscalar);

	SplitComplex4D get_split_complex() const;
	void set_split_complex(const SplitComplex4D &p_split);

	// Operators.
	_FORCE_INLINE_ const real_t &operator[](int p_axis) const {
		DEV_ASSERT((unsigned int)p_axis < 8);
		return elements[p_axis];
	}

	_FORCE_INLINE_ real_t &operator[](int p_axis) {
		DEV_ASSERT((unsigned int)p_axis < 8);
		return elements[p_axis];
	}

	bool is_equal_approx(const Rotor4D &p_other) const;
	bool operator==(const Rotor4D &p_v) const;
	bool operator!=(const Rotor4D &p_v) const;
	Rotor4D operator-() const;

	Rotor4D &operator+=(const Rotor4D &p_v);
	Rotor4D operator+(const Rotor4D &p_v) const;
	Rotor4D &operator-=(const Rotor4D &p_v);
	Rotor4D operator-(const Rotor4D &p_v) const;

	Rotor4D &operator*=(const Rotor4D &p_b);
	Rotor4D operator*(const Rotor4D &p_b) const;

	Rotor4D &operator*=(const SplitComplex4D &p_split);
	Rotor4D operator*(const SplitComplex4D &p_split) const;
	Rotor4D &operator/=(const SplitComplex4D &p_split);
	Rotor4D operator/(const SplitComplex4D &p_split) const;

	Rotor4D &operator+=(const real_t p_scalar);
	Rotor4D operator+(const real_t p_scalar) const;
	Rotor4D &operator-=(const real_t p_scalar);
	Rotor4D operator-(const real_t p_scalar) const;

	Rotor4D &operator*=(const real_t p_scalar);
	Rotor4D operator*(const real_t p_scalar) const;
	Rotor4D &operator/=(const real_t p_scalar);
	Rotor4D operator/(const real_t p_scalar) const;

	Rotor4D operator*(const Bivector4D &p_b) const;
	Vector4 operator*(const Vector4 &p_b) const; // Not actually a multiplication, instead rotates the vector by the rotor.

	// Actually a multiplication, but has 2 outputs so we can't use the operator* syntax.
	// It's unclear if making a struct for Vector4+Trivector4D is worth it or what it would be called.
	void multiply_vector(const Vector4 &p_in_vector, Vector4 &r_out_vector, Trivector4D &r_out_trivec) const;
	void multiply_vector_trivector(const Vector4 &p_in_vec, const Trivector4D &p_in_trivec, Vector4 &r_out_vector, Trivector4D &r_out_trivec) const;
	void premultiply_vector(const Vector4 &p_in_vector, Vector4 &r_out_vector, Trivector4D &r_out_trivec) const;
	void premultiply_vector_trivector(const Vector4 &p_in_vec, const Trivector4D &p_in_trivec, Vector4 &r_out_vector, Trivector4D &r_out_triv) const;

	// Conversion.
	static Rotor4D from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;

	static Rotor4D from_scalar(const real_t p_scalar);
	static Rotor4D from_bivector(const Bivector4D &p_bivector);
	static Rotor4D from_pseudoscalar(const real_t p_pseudoscalar);
	static Rotor4D from_scalar_bivector(const real_t p_scalar, const Bivector4D &p_bivector);
	static Rotor4D from_scalar_pseudoscalar(const real_t p_scalar, const real_t p_pseudoscalar);
	static Rotor4D from_bivector_pseudoscalar(const Bivector4D &p_bivector, const real_t p_pseudoscalar);
	// If you want to construct from 3, 4, or 5 components just use the constructor instead of these static functions.

	operator String() const;

	// Constructors.
	Rotor4D() {}
	Rotor4D(const real_t p_scalar, const Bivector4D &p_bivector, const real_t p_pseudoscalar = 0.0);
	Rotor4D(const real_t p_s, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw, const real_t p_xyzw = 0.0);
};

Rotor4D operator+(const real_t p_scalar, const Rotor4D &p_rotor);
Rotor4D operator+(const Bivector4D &p_bivector, const Rotor4D &p_rotor);

Rotor4D operator*(const real_t p_scalar, const Rotor4D &p_rotor);
Rotor4D operator*(const Bivector4D &p_bivector, const Rotor4D &p_rotor);
Rotor4D operator*(const SplitComplex4D &p_split, const Rotor4D &p_rotor);

static_assert(sizeof(Rotor4D) == sizeof(real_t) * 8);
