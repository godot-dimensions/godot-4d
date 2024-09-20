#include "bivector_4d.h"

// Bivector functions.

// Conjugate flips the sign of the bivector components. It is functionally identical to the unary "-" operator, but included for completeness and clarity.
Bivector4D Bivector4D::conjugate() const {
	return -*this;
}

// The dot product measures how aligned two bivectors are.
real_t Bivector4D::dot(const Bivector4D &p_b) const {
	return xy * p_b.xy + xz * p_b.xz + xw * p_b.xw + yz * p_b.yz + yw * p_b.yw + zw * p_b.zw;
}

// Dual multiplies each component by the unit pseudoscalar (xyzw) from the right.
// For example, xy * xyzw = xyxyzw = -xxyyzw = -zw, and xz * xyzw = xzxyzw = +xxyzzw = +yw.
// This is symmetric front to back, result[i] is set to +/- elements[5 - i].
Bivector4D Bivector4D::dual() const {
	Bivector4D result;
	result.xy = -zw;
	result.xz = +yw;
	result.xw = -yz;
	result.yz = -xw;
	result.yw = +xz;
	result.zw = -xy;
	return result;
}

// The scalar part of the geometric algebra inner product only. This is also the scalar part of multiplication.
real_t Bivector4D::scalar_product(const Bivector4D &p_b) const {
	return (xy * p_b.xy + xz * p_b.xz + xw * p_b.xw + yz * p_b.yz + yw * p_b.yw + zw * p_b.zw) * -1.0f;
}

// Involute is a no-op for 4D bivectors. It is included for completeness and clarity.
Bivector4D Bivector4D::involute() const {
	return *this;
}

// The regressive product between two bivectors yields a scalar. For 4D bivectors, this is identical to the wedge product, but both are included for completeness and clarity.
real_t Bivector4D::regressive_product(const Bivector4D &p_b) const {
	return xy * p_b.zw - xz * p_b.yw + xw * p_b.yz + yz * p_b.xw - yw * p_b.xz + zw * p_b.xy;
}

// Reverse flips the sign of the bivector components. It is functionally identical to the unary "-" operator, but included for completeness and clarity.
Bivector4D Bivector4D::reverse() const {
	return -*this;
}

// The wedge product between two bivectors yields a scalar. For 4D bivectors, this is identical to the regressive product, but both are included for completeness and clarity.
real_t Bivector4D::wedge_product(const Bivector4D &p_b) const {
	return xy * p_b.zw - xz * p_b.yw + xw * p_b.yz + yz * p_b.xw - yw * p_b.xz + zw * p_b.xy;
}

// Length functions.

real_t Bivector4D::length() const {
	return Math::sqrt(length_squared());
}

real_t Bivector4D::length_squared() const {
	return xy * xy + xz * xz + xw * xw + yz * yz + yw * yw + zw * zw;
}

Bivector4D Bivector4D::normalized() const {
	const real_t len_sq = length_squared();
	if (len_sq == 0) {
		return Bivector4D();
	}
	return *this / Math::sqrt(len_sq);
}

bool Bivector4D::is_normalized() const {
	return Math::is_equal_approx(length_squared(), (real_t)1.0);
}

// Static functions for doing math on non-Bivector4D types and returning a Bivector4D.

Bivector4D Bivector4D::vector_wedge_product(const Vector4 &p_a, const Vector4 &p_b) {
	Bivector4D result;
	result.xy = p_a.x * p_b.y - p_a.y * p_b.x;
	result.xz = p_a.x * p_b.z - p_a.z * p_b.x;
	result.xw = p_a.x * p_b.w - p_a.w * p_b.x;
	result.yz = p_a.y * p_b.z - p_a.z * p_b.y;
	result.yw = p_a.y * p_b.w - p_a.w * p_b.y;
	result.zw = p_a.z * p_b.w - p_a.w * p_b.z;
	return result;
}

// Operators.

bool Bivector4D::is_equal_approx(const Bivector4D &p_other) const {
	return Math::is_equal_approx(xy, p_other.xy) && Math::is_equal_approx(xz, p_other.xz) && Math::is_equal_approx(xw, p_other.xw) && Math::is_equal_approx(yz, p_other.yz) && Math::is_equal_approx(yw, p_other.yw) && Math::is_equal_approx(zw, p_other.zw);
}

bool Bivector4D::operator==(const Bivector4D &p_v) const {
	return xy == p_v.xy && xz == p_v.xz && xw == p_v.xw && yz == p_v.yz && yw == p_v.yw && zw == p_v.zw;
}

bool Bivector4D::operator!=(const Bivector4D &p_v) const {
	return xy != p_v.xy || xz != p_v.xz || xw != p_v.xw || yz != p_v.yz || yw != p_v.yw || zw != p_v.zw;
}

Bivector4D Bivector4D::operator-() const {
	return Bivector4D(-xy, -xz, -xw, -yz, -yw, -zw);
}

Bivector4D &Bivector4D::operator+=(const Bivector4D &p_v) {
	xy += p_v.xy;
	xz += p_v.xz;
	xw += p_v.xw;
	yz += p_v.yz;
	yw += p_v.yw;
	zw += p_v.zw;
	return *this;
}

Bivector4D Bivector4D::operator+(const Bivector4D &p_v) const {
	return Bivector4D(xy + p_v.xy, xz + p_v.xz, xw + p_v.xw, yz + p_v.yz, yw + p_v.yw, zw + p_v.zw);
}

Bivector4D &Bivector4D::operator-=(const Bivector4D &p_v) {
	xy -= p_v.xy;
	xz -= p_v.xz;
	xw -= p_v.xw;
	yz -= p_v.yz;
	yw -= p_v.yw;
	zw -= p_v.zw;
	return *this;
}

Bivector4D Bivector4D::operator-(const Bivector4D &p_v) const {
	return Bivector4D(xy - p_v.xy, xz - p_v.xz, xw - p_v.xw, yz - p_v.yz, yw - p_v.yw, zw - p_v.zw);
}

Bivector4D &Bivector4D::operator*=(const real_t p_scalar) {
	xy *= p_scalar;
	xz *= p_scalar;
	xw *= p_scalar;
	yz *= p_scalar;
	yw *= p_scalar;
	zw *= p_scalar;
	return *this;
}

Bivector4D Bivector4D::operator*(const real_t p_scalar) const {
	return Bivector4D(xy * p_scalar, xz * p_scalar, xw * p_scalar, yz * p_scalar, yw * p_scalar, zw * p_scalar);
}

Bivector4D &Bivector4D::operator/=(const real_t p_scalar) {
	xy /= p_scalar;
	xz /= p_scalar;
	xw /= p_scalar;
	yz /= p_scalar;
	yw /= p_scalar;
	zw /= p_scalar;
	return *this;
}

Bivector4D Bivector4D::operator/(const real_t p_scalar) const {
	return Bivector4D(xy / p_scalar, xz / p_scalar, xw / p_scalar, yz / p_scalar, yw / p_scalar, zw / p_scalar);
}

Bivector4D operator*(const real_t p_scalar, const Bivector4D &p_bivec) {
	return Bivector4D(p_scalar * p_bivec.xy, p_scalar * p_bivec.xz, p_scalar * p_bivec.xw, p_scalar * p_bivec.yz, p_scalar * p_bivec.yw, p_scalar * p_bivec.zw);
}

// Conversion.

Bivector4D Bivector4D::from_array(const PackedRealArray &p_from_array) {
	const int stop_index = MIN(p_from_array.size(), 6);
	Bivector4D bivec;
	for (int i = 0; i < stop_index; i++) {
		bivec.elements[i] = p_from_array[i];
	}
	return bivec;
}

PackedRealArray Bivector4D::to_array() const {
	PackedRealArray arr;
	arr.resize(6);
	real_t *ptr = arr.ptrw();
	ptr[0] = xy;
	ptr[1] = xz;
	ptr[2] = xw;
	ptr[3] = yz;
	ptr[4] = yw;
	ptr[5] = zw;
	return arr;
}

Bivector4D::Bivector4D(const AABB &p_from) {
	xy = p_from.position.x;
	xz = p_from.position.y;
	xw = p_from.position.z;
	yz = p_from.size.x;
	yw = p_from.size.y;
	zw = p_from.size.z;
}

Bivector4D::Bivector4D(const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw) {
	xy = p_xy;
	xz = p_xz;
	xw = p_xw;
	yz = p_yz;
	yw = p_yw;
	zw = p_zw;
}

Bivector4D::operator AABB() const {
	AABB aabb;
	aabb.position = Vector3(xy, xz, xw);
	aabb.size = Vector3(yz, yw, zw);
	return aabb;
}

#include "component_to_string.inc"

Bivector4D::operator String() const {
	String ret;
	if (!Math::is_zero_approx(xy)) {
		ret += String::num(xy) + "xy";
	}
	APPEND_COMPONENT_TO_STRING(ret, xz);
	APPEND_COMPONENT_TO_STRING(ret, xw);
	APPEND_COMPONENT_TO_STRING(ret, yz);
	APPEND_COMPONENT_TO_STRING(ret, yw);
	APPEND_COMPONENT_TO_STRING(ret, zw);
	return ret;
}

static_assert(sizeof(Bivector4D) == 6 * sizeof(real_t));
