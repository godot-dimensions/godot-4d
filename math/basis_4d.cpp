#include "basis_4d.h"

#include "geometric_algebra/rotor_4d.h"
#include "vector_4d.h"

// Misc methods.

real_t Basis4D::determinant() const {
	// This method accumulates a lot of floating-point error,
	// but doing all calculations in at least doubles helps a ton.
	double_t a = y.y * (z.z * (double_t)w.w - z.w * (double_t)w.z) - z.y * (y.z * (double_t)w.w - y.w * (double_t)w.z) + w.y * (y.z * (double_t)z.w - y.w * (double_t)z.z);
	double_t b = x.y * (z.z * (double_t)w.w - z.w * (double_t)w.z) - z.y * (x.z * (double_t)w.w - x.w * (double_t)w.z) + w.y * (x.z * (double_t)z.w - x.w * (double_t)z.z);
	double_t c = x.y * (y.z * (double_t)w.w - y.w * (double_t)w.z) - y.y * (x.z * (double_t)w.w - x.w * (double_t)w.z) + w.y * (x.z * (double_t)y.w - x.w * (double_t)y.z);
	double_t d = x.y * (y.z * (double_t)z.w - y.w * (double_t)z.z) - y.y * (x.z * (double_t)z.w - x.w * (double_t)z.z) + z.y * (x.z * (double_t)y.w - x.w * (double_t)y.z);
	return x.x * a - y.x * b + z.x * c - w.x * d;
}

bool Basis4D::is_equal_approx(const Basis4D &p_basis) const {
	return (
			x.is_equal_approx(p_basis.x) &&
			y.is_equal_approx(p_basis.y) &&
			z.is_equal_approx(p_basis.z) &&
			w.is_equal_approx(p_basis.w));
}

// Interpolation.

Basis4D Basis4D::lerp(const Basis4D &p_to, const real_t p_weight) const {
	return Basis4D(
			x.lerp(p_to.x, p_weight),
			y.lerp(p_to.y, p_weight),
			z.lerp(p_to.z, p_weight),
			w.lerp(p_to.w, p_weight));
}

Basis4D Basis4D::slerp(const Basis4D &p_to, const real_t p_weight) const {
	const Vector4 scale = get_scale().lerp(p_to.get_scale(), p_weight);
	Basis4D ret = orthonormalized().slerp_rotation(p_to.orthonormalized(), p_weight);
	ret.scale_local(scale);
	return ret;
}

Basis4D Basis4D::slerp_rotation(const Basis4D &p_to, const real_t p_weight) const {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(!is_rotation(), Basis4D(), "Basis4D.slerp_rotation: The from basis " + operator String() + " must be a pure rotation.");
	ERR_FAIL_COND_V_MSG(!p_to.is_rotation(), Basis4D(), "Basis4D.slerp_rotation: The to basis " + p_to.operator String() + " must be a pure rotation.");
#endif
	const Rotor4D a = Rotor4D::from_basis(*this);
	const Rotor4D b = Rotor4D::from_basis(p_to);
	return a.slerp(b, p_weight).to_basis();
}

// Transformation methods.

Basis4D Basis4D::compose(const Basis4D &p_child) const {
	return *this * p_child;
}

Basis4D Basis4D::transform_to(const Basis4D &p_to) const {
	return p_to * inverse();
}

Vector4 Basis4D::xform(const Vector4 &p_vector) const {
	return Vector4(
			get_row(0).dot(p_vector),
			get_row(1).dot(p_vector),
			get_row(2).dot(p_vector),
			get_row(3).dot(p_vector));
}

Vector4 Basis4D::xform_inv(const Vector4 &p_vector) const {
	return inverse().xform(p_vector);
}

Vector4 Basis4D::xform_transposed(const Vector4 &p_vector) const {
	return Vector4(
			x.dot(p_vector),
			y.dot(p_vector),
			z.dot(p_vector),
			w.dot(p_vector));
}

// Rotation methods.

Bivector4D Basis4D::rotate_bivector(const Bivector4D &p_bivector) const {
	// This commented-out code is simple, following the math, but computationally wasteful:
	//Basis4D antisymmetric_matrix = Basis4D(
	//		Vector4(0, -p_bivector.xy, -p_bivector.xz, -p_bivector.xw),
	//		Vector4(p_bivector.xy, 0, -p_bivector.yz, -p_bivector.yw),
	//		Vector4(p_bivector.xz, p_bivector.yz, 0, -p_bivector.zw),
	//		Vector4(p_bivector.xw, p_bivector.yw, p_bivector.zw, 0));
	//Basis4D transformed = *this * antisymmetric_matrix * transposed();
	// Unrolled version of the above calculation for efficiency (avoid computing half the matrix unnecessarily):
	real_t wip_x_x = y.x * -p_bivector.xy + z.x * -p_bivector.xz + w.x * -p_bivector.xw;
	real_t wip_x_y = x.x * +p_bivector.xy + z.x * -p_bivector.yz + w.x * -p_bivector.yw;
	real_t wip_x_z = x.x * +p_bivector.xz + y.x * +p_bivector.yz + w.x * -p_bivector.zw;
	real_t wip_x_w = x.x * +p_bivector.xw + y.x * +p_bivector.yw + z.x * +p_bivector.zw;
	real_t wip_y_x = y.y * -p_bivector.xy + z.y * -p_bivector.xz + w.y * -p_bivector.xw;
	real_t wip_y_y = x.y * +p_bivector.xy + z.y * -p_bivector.yz + w.y * -p_bivector.yw;
	real_t wip_y_z = x.y * +p_bivector.xz + y.y * +p_bivector.yz + w.y * -p_bivector.zw;
	real_t wip_y_w = x.y * +p_bivector.xw + y.y * +p_bivector.yw + z.y * +p_bivector.zw;
	real_t wip_z_x = y.z * -p_bivector.xy + z.z * -p_bivector.xz + w.z * -p_bivector.xw;
	real_t wip_z_y = x.z * +p_bivector.xy + z.z * -p_bivector.yz + w.z * -p_bivector.yw;
	real_t wip_z_z = x.z * +p_bivector.xz + y.z * +p_bivector.yz + w.z * -p_bivector.zw;
	real_t wip_z_w = x.z * +p_bivector.xw + y.z * +p_bivector.yw + z.z * +p_bivector.zw;
	Bivector4D rotated;
	rotated.xy = wip_x_x * x.y + wip_x_y * y.y + wip_x_z * z.y + wip_x_w * w.y;
	rotated.xz = wip_x_x * x.z + wip_x_y * y.z + wip_x_z * z.z + wip_x_w * w.z;
	rotated.xw = wip_x_x * x.w + wip_x_y * y.w + wip_x_z * z.w + wip_x_w * w.w;
	rotated.yz = wip_y_x * x.z + wip_y_y * y.z + wip_y_z * z.z + wip_y_w * w.z;
	rotated.yw = wip_y_x * x.w + wip_y_y * y.w + wip_y_z * z.w + wip_y_w * w.w;
	rotated.zw = wip_z_x * x.w + wip_z_y * y.w + wip_z_z * z.w + wip_z_w * w.w;
	return rotated;
}

Basis4D Basis4D::rotate_plane_global(const Vector4 &p_plane_from, const Vector4 &p_plane_to, const real_t p_angle) const {
	return Basis4D(Vector4D::rotate_in_plane(x, p_plane_from, p_plane_to, p_angle),
			Vector4D::rotate_in_plane(y, p_plane_from, p_plane_to, p_angle),
			Vector4D::rotate_in_plane(z, p_plane_from, p_plane_to, p_angle),
			Vector4D::rotate_in_plane(w, p_plane_from, p_plane_to, p_angle));
}

Basis4D Basis4D::rotate_plane_local(const Vector4 &p_local_plane_from, const Vector4 &p_local_plane_to, const real_t p_angle) const {
	const Vector4 global_plane_from = xform(p_local_plane_from);
	const Vector4 global_plane_to = xform(p_local_plane_to);
	return rotate_plane_global(global_plane_from, global_plane_to, p_angle);
}

// Inversion methods.

Basis4D Basis4D::inverse() const {
	return adjugate() / determinant();
}

void Basis4D::transpose() {
	SWAP(x.y, y.x);
	SWAP(x.z, z.x);
	SWAP(x.w, w.x);
	SWAP(y.z, z.y);
	SWAP(y.w, w.y);
	SWAP(z.w, w.z);
}

Basis4D Basis4D::transposed() const {
	Basis4D tr = *this;
	tr.transpose();
	return tr;
}

Basis4D Basis4D::adjugate() const {
	// This method accumulates a lot of floating-point error,
	// but doing all calculations in at least doubles helps a ton.
	const real_t xx = y.y * (double_t)z.z * w.w + z.y * (double_t)w.z * y.w + w.y * (double_t)y.z * z.w - w.y * (double_t)z.z * y.w - z.y * (double_t)y.z * w.w - y.y * (double_t)w.z * z.w;
	const real_t xy = w.y * (double_t)z.z * x.w + z.y * (double_t)x.z * w.w + x.y * (double_t)w.z * z.w - x.y * (double_t)z.z * w.w - z.y * (double_t)w.z * x.w - w.y * (double_t)x.z * z.w;
	const real_t xz = x.y * (double_t)y.z * w.w + y.y * (double_t)w.z * x.w + w.y * (double_t)x.z * y.w - w.y * (double_t)y.z * x.w - y.y * (double_t)x.z * w.w - x.y * (double_t)w.z * y.w;
	const real_t xw = z.y * (double_t)y.z * x.w + y.y * (double_t)x.z * z.w + x.y * (double_t)z.z * y.w - x.y * (double_t)y.z * z.w - y.y * (double_t)z.z * x.w - z.y * (double_t)x.z * y.w;
	const real_t yx = w.x * (double_t)z.z * y.w + z.x * (double_t)y.z * w.w + y.x * (double_t)w.z * z.w - y.x * (double_t)z.z * w.w - z.x * (double_t)w.z * y.w - w.x * (double_t)y.z * z.w;
	const real_t yy = x.x * (double_t)z.z * w.w + z.x * (double_t)w.z * x.w + w.x * (double_t)x.z * z.w - w.x * (double_t)z.z * x.w - z.x * (double_t)x.z * w.w - x.x * (double_t)w.z * z.w;
	const real_t yz = w.x * (double_t)y.z * x.w + y.x * (double_t)x.z * w.w + x.x * (double_t)w.z * y.w - x.x * (double_t)y.z * w.w - y.x * (double_t)w.z * x.w - w.x * (double_t)x.z * y.w;
	const real_t yw = x.x * (double_t)y.z * z.w + y.x * (double_t)z.z * x.w + z.x * (double_t)x.z * y.w - z.x * (double_t)y.z * x.w - y.x * (double_t)x.z * z.w - x.x * (double_t)z.z * y.w;
	const real_t zx = y.x * (double_t)z.y * w.w + z.x * (double_t)w.y * y.w + w.x * (double_t)y.y * z.w - w.x * (double_t)z.y * y.w - z.x * (double_t)y.y * w.w - y.x * (double_t)w.y * z.w;
	const real_t zy = w.x * (double_t)z.y * x.w + z.x * (double_t)x.y * w.w + x.x * (double_t)w.y * z.w - x.x * (double_t)z.y * w.w - z.x * (double_t)w.y * x.w - w.x * (double_t)x.y * z.w;
	const real_t zz = x.x * (double_t)y.y * w.w + y.x * (double_t)w.y * x.w + w.x * (double_t)x.y * y.w - w.x * (double_t)y.y * x.w - y.x * (double_t)x.y * w.w - x.x * (double_t)w.y * y.w;
	const real_t zw = z.x * (double_t)y.y * x.w + y.x * (double_t)x.y * z.w + x.x * (double_t)z.y * y.w - x.x * (double_t)y.y * z.w - y.x * (double_t)z.y * x.w - z.x * (double_t)x.y * y.w;
	const real_t wx = w.x * (double_t)z.y * y.z + z.x * (double_t)y.y * w.z + y.x * (double_t)w.y * z.z - y.x * (double_t)z.y * w.z - z.x * (double_t)w.y * y.z - w.x * (double_t)y.y * z.z;
	const real_t wy = x.x * (double_t)z.y * w.z + z.x * (double_t)w.y * x.z + w.x * (double_t)x.y * z.z - w.x * (double_t)z.y * x.z - z.x * (double_t)x.y * w.z - x.x * (double_t)w.y * z.z;
	const real_t wz = w.x * (double_t)y.y * x.z + y.x * (double_t)x.y * w.z + x.x * (double_t)w.y * y.z - x.x * (double_t)y.y * w.z - y.x * (double_t)w.y * x.z - w.x * (double_t)x.y * y.z;
	const real_t ww = x.x * (double_t)y.y * z.z + y.x * (double_t)z.y * x.z + z.x * (double_t)x.y * y.z - z.x * (double_t)y.y * x.z - y.x * (double_t)x.y * z.z - x.x * (double_t)z.y * y.z;
	return Basis4D(xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, wx, wy, wz, ww);
}

// Scale methods.

void Basis4D::scale_global(const Vector4 &p_scale) {
	set_row(0, get_row(0) * p_scale.x);
	set_row(1, get_row(1) * p_scale.y);
	set_row(2, get_row(2) * p_scale.z);
	set_row(3, get_row(3) * p_scale.w);
}

Basis4D Basis4D::scaled_global(const Vector4 &p_scale) const {
	Basis4D m = *this;
	m.scale_global(p_scale);
	return m;
}

void Basis4D::scale_local(const Vector4 &p_scale) {
	x *= p_scale.x;
	y *= p_scale.y;
	z *= p_scale.z;
	w *= p_scale.w;
}

Basis4D Basis4D::scaled_local(const Vector4 &p_scale) const {
	return Basis4D(
			x * p_scale.x,
			y * p_scale.y,
			z * p_scale.z,
			w * p_scale.w);
}

void Basis4D::scale_uniform(const real_t p_scale) {
	x *= p_scale;
	y *= p_scale;
	z *= p_scale;
	w *= p_scale;
}

Vector4 Basis4D::get_scale() const {
	Vector4 scale = get_scale_abs();
	if (determinant() < 0.0f) {
		// In order to have a negative scale, we need to flip the handedness.
		// This means we need to flip an odd number of axes. In 2D we flip Y,
		// in 3D we flip all axes, for 4D we will just flip the W axis.
		scale.w = -scale.w;
	}
	return scale;
}

Vector4 Basis4D::get_scale_abs() const {
	return Vector4(
			x.length(),
			y.length(),
			z.length(),
			w.length());
}

Basis4D Basis4D::scaled_uniform(const real_t p_scale) const {
	return Basis4D(
			x * p_scale,
			y * p_scale,
			z * p_scale,
			w * p_scale);
}

void Basis4D::set_scale(const Vector4 &p_scale) {
	Vector4 new_scale = Vector4(
			Math::is_zero_approx(p_scale.x) ? CMP_EPSILON : p_scale.x,
			Math::is_zero_approx(p_scale.y) ? CMP_EPSILON : p_scale.y,
			Math::is_zero_approx(p_scale.z) ? CMP_EPSILON : p_scale.z,
			Math::is_zero_approx(p_scale.w) ? CMP_EPSILON : p_scale.w);
	Vector4 delta_scale = new_scale / get_scale();
	x *= delta_scale.x;
	y *= delta_scale.y;
	z *= delta_scale.z;
	w *= delta_scale.w;
}

real_t Basis4D::get_uniform_scale() const {
	const real_t det = determinant();
	// Take the fourth root of the determinant. For example, if we have a
	// scale of (2, 2, 2, 2), the determinant will be 16, and the fourth root
	// will be 2. If the determinant is negative, scale of (2, 2, 2, -2),
	// the determinant will be -16, and we want to return -2.
	if (det < 0.0f) {
		return -Math::sqrt(Math::sqrt(-det));
	} else {
		return Math::sqrt(Math::sqrt(det));
	}
}

void Basis4D::set_uniform_scale(const real_t p_uniform_scale) {
	const real_t det = determinant();
	const real_t abs_scale = Math::abs(p_uniform_scale);
	x.normalize();
	x *= abs_scale;
	y.normalize();
	y *= abs_scale;
	z.normalize();
	z *= abs_scale;
	w.normalize();
	// Check if the signs match. We only want positive/negative, not zero, so don't use Godot's SIGN macro.
	if ((det < 0.0f) == (p_uniform_scale < 0.0f)) {
		// If the determinant and the scale have the same sign, we can just multiply the W axis by the scale.
		w *= abs_scale;
	} else {
		// If the determinant and the scale have different signs, we need to flip the W axis.
		w *= -abs_scale;
	}
}

// Validation methods.

void Basis4D::conformalize() {
	const real_t scale = get_uniform_scale();
	orthonormalize();
	*this *= scale;
}

Basis4D Basis4D::conformalized() const {
	const real_t scale = get_uniform_scale();
	return orthonormalized() * scale;
}

void Basis4D::normalize() {
	x.normalize();
	y.normalize();
	z.normalize();
	w.normalize();
}

Basis4D Basis4D::normalized() const {
	Basis4D basis = *this;
	basis.normalize();
	return basis;
}

void Basis4D::orthonormalize() {
	// Gram-Schmidt Process, now in 4D.
	// https://en.wikipedia.org/wiki/Gram-Schmidt_process
	x.normalize();
	y = y - x * x.dot(y);
	y.normalize();
	z = z - x * x.dot(z) - y * y.dot(z);
	z.normalize();
	w = w - x * x.dot(w) - y * y.dot(w) - z * z.dot(w);
	w.normalize();
}

Basis4D Basis4D::orthonormalized() const {
	Basis4D basis = *this;
	basis.orthonormalize();
	return basis;
}

void Basis4D::orthogonalize() {
	Vector4 scale = get_scale_abs();
	orthonormalize();
	scale_local(scale);
}

Basis4D Basis4D::orthogonalized() const {
	Basis4D basis = *this;
	basis.orthogonalize();
	return basis;
}

// Returns true if the basis vectors are orthogonal (perpendicular), so it has no skew or shear, and can be decomposed into rotation and scale.
// See https://en.wikipedia.org/wiki/Orthogonal_basis
bool Basis4D::is_orthogonal() const {
	return Math::is_zero_approx(x.dot(y)) && Math::is_zero_approx(x.dot(z)) && Math::is_zero_approx(y.dot(z)) && Math::is_zero_approx(x.dot(w)) && Math::is_zero_approx(y.dot(w)) && Math::is_zero_approx(z.dot(w));
}

// Returns true if the basis vectors are orthonormal (orthogonal and normalized), so it has no scale, skew, or shear.
// See https://en.wikipedia.org/wiki/Orthonormal_basis
bool Basis4D::is_orthonormal() const {
	const bool is_unscaled = Math::is_equal_approx(x.length_squared(), 1) && Math::is_equal_approx(y.length_squared(), 1) && Math::is_equal_approx(z.length_squared(), 1) && Math::is_equal_approx(w.length_squared(), 1);
	return is_unscaled && is_orthogonal();
}

// Returns true if the basis is conformal (orthogonal, uniform scale, preserves angles and distance ratios).
// See https://en.wikipedia.org/wiki/Conformal_linear_transformation
bool Basis4D::is_conformal() const {
	const real_t x_len_sq = x.length_squared();
	const bool is_uniform_scale = Math::is_equal_approx(x_len_sq, y.length_squared()) && Math::is_equal_approx(x_len_sq, z.length_squared()) && Math::is_equal_approx(x_len_sq, w.length_squared());
	return is_uniform_scale && is_orthogonal();
}

bool Basis4D::is_diagonal() const {
	return (
			Math::is_zero_approx(x.y) && Math::is_zero_approx(y.x) &&
			Math::is_zero_approx(x.z) && Math::is_zero_approx(z.x) &&
			Math::is_zero_approx(x.w) && Math::is_zero_approx(w.x) &&
			Math::is_zero_approx(y.z) && Math::is_zero_approx(z.y) &&
			Math::is_zero_approx(y.w) && Math::is_zero_approx(w.y) &&
			Math::is_zero_approx(z.w) && Math::is_zero_approx(w.z));
}

bool Basis4D::is_rotation() const {
	return is_conformal() && Math::is_equal_approx(determinant(), 1, (real_t)UNIT_EPSILON);
}

// Operators.

bool Basis4D::operator==(const Basis4D &p_basis) const {
	return (
			x == p_basis.x &&
			y == p_basis.y &&
			z == p_basis.z &&
			w == p_basis.w);
}

bool Basis4D::operator!=(const Basis4D &p_basis) const {
	return (
			x != p_basis.x ||
			y != p_basis.y ||
			z != p_basis.z ||
			w != p_basis.w);
}

Basis4D Basis4D::operator-() const {
	return Basis4D(-x, -y, -z, -w);
}

void Basis4D::operator*=(const Basis4D &p_matrix) {
	*this = *this * p_matrix;
}

Basis4D Basis4D::operator*(const Basis4D &p_matrix) const {
	return Basis4D(
			xform(p_matrix.x),
			xform(p_matrix.y),
			xform(p_matrix.z),
			xform(p_matrix.w));
}

Vector4 Basis4D::operator*(const Vector4 &p_vector) const {
	return xform(p_vector);
}

void Basis4D::operator+=(const Basis4D &p_matrix) {
	x += p_matrix.x;
	y += p_matrix.y;
	z += p_matrix.z;
	w += p_matrix.w;
}

Basis4D Basis4D::operator+(const Basis4D &p_matrix) const {
	return Basis4D(
			x + p_matrix.x,
			y + p_matrix.y,
			z + p_matrix.z,
			w + p_matrix.w);
}

void Basis4D::operator-=(const Basis4D &p_matrix) {
	x -= p_matrix.x;
	y -= p_matrix.y;
	z -= p_matrix.z;
	w -= p_matrix.w;
}

Basis4D Basis4D::operator-(const Basis4D &p_matrix) const {
	return Basis4D(
			x - p_matrix.x,
			y - p_matrix.y,
			z - p_matrix.z,
			w - p_matrix.w);
}

void Basis4D::operator*=(const real_t p_val) {
	x *= p_val;
	y *= p_val;
	z *= p_val;
	w *= p_val;
}

Basis4D Basis4D::operator*(const real_t p_val) const {
	return Basis4D(
			x * p_val,
			y * p_val,
			z * p_val,
			w * p_val);
}

void Basis4D::operator/=(const real_t p_val) {
	x /= p_val;
	y /= p_val;
	z /= p_val;
	w /= p_val;
}

Basis4D Basis4D::operator/(const real_t p_val) const {
	return Basis4D(
			x / p_val,
			y / p_val,
			z / p_val,
			w / p_val);
}

Basis4D::operator String() const {
	return "[X: " + x.operator String() +
			", Y: " + y.operator String() +
			", Z: " + z.operator String() +
			", W: " + w.operator String() + "]";
}

// Conversion.

Basis4D Basis4D::from_3d(const Basis &p_from_3d) {
	return Basis4D(
			Vector4(p_from_3d.rows[0][0], p_from_3d.rows[1][0], p_from_3d.rows[2][0], 0),
			Vector4(p_from_3d.rows[0][1], p_from_3d.rows[1][1], p_from_3d.rows[2][1], 0),
			Vector4(p_from_3d.rows[0][2], p_from_3d.rows[1][2], p_from_3d.rows[2][2], 0),
			Vector4(0, 0, 0, 1));
}

Basis Basis4D::to_3d(const bool p_orthonormalized) const {
	// Remember that Godot's Basis stores rows for low-level optimization reasons.
	Basis basis = Basis(x.x, y.x, z.x, x.y, y.y, z.y, x.z, y.z, z.z);
	if (p_orthonormalized) {
		basis.orthonormalize();
	}
	return basis;
}

Basis4D::operator Projection() const {
	return Projection(x, y, z, w);
}

Basis4D Basis4D::from_array(const PackedRealArray &p_from_array) {
	ERR_FAIL_COND_V_MSG(p_from_array.size() < 16, Basis4D(), "Array must contain 16 numbers.");
	return Basis4D(p_from_array[0], p_from_array[1], p_from_array[2], p_from_array[3],
			p_from_array[4], p_from_array[5], p_from_array[6], p_from_array[7],
			p_from_array[8], p_from_array[9], p_from_array[10], p_from_array[11],
			p_from_array[12], p_from_array[13], p_from_array[14], p_from_array[15]);
}

PackedRealArray Basis4D::to_array() const {
	PackedRealArray arr;
	arr.resize(16);
	real_t *ptr = arr.ptrw();
	ptr[0] = x.x;
	ptr[1] = x.y;
	ptr[2] = x.z;
	ptr[3] = x.w;
	ptr[4] = y.x;
	ptr[5] = y.y;
	ptr[6] = y.z;
	ptr[7] = y.w;
	ptr[8] = z.x;
	ptr[9] = z.y;
	ptr[10] = z.z;
	ptr[11] = z.w;
	ptr[12] = w.x;
	ptr[13] = w.y;
	ptr[14] = w.z;
	ptr[15] = w.w;
	return arr;
}

Basis4D::Basis4D(const Projection &p_from) {
	x = p_from[0];
	y = p_from[1];
	z = p_from[2];
	w = p_from[3];
}

// Constructors.

Basis4D Basis4D::from_scale(const Vector4 &p_scale) {
	return Basis4D(
			Vector4(p_scale.x, 0, 0, 0),
			Vector4(0, p_scale.y, 0, 0),
			Vector4(0, 0, p_scale.z, 0),
			Vector4(0, 0, 0, p_scale.w));
}

Basis4D Basis4D::from_scale(const real_t p_x, const real_t p_y, const real_t p_z, const real_t p_w) {
	return Basis4D(
			Vector4(p_x, 0, 0, 0),
			Vector4(0, p_y, 0, 0),
			Vector4(0, 0, p_z, 0),
			Vector4(0, 0, 0, p_w));
}

Basis4D Basis4D::from_scale_uniform(const real_t p_scale) {
	if (p_scale < 0.0f) {
		// For negative uniform scales, only the W axis is flipped.
		return Basis4D(
				Vector4(-p_scale, 0, 0, 0),
				Vector4(0, -p_scale, 0, 0),
				Vector4(0, 0, -p_scale, 0),
				Vector4(0, 0, 0, p_scale));
	}
	return Basis4D(
			Vector4(p_scale, 0, 0, 0),
			Vector4(0, p_scale, 0, 0),
			Vector4(0, 0, p_scale, 0),
			Vector4(0, 0, 0, p_scale));
}

#define FROM_SINGLE_ANGLE(a, b)                                   \
	Basis4D Basis4D::from_##a##b(const real_t p_##a##b##_angle) { \
		real_t sin_angle = Math::sin(p_##a##b##_angle);           \
		real_t cos_angle = Math::cos(p_##a##b##_angle);           \
		Basis4D basis = Basis4D();                                \
		basis.a.a = cos_angle;                                    \
		basis.a.b = sin_angle;                                    \
		basis.b.a = -sin_angle;                                   \
		basis.b.b = cos_angle;                                    \
		return basis;                                             \
	}

FROM_SINGLE_ANGLE(y, z)
FROM_SINGLE_ANGLE(z, x)
FROM_SINGLE_ANGLE(x, y)
FROM_SINGLE_ANGLE(x, w)
FROM_SINGLE_ANGLE(w, y)
FROM_SINGLE_ANGLE(z, w)

Basis4D Basis4D::looking_at(const Vector4 &p_target, const Vector4 &p_perp, const Vector4 &p_up, bool p_use_model_front) {
#ifdef MATH_CHECKS
	ERR_FAIL_COND_V_MSG(p_target.is_zero_approx(), Basis4D(), "Basis4D.looking_at: The target vector can't be zero.");
	ERR_FAIL_COND_V_MSG(p_up.is_zero_approx(), Basis4D(), "Basis4D.looking_at: The up vector can't be zero.");
	ERR_FAIL_COND_V_MSG(p_perp.is_zero_approx(), Basis4D(), "Basis4D.looking_at: The perpendicular vector can't be zero.");
#endif
	Vector4 z = p_target.normalized();
	if (!p_use_model_front) {
		z = -z;
	}
	Vector4 x = Vector4D::perpendicular(z, p_up, p_perp);
	if (x.is_zero_approx()) {
		WARN_PRINT("Basis4D.looking_at: Target, up, and/or perpendicular vectors are colinear.");
		x = Vector4(1, 0, 0, 0);
	} else {
		x.normalize();
	}
	Vector4 w = Vector4D::perpendicular(x, p_up, z);
	if (w.is_zero_approx()) {
		WARN_PRINT("Basis4D.looking_at: Target, up, and/or perpendicular vectors are colinear.");
		w = Vector4(0, 0, 0, 1);
	} else {
		w.normalize();
	}
	Vector4 y = Vector4D::perpendicular(x, z, w);
	if (y.is_zero_approx()) {
		WARN_PRINT("Basis4D.looking_at: Target, up, and/or perpendicular vectors are colinear.");
		y = Vector4(0, 1, 0, 0);
	} else {
		y.normalize();
	}
	const Basis4D b = Basis4D(x, y, z, w);
	ERR_FAIL_COND_V_MSG(!Math::is_equal_approx(b.determinant(), (real_t)1), Basis4D(), "Basis4D.looking_at: Failed to calculate a valid rotation.");
	return b;
}

static_assert(sizeof(Basis4D) == 16 * sizeof(real_t));
