#include "transform_4d.h"

// Misc methods.

real_t Transform4D::determinant() const {
	return basis.determinant();
}

Transform4D Transform4D::lerp(const Transform4D &p_transform, real_t p_weight) const {
	Transform4D interp;

	interp.basis = basis.lerp(p_transform.basis, p_weight);
	interp.origin = origin.lerp(p_transform.origin, p_weight);

	return interp;
}

bool Transform4D::is_equal_approx(const Transform4D &p_transform) const {
	return basis.is_equal_approx(p_transform.basis) && origin.is_equal_approx(p_transform.origin);
}

void Transform4D::translate_local(const Vector4 &p_translation) {
	origin += basis * p_translation;
}

Transform4D Transform4D::translated_local(const Vector4 &p_translation) const {
	Transform4D t = *this;
	t.translate_local(p_translation);
	return t;
}

// Transformation methods.

Transform4D Transform4D::compose(const Transform4D &p_child_transform) const {
	return *this * p_child_transform;
}

Transform4D Transform4D::transform_to(const Transform4D &p_to) const {
	return p_to * inverse();
}

Vector4 Transform4D::xform(const Vector4 &p_vector) const {
	return basis.xform(p_vector) + origin;
}

PackedVector4Array Transform4D::xform_many(const PackedVector4Array &p_vectors) const {
	PackedVector4Array array;
	array.resize(p_vectors.size());

	const Vector4 *r = p_vectors.ptr();
	Vector4 *w = array.ptrw();

	for (int i = 0; i < p_vectors.size(); ++i) {
		w[i] = xform(r[i]);
	}
	return array;
}

Basis4D Transform4D::xform_basis(const Basis4D &p_basis) const {
	return basis * p_basis;
}

Vector4 Transform4D::xform_inv(const Vector4 &p_vector) const {
	return inverse().xform(p_vector);
}

PackedVector4Array Transform4D::xform_inv_many(const PackedVector4Array &p_vectors) const {
	PackedVector4Array array;
	array.resize(p_vectors.size());

	const Vector4 *r = p_vectors.ptr();
	Vector4 *w = array.ptrw();

	for (int i = 0; i < p_vectors.size(); ++i) {
		w[i] = xform_inv(r[i]);
	}
	return array;
}

Basis4D Transform4D::xform_inv_basis(const Basis4D &p_basis) const {
	return basis.inverse() * p_basis;
}

Vector4 Transform4D::xform_transposed(const Vector4 &p_vector) const {
	return basis.xform_transposed(p_vector - origin);
}

PackedVector4Array Transform4D::xform_transposed_many(const PackedVector4Array &p_vectors) const {
	PackedVector4Array array;
	array.resize(p_vectors.size());

	const Vector4 *r = p_vectors.ptr();
	Vector4 *w = array.ptrw();

	for (int i = 0; i < p_vectors.size(); ++i) {
		w[i] = xform_transposed(r[i]);
	}
	return array;
}

Basis4D Transform4D::xform_transposed_basis(const Basis4D &p_basis) const {
	return basis.transposed() * p_basis;
}

// Inversion methods.

Transform4D Transform4D::inverse() const {
	Transform4D ret = Transform4D(basis.inverse());
	ret.origin = ret.basis.xform(-origin);
	return ret;
}

Transform4D Transform4D::inverse_basis() const {
	return Transform4D(basis.inverse());
}

Transform4D Transform4D::inverse_transposed() const {
	Transform4D ret = Transform4D(basis.transposed());
	ret.origin = ret.basis.xform(-origin);
	return ret;
}

// Rotation methods.

void Transform4D::rotate_global(const Euler4D &p_euler) {
	Basis4D rotation = p_euler.to_basis();
	basis = rotation * basis;
	origin = rotation.xform(origin);
}

Transform4D Transform4D::rotated_global(const Euler4D &p_euler) const {
	Basis4D rotation = p_euler.to_basis();
	return Transform4D(rotation * basis, rotation.xform(origin));
}

void Transform4D::rotate_local(const Euler4D &p_euler) {
	basis *= p_euler.to_basis();
}

Transform4D Transform4D::rotated_local(const Euler4D &p_euler) const {
	return Transform4D(basis * p_euler.to_basis(), origin);
}

Euler4D Transform4D::get_rotation() const {
	return Euler4D::from_basis(basis);
}

void Transform4D::set_rotation(const Euler4D &p_euler) {
	basis = p_euler.to_basis();
}

Euler4D Transform4D::get_rotation_degrees() const {
	return Euler4D::from_basis(basis).rad_to_deg();
}

void Transform4D::set_rotation_degrees(const Euler4D &p_euler) {
	basis = p_euler.deg_to_rad().to_basis();
}

// Geometric algebra rotation altering methods.

void Transform4D::rotate_bivector_magnitude(const Bivector4D &p_bivector) {
	Rotor4D rot = Rotor4D::from_bivector_magnitude(p_bivector);
	basis = rot.rotate_basis(basis);
	origin = rot.rotate_vector(origin);
}

void Transform4D::rotate_bivector_magnitude_local(const Bivector4D &p_bivector_local) {
	Rotor4D rot_local = Rotor4D::from_bivector_magnitude(p_bivector_local);
	basis *= rot_local.to_basis();
}

void Transform4D::rotate_rotor(const Rotor4D &p_rotor) {
	basis = p_rotor.rotate_basis(basis);
	origin = p_rotor.rotate_vector(origin);
}

void Transform4D::rotate_rotor_local(const Rotor4D &p_rotor_local) {
	basis *= p_rotor_local.to_basis();
}

// Geometric algebra rotation properties.

void Transform4D::set_rotation_bivector_magnitude(const Bivector4D &p_bivector) {
	const Vector4 scale = basis.get_scale();
	Rotor4D rot = Rotor4D::from_bivector_magnitude(p_bivector);
	basis = rot.to_basis().scaled_local(scale);
}

void Transform4D::set_rotation_rotor(const Rotor4D &p_rotor) {
	const Vector4 scale = basis.get_scale();
	basis = p_rotor.to_basis().scaled_local(scale);
}

// Scale methods.

void Transform4D::scale_global(const Vector4 &p_scale) {
	basis.scale_global(p_scale);
	origin *= p_scale;
}

Transform4D Transform4D::scaled_global(const Vector4 &p_scale) const {
	Transform4D t = *this;
	t.scale_global(p_scale);
	return t;
}

void Transform4D::scale_local(const Vector4 &p_scale) {
	basis.scale_local(p_scale);
}

Transform4D Transform4D::scaled_local(const Vector4 &p_scale) const {
	Transform4D t = *this;
	t.scale_local(p_scale);
	return t;
}

void Transform4D::scale_uniform_global(const real_t p_scale) {
	basis.scale_uniform(p_scale);
	origin *= p_scale;
}

Transform4D Transform4D::scaled_uniform_global(const real_t p_scale) const {
	Transform4D t = *this;
	t.scale_uniform_global(p_scale);
	return t;
}

void Transform4D::scale_uniform_local(const real_t p_scale) {
	basis.scale_uniform(p_scale);
}

Transform4D Transform4D::scaled_uniform_local(const real_t p_scale) const {
	Transform4D t = *this;
	t.scale_uniform_local(p_scale);
	return t;
}

Vector4 Transform4D::get_scale() const {
	return basis.get_scale();
}

Vector4 Transform4D::get_scale_abs() const {
	return basis.get_scale_abs();
}

void Transform4D::set_scale(const Vector4 &p_scale) {
	basis.set_scale(p_scale);
}

real_t Transform4D::get_uniform_scale() const {
	return basis.get_uniform_scale();
}

void Transform4D::set_uniform_scale(const real_t p_scale) {
	basis.set_uniform_scale(p_scale);
}

// Validation methods.

void Transform4D::conformalize() {
	basis.conformalize();
}

Transform4D Transform4D::conformalized() const {
	return Transform4D(basis.conformalized(), origin);
}

void Transform4D::orthonormalize() {
	basis.orthonormalize();
}

Transform4D Transform4D::orthonormalized() const {
	Transform4D ortho = *this;
	ortho.orthonormalize();
	return ortho;
}

void Transform4D::orthogonalize() {
	basis.orthogonalize();
}

Transform4D Transform4D::orthogonalized() const {
	Transform4D ortho = *this;
	ortho.orthogonalize();
	return ortho;
}

// Operators.

bool Transform4D::operator==(const Transform4D &p_transform) const {
	return (basis == p_transform.basis && origin == p_transform.origin);
}

bool Transform4D::operator!=(const Transform4D &p_transform) const {
	return (basis != p_transform.basis || origin != p_transform.origin);
}

void Transform4D::operator+=(const Transform4D &p_transform) {
	origin += p_transform.origin;
	basis += p_transform.basis;
}

Transform4D Transform4D::operator+(const Transform4D &p_transform) const {
	return Transform4D(basis + p_transform.basis, origin + p_transform.origin);
}

void Transform4D::operator*=(const Transform4D &p_transform) {
	origin = xform(p_transform.origin);
	basis *= p_transform.basis;
}

Transform4D Transform4D::operator*(const Transform4D &p_transform) const {
	return Transform4D(
			basis * p_transform.basis,
			xform(p_transform.origin));
}

Basis4D Transform4D::operator*(const Basis4D &p_basis) const {
	return basis * p_basis;
}

Vector4 Transform4D::operator*(const Vector4 &p_vector) const {
	return basis.xform(p_vector) + origin;
}

void Transform4D::operator*=(const real_t p_val) {
	origin *= p_val;
	basis *= p_val;
}

Transform4D Transform4D::operator*(const real_t p_val) const {
	return Transform4D(basis * p_val, origin * p_val);
}

void Transform4D::operator/=(const real_t p_val) {
	origin /= p_val;
	basis /= p_val;
}

Transform4D Transform4D::operator/(const real_t p_val) const {
	return Transform4D(basis / p_val, origin / p_val);
}

Transform4D::operator String() const {
	return "[X: " + basis.x.operator String() +
			", Y: " + basis.y.operator String() +
			", Z: " + basis.z.operator String() +
			", W: " + basis.w.operator String() +
			", O: " + origin.operator String() + "]";
}

// Conversion.

Transform4D Transform4D::from_3d(const Transform3D &p_from_3d) {
	return Transform4D(Basis4D::from_3d(p_from_3d.basis), Vector4(p_from_3d.origin.x, p_from_3d.origin.y, p_from_3d.origin.z, 0.0));
}

Transform3D Transform4D::to_3d(const bool p_orthonormalized) const {
	return Transform3D(basis.to_3d(p_orthonormalized), Vector3(origin.x, origin.y, origin.z));
}

Transform4D Transform4D::from_array(const PackedRealArray &p_array) {
	ERR_FAIL_COND_V_MSG(p_array.size() < 20, Transform4D(), "Array must contain 20 numbers.");
	return Transform4D(
			Basis4D(p_array[0], p_array[1], p_array[2], p_array[3],
					p_array[4], p_array[5], p_array[6], p_array[7],
					p_array[8], p_array[9], p_array[10], p_array[11],
					p_array[12], p_array[13], p_array[14], p_array[15]),
			Vector4(p_array[16], p_array[17], p_array[18], p_array[19]));
}

PackedRealArray Transform4D::to_array() const {
	PackedRealArray arr;
	arr.resize(20);
	real_t *ptr = arr.ptrw();
	ptr[0] = basis.x.x;
	ptr[1] = basis.x.y;
	ptr[2] = basis.x.z;
	ptr[3] = basis.x.w;
	ptr[4] = basis.y.x;
	ptr[5] = basis.y.y;
	ptr[6] = basis.y.z;
	ptr[7] = basis.y.w;
	ptr[8] = basis.z.x;
	ptr[9] = basis.z.y;
	ptr[10] = basis.z.z;
	ptr[11] = basis.z.w;
	ptr[12] = basis.w.x;
	ptr[13] = basis.w.y;
	ptr[14] = basis.w.z;
	ptr[15] = basis.w.w;
	ptr[16] = origin.x;
	ptr[17] = origin.y;
	ptr[18] = origin.z;
	ptr[19] = origin.w;
	return arr;
}

static_assert(sizeof(Transform4D) == 20 * sizeof(real_t));
