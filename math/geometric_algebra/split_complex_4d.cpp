#include "split_complex_4d.h"

#include "../euler_4d.h"

// Geometric algebra functions.

// The dot product measures how aligned two multivectors are.
real_t SplitComplex4D::dot(const SplitComplex4D &p_b) const {
	return s * p_b.s + xyzw * p_b.xyzw;
}

SplitComplex4D SplitComplex4D::inverse() const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(s == 0.0f, SplitComplex4D(0.0f, 0.0f), "Cannot invert a split complex number with a zero scalar.");
	ERR_FAIL_COND_V_MSG(Math::abs(xyzw) >= Math::abs(s), SplitComplex4D(0.0f, 0.0f), "Cannot invert a split complex number with a more significant pseudoscalar than scalar.");
#endif
	const real_t c = s / (s * s - xyzw * xyzw);
	const real_t d = -c * (xyzw / s);
	return SplitComplex4D(c, d);
}

SplitComplex4D SplitComplex4D::inverse_square_root() const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(s <= 0.0f, SplitComplex4D(0.0f, 0.0f), "Cannot inverse square root a split complex number with a non-positive scalar.");
	ERR_FAIL_COND_V_MSG(Math::abs(xyzw) >= s, SplitComplex4D(0.0f, 0.0f), "Cannot inverse square root a split complex number with a more significant pseudoscalar than scalar.");
#endif
	const real_t sum = 1.0f / Math::sqrt(s + xyzw); // c+d
	const real_t prod = xyzw / (2.0f * (xyzw * xyzw - s * s)); // cd
	//c, d = (c+d)/2 +- sqrt( (c+d)^2 - 4 cd)/2
	const real_t sqrt_part = Math::sqrt(sum * sum - 4.0f * prod);
	const real_t c = (sum + sqrt_part) * 0.5f;
	const real_t d = (sum - sqrt_part) * 0.5f;
	return SplitComplex4D(c, d);
}

SplitComplex4D SplitComplex4D::square_root() const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(s < 0.0f, SplitComplex4D(0.0f, 0.0f), "Cannot square root a split complex number with a negative scalar.");
	ERR_FAIL_COND_V_MSG(Math::abs(xyzw) > s, SplitComplex4D(0.0f, 0.0f), "Cannot square root a split complex number with a more significant pseudoscalar than scalar.");
#endif
	const real_t c = Math::sqrt((s + Math::sqrt(s * s - xyzw * xyzw)) * 0.5f);
	if (unlikely(c == 0.0f)) {
		return SplitComplex4D(0.0f, 0.0f);
	}
	const real_t d = xyzw / (2.0f * c);
	return SplitComplex4D(c, d);
}

// Returns true if the SplitComplex4D is valid for all operations, meaning
// that inverse, square root, and inverse square root are all possible.
bool SplitComplex4D::is_valid() const {
	return s > 0.0f && Math::abs(xyzw) < s;
}

// Trivial getters and setters.

real_t SplitComplex4D::get_scalar() const {
	return parts.scalar;
}

void SplitComplex4D::set_scalar(const real_t p_scalar) {
	parts.scalar = p_scalar;
}

real_t SplitComplex4D::get_pseudoscalar() const {
	return parts.pseudoscalar;
}

void SplitComplex4D::set_pseudoscalar(const real_t p_pseudoscalar) {
	parts.pseudoscalar = p_pseudoscalar;
}

// Operators.

bool SplitComplex4D::is_equal_approx(const SplitComplex4D &p_other) const {
	return Math::is_equal_approx(parts.scalar, p_other.parts.scalar) && Math::is_equal_approx(parts.pseudoscalar, p_other.parts.pseudoscalar);
}

bool SplitComplex4D::operator==(const SplitComplex4D &p_v) const {
	return (parts.scalar == p_v.parts.scalar && parts.pseudoscalar == p_v.parts.pseudoscalar);
}

bool SplitComplex4D::operator!=(const SplitComplex4D &p_v) const {
	return (parts.scalar != p_v.parts.scalar || parts.pseudoscalar != p_v.parts.pseudoscalar);
}

SplitComplex4D SplitComplex4D::operator-() const {
	return SplitComplex4D(-parts.scalar, -parts.pseudoscalar);
}

SplitComplex4D &SplitComplex4D::operator+=(const SplitComplex4D &p_v) {
	parts.scalar += p_v.parts.scalar;
	parts.pseudoscalar += p_v.parts.pseudoscalar;
	return *this;
}

SplitComplex4D SplitComplex4D::operator+(const SplitComplex4D &p_v) const {
	return SplitComplex4D(parts.scalar + p_v.parts.scalar, parts.pseudoscalar + p_v.parts.pseudoscalar);
}

SplitComplex4D &SplitComplex4D::operator-=(const SplitComplex4D &p_v) {
	parts.scalar -= p_v.parts.scalar;
	parts.pseudoscalar -= p_v.parts.pseudoscalar;
	return *this;
}

SplitComplex4D SplitComplex4D::operator-(const SplitComplex4D &p_v) const {
	return SplitComplex4D(parts.scalar - p_v.parts.scalar, parts.pseudoscalar - p_v.parts.pseudoscalar);
}

SplitComplex4D &SplitComplex4D::operator*=(const SplitComplex4D &p_child) {
	// Can't do an in-place multiply because the operation requires the original values throughout the calculation.
	SplitComplex4D result = *this * p_child;
	parts.scalar = result.parts.scalar;
	parts.pseudoscalar = result.parts.pseudoscalar;
	return *this;
}

SplitComplex4D SplitComplex4D::operator*(const SplitComplex4D &p_b) const {
	SplitComplex4D result;
	result.s = s * p_b.s + xyzw * p_b.xyzw; // Scalar
	result.xyzw = s * p_b.xyzw + xyzw * p_b.s; // XYZW
	return result;
}

SplitComplex4D &SplitComplex4D::operator+=(const real_t p_scalar) {
	parts.scalar += p_scalar;
	return *this;
}

SplitComplex4D SplitComplex4D::operator+(const real_t p_scalar) const {
	SplitComplex4D result = *this;
	result.parts.scalar += p_scalar;
	return result;
}

SplitComplex4D &SplitComplex4D::operator-=(const real_t p_scalar) {
	parts.scalar -= p_scalar;
	return *this;
}

SplitComplex4D SplitComplex4D::operator-(const real_t p_scalar) const {
	SplitComplex4D result = *this;
	result.parts.scalar -= p_scalar;
	return result;
}

SplitComplex4D &SplitComplex4D::operator*=(const real_t p_scalar) {
	parts.scalar *= p_scalar;
	parts.pseudoscalar *= p_scalar;
	return *this;
}

SplitComplex4D SplitComplex4D::operator*(const real_t p_scalar) const {
	return SplitComplex4D(parts.scalar * p_scalar, parts.pseudoscalar * p_scalar);
}

SplitComplex4D &SplitComplex4D::operator/=(const real_t p_scalar) {
	parts.scalar /= p_scalar;
	parts.pseudoscalar /= p_scalar;
	return *this;
}

SplitComplex4D SplitComplex4D::operator/(const real_t p_scalar) const {
	return SplitComplex4D(parts.scalar / p_scalar, parts.pseudoscalar / p_scalar);
}

SplitComplex4D operator+(const real_t p_scalar, const SplitComplex4D &p_rotor) {
	SplitComplex4D result = p_rotor;
	result.parts.scalar += p_scalar;
	return result;
}

SplitComplex4D operator*(const real_t p_scalar, const SplitComplex4D &p_rotor) {
	return SplitComplex4D(p_scalar * p_rotor.parts.scalar, p_scalar * p_rotor.parts.pseudoscalar);
}

SplitComplex4D SplitComplex4D::from_array(const PackedRealArray &p_from_array) {
	const int stop_index = MIN(p_from_array.size(), 2);
	SplitComplex4D split;
	for (int i = 0; i < stop_index; i++) {
		split.elements[i] = p_from_array[i];
	}
	return split;
}

PackedRealArray SplitComplex4D::to_array() const {
	PackedRealArray arr;
	arr.resize(2);
	real_t *ptr = arr.ptrw();
	ptr[0] = s;
	ptr[1] = xyzw;
	return arr;
}

SplitComplex4D SplitComplex4D::identity() {
	return SplitComplex4D(1, 0);
}

SplitComplex4D::operator Vector2() const {
	return Vector2(parts.scalar, parts.pseudoscalar);
}

SplitComplex4D::SplitComplex4D(const Vector2 &p_from) {
	parts.scalar = p_from.x;
	parts.pseudoscalar = p_from.y;
}

SplitComplex4D::SplitComplex4D(const real_t p_scalar, const real_t p_pseudoscalar) {
	parts.scalar = p_scalar;
	parts.pseudoscalar = p_pseudoscalar;
}

#include "component_to_string.inc"

SplitComplex4D::operator String() const {
	String ret;
	if (!Math::is_zero_approx(s)) {
		ret += String::num(s);
	}
	APPEND_COMPONENT_TO_STRING(ret, xyzw);
	return ret;
}

static_assert(sizeof(SplitComplex4D) == 2 * sizeof(real_t));
