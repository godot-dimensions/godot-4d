#include "trivector_4d.h"

// Trivector functions.

// Conjugate is a no-op for 4D trivectors. It is included for completeness and clarity.
Trivector4D Trivector4D::conjugate() const {
	return *this;
}

// Dual multiplies each component by the unit pseudoscalar (xyzw) from the right.
// For example, xyz * xyzw = xyzxyzw = xxyzyzw = -xxyyzzw = -w.
// This is symmetric front to back, result[i] is set to +/- elements[3 - i].
Vector4 Trivector4D::dual() const {
	return Vector4(+yzw, -xzw, +xyw, -xyz);
}

// Involute flips the sign of the trivector components. It is functionally identical to the unary "-" operator, but included for completeness and clarity.
Trivector4D Trivector4D::involute() const {
	return -*this;
}

// The regressive product between two trivectors yields a bivector.
Bivector4D Trivector4D::regressive_product(const Trivector4D &p_b) const {
	Bivector4D result;
	result.xy = xyw * p_b.xyz - xyz * p_b.xyw;
	result.xz = xzw * p_b.xyz - xyz * p_b.xzw;
	result.xw = xzw * p_b.xyw - xyw * p_b.xzw;
	result.yz = yzw * p_b.xyz - xyz * p_b.yzw;
	result.yw = yzw * p_b.xyw - xyw * p_b.yzw;
	result.zw = yzw * p_b.xzw - xzw * p_b.yzw;
	return result;
}

// Reverse flips the sign of the trivector components. It is functionally identical to the unary "-" operator, but included for completeness and clarity.
Trivector4D Trivector4D::reverse() const {
	return -*this;
}

// The scalar part of the geometric algebra inner product only. This is also the scalar part of multiplication.
real_t Trivector4D::scalar_product(const Trivector4D &p_b) const {
	return (xyz * p_b.xyz + xyw * p_b.xyw + xzw * p_b.xzw + yzw * p_b.yzw) * -1.0f;
}

// Length functions.

real_t Trivector4D::length() const {
	return Math::sqrt(length_squared());
}

real_t Trivector4D::length_squared() const {
	return xyz * xyz + xyw * xyw + xzw * xzw + yzw * yzw;
}

Trivector4D Trivector4D::normalized() const {
	const real_t len_sq = length_squared();
	if (len_sq == 0) {
		return Trivector4D();
	}
	return *this / Math::sqrt(len_sq);
}

bool Trivector4D::is_normalized() const {
	return Math::is_equal_approx(length_squared(), (real_t)1.0);
}

// Operators.

bool Trivector4D::is_equal_approx(const Trivector4D &p_other) const {
	return Math::is_equal_approx(xyz, p_other.xyz) && Math::is_equal_approx(xyw, p_other.xyw) && Math::is_equal_approx(xzw, p_other.xzw) && Math::is_equal_approx(yzw, p_other.yzw);
}

bool Trivector4D::operator==(const Trivector4D &p_v) const {
	return xyz == p_v.xyz && xyw == p_v.xyw && xzw == p_v.xzw && yzw == p_v.yzw;
}

bool Trivector4D::operator!=(const Trivector4D &p_v) const {
	return xyz != p_v.xyz || xyw != p_v.xyw || xzw != p_v.xzw || yzw != p_v.yzw;
}

Trivector4D Trivector4D::operator-() const {
	return Trivector4D(-xyz, -xyw, -xzw, -yzw);
}

Trivector4D &Trivector4D::operator+=(const Trivector4D &p_v) {
	xyz += p_v.xyz;
	xyw += p_v.xyw;
	xzw += p_v.xzw;
	yzw += p_v.yzw;
	return *this;
}

Trivector4D Trivector4D::operator+(const Trivector4D &p_v) const {
	return Trivector4D(xyz + p_v.xyz, xyw + p_v.xyw, xzw + p_v.xzw, yzw + p_v.yzw);
}

Trivector4D &Trivector4D::operator-=(const Trivector4D &p_v) {
	xyz -= p_v.xyz;
	xyw -= p_v.xyw;
	xzw -= p_v.xzw;
	yzw -= p_v.yzw;
	return *this;
}

Trivector4D Trivector4D::operator-(const Trivector4D &p_v) const {
	return Trivector4D(xyz - p_v.xyz, xyw - p_v.xyw, xzw - p_v.xzw, yzw - p_v.yzw);
}

Trivector4D &Trivector4D::operator*=(const real_t p_scalar) {
	xyz *= p_scalar;
	xyw *= p_scalar;
	xzw *= p_scalar;
	yzw *= p_scalar;
	return *this;
}

Trivector4D Trivector4D::operator*(const real_t p_scalar) const {
	return Trivector4D(xyz * p_scalar, xyw * p_scalar, xzw * p_scalar, yzw * p_scalar);
}

Trivector4D &Trivector4D::operator/=(const real_t p_scalar) {
	xyz /= p_scalar;
	xyw /= p_scalar;
	xzw /= p_scalar;
	yzw /= p_scalar;
	return *this;
}

Trivector4D Trivector4D::operator/(const real_t p_scalar) const {
	return Trivector4D(xyz / p_scalar, xyw / p_scalar, xzw / p_scalar, yzw / p_scalar);
}

Trivector4D operator*(const real_t p_scalar, const Trivector4D &p_trivec) {
	return Trivector4D(p_scalar * p_trivec.xyz, p_scalar * p_trivec.xyw, p_scalar * p_trivec.xzw, p_scalar * p_trivec.yzw);
}

// Conversion.

Trivector4D Trivector4D::from_array(const PackedRealArray &p_from_array) {
	const int stop_index = MIN(p_from_array.size(), 4);
	Trivector4D trivec;
	for (int i = 0; i < stop_index; i++) {
		trivec.elements[i] = p_from_array[i];
	}
	return trivec;
}

PackedRealArray Trivector4D::to_array() const {
	PackedRealArray arr;
	arr.resize(4);
	real_t *ptr = arr.ptrw();
	ptr[0] = xyz;
	ptr[1] = xyw;
	ptr[2] = xzw;
	ptr[3] = yzw;
	return arr;
}

Trivector4D::Trivector4D(const real_t p_xyz, const real_t p_xyw, const real_t p_xzw, const real_t p_yzw) {
	xyz = p_xyz;
	xyw = p_xyw;
	xzw = p_xzw;
	yzw = p_yzw;
}

Trivector4D::operator Vector4() const {
	return Vector4(xyz, xyw, xzw, yzw);
}

#include "component_to_string.inc"

Trivector4D::operator String() const {
	String ret;
	if (!Math::is_zero_approx(xyz)) {
		ret += String::num(xyz) + "xyz";
	}
	APPEND_COMPONENT_TO_STRING(ret, xyw);
	APPEND_COMPONENT_TO_STRING(ret, xzw);
	APPEND_COMPONENT_TO_STRING(ret, yzw);
	return ret;
}

static_assert(sizeof(Trivector4D) == 4 * sizeof(real_t));
