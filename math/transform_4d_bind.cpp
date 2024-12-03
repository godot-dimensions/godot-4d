#include "transform_4d_bind.h"

// Beware, this file contains a massive amount of boring repetitive boilerplate code.

#include "euler_4d_bind.h"

// Instance methods, providing a friendly readable Transform4D object, which unfortunately uses lots of memory and is passed by reference.

#define TRANSFORM4D_BIND_RETURN_REF(m_value)        \
	Ref<godot_4d_bind::Transform4D> transform_bind; \
	transform_bind.instantiate();                   \
	transform_bind->set_transform(m_value);         \
	return transform_bind;

// Misc methods.

real_t godot_4d_bind::Transform4D::determinant() const {
	return transform.determinant();
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::lerp(const Ref<godot_4d_bind::Transform4D> &p_transform, real_t p_weight) const {
	TRANSFORM4D_BIND_RETURN_REF(transform.lerp(p_transform->get_transform(), p_weight));
}

bool godot_4d_bind::Transform4D::is_equal_approx(const Ref<godot_4d_bind::Transform4D> &p_transform) const {
	return transform.is_equal_approx(p_transform->get_transform());
}

bool godot_4d_bind::Transform4D::is_equal_exact(const Ref<godot_4d_bind::Transform4D> &p_transform) const {
	return transform == p_transform->get_transform();
}

void godot_4d_bind::Transform4D::translate_local(const Vector4 &p_translation) {
	transform.translate_local(p_translation);
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::translated_local(const Vector4 &p_translation) const {
	TRANSFORM4D_BIND_RETURN_REF(transform.translated_local(p_translation));
}

// Transformation methods.

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::compose(const Ref<godot_4d_bind::Transform4D> &p_child_transform) const {
	TRANSFORM4D_BIND_RETURN_REF(transform.compose(p_child_transform->get_transform()));
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::transform_to(const Ref<godot_4d_bind::Transform4D> &p_to) const {
	TRANSFORM4D_BIND_RETURN_REF(transform.transform_to(p_to->get_transform()));
}

Vector4 godot_4d_bind::Transform4D::xform(const Vector4 &p_vector) const {
	return transform.xform(p_vector);
}

PackedVector4Array godot_4d_bind::Transform4D::xform_many(const PackedVector4Array &p_vectors) const {
	return transform.xform_many(p_vectors);
}

Projection godot_4d_bind::Transform4D::xform_basis(const Projection &p_basis) const {
	return transform.xform_basis(Basis4D(p_basis));
}

Vector4 godot_4d_bind::Transform4D::xform_inv(const Vector4 &p_vector) const {
	return transform.xform_inv(p_vector);
}

PackedVector4Array godot_4d_bind::Transform4D::xform_inv_many(const PackedVector4Array &p_vectors) const {
	return transform.xform_inv_many(p_vectors);
}

Projection godot_4d_bind::Transform4D::xform_inv_basis(const Projection &p_basis) const {
	return transform.xform_inv_basis(Basis4D(p_basis));
}

Vector4 godot_4d_bind::Transform4D::xform_transposed(const Vector4 &p_vector) const {
	return transform.xform_transposed(p_vector);
}

PackedVector4Array godot_4d_bind::Transform4D::xform_transposed_many(const PackedVector4Array &p_vectors) const {
	return transform.xform_transposed_many(p_vectors);
}

Projection godot_4d_bind::Transform4D::xform_transposed_basis(const Projection &p_basis) const {
	return transform.xform_transposed_basis(Basis4D(p_basis));
}

// Inversion methods.

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::inverse() const {
	TRANSFORM4D_BIND_RETURN_REF(transform.inverse());
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::inverse_basis() const {
	TRANSFORM4D_BIND_RETURN_REF(transform.inverse_basis());
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::inverse_transposed() const {
	TRANSFORM4D_BIND_RETURN_REF(transform.inverse_transposed());
}

// Rotation methods.

void godot_4d_bind::Transform4D::rotate_global(const Ref<godot_4d_bind::Euler4D> &p_euler) {
	transform.rotate_global(p_euler->get_euler());
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::rotated_global(const Ref<godot_4d_bind::Euler4D> &p_euler) const {
	TRANSFORM4D_BIND_RETURN_REF(transform.rotated_global(p_euler->get_euler()));
}

void godot_4d_bind::Transform4D::rotate_local(const Ref<godot_4d_bind::Euler4D> &p_euler) {
	transform.rotate_local(p_euler->get_euler());
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::rotated_local(const Ref<godot_4d_bind::Euler4D> &p_euler) const {
	TRANSFORM4D_BIND_RETURN_REF(transform.rotated_local(p_euler->get_euler()));
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Transform4D::get_rotation() const {
	Ref<godot_4d_bind::Euler4D> euler_bind;
	euler_bind.instantiate();
	euler_bind->set_euler(transform.get_rotation());
	return euler_bind;
}

void godot_4d_bind::Transform4D::set_rotation(const Ref<godot_4d_bind::Euler4D> &p_euler) {
	transform.set_rotation(p_euler->get_euler());
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Transform4D::get_rotation_degrees() const {
	Ref<godot_4d_bind::Euler4D> euler_bind;
	euler_bind.instantiate();
	euler_bind->set_euler(transform.get_rotation_degrees());
	return euler_bind;
}

void godot_4d_bind::Transform4D::set_rotation_degrees(const Ref<godot_4d_bind::Euler4D> &p_euler) {
	transform.set_rotation_degrees(p_euler->get_euler());
}

// Geometric algebra rotation altering methods.

void godot_4d_bind::Transform4D::rotate_bivector_magnitude(const AABB &p_bivector) {
	transform.rotate_bivector_magnitude(p_bivector);
}

void godot_4d_bind::Transform4D::rotate_bivector_magnitude_local(const AABB &p_bivector_local) {
	transform.rotate_bivector_magnitude_local(p_bivector_local);
}

void godot_4d_bind::Transform4D::rotate_rotor(const Ref<godot_4d_bind::Rotor4D> &p_rotor) {
	transform.rotate_rotor(p_rotor->get_rotor());
}

void godot_4d_bind::Transform4D::rotate_rotor_local(const Ref<godot_4d_bind::Rotor4D> &p_rotor_local) {
	transform.rotate_rotor_local(p_rotor_local->get_rotor());
}

// Geometric algebra rotation properties.

void godot_4d_bind::Transform4D::set_rotation_bivector_magnitude(const AABB &p_bivector) {
	transform.set_rotation_bivector_magnitude(p_bivector);
}

void godot_4d_bind::Transform4D::set_rotation_rotor(const Ref<godot_4d_bind::Rotor4D> &p_rotor) {
	transform.set_rotation_rotor(p_rotor->get_rotor());
}

// Scale methods.

void godot_4d_bind::Transform4D::scale_global(const Vector4 &p_scale) {
	transform.scale_global(p_scale);
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::scaled_global(const Vector4 &p_scale) const {
	TRANSFORM4D_BIND_RETURN_REF(transform.scaled_global(p_scale));
}

void godot_4d_bind::Transform4D::scale_local(const Vector4 &p_scale) {
	transform.scale_local(p_scale);
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::scaled_local(const Vector4 &p_scale) const {
	TRANSFORM4D_BIND_RETURN_REF(transform.scaled_local(p_scale));
}

void godot_4d_bind::Transform4D::scale_uniform_global(const real_t p_scale) {
	transform.scale_uniform_global(p_scale);
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::scaled_uniform_global(const real_t p_scale) const {
	TRANSFORM4D_BIND_RETURN_REF(transform.scaled_uniform_global(p_scale));
}

void godot_4d_bind::Transform4D::scale_uniform_local(const real_t p_scale) {
	transform.scale_uniform_local(p_scale);
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::scaled_uniform_local(const real_t p_scale) const {
	TRANSFORM4D_BIND_RETURN_REF(transform.scaled_uniform_local(p_scale));
}

Vector4 godot_4d_bind::Transform4D::get_scale() const {
	return transform.get_scale();
}

Vector4 godot_4d_bind::Transform4D::get_scale_abs() const {
	return transform.get_scale_abs();
}

void godot_4d_bind::Transform4D::set_scale(const Vector4 &p_scale) {
	transform.set_scale(p_scale);
}

real_t godot_4d_bind::Transform4D::get_uniform_scale() const {
	return transform.get_uniform_scale();
}

void godot_4d_bind::Transform4D::set_uniform_scale(const real_t p_uniform_scale) {
	transform.set_uniform_scale(p_uniform_scale);
}

// Validation methods.

void godot_4d_bind::Transform4D::orthonormalize() {
	transform.orthonormalize();
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::orthonormalized() const {
	TRANSFORM4D_BIND_RETURN_REF(transform.orthonormalized());
}

void godot_4d_bind::Transform4D::orthogonalize() {
	transform.orthogonalize();
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::orthogonalized() const {
	TRANSFORM4D_BIND_RETURN_REF(transform.orthogonalized());
}

// Conversion.

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::from_3d(const Transform3D &p_from_3d) {
	TRANSFORM4D_BIND_RETURN_REF(::Transform4D::from_3d(p_from_3d));
}

Transform3D godot_4d_bind::Transform4D::to_3d(const bool p_orthonormalized) const {
	return transform.to_3d(p_orthonormalized);
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::from_array(const PackedRealArray &p_array) {
	TRANSFORM4D_BIND_RETURN_REF(::Transform4D::from_array(p_array));
}

PackedRealArray godot_4d_bind::Transform4D::to_array() const {
	return transform.to_array();
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::from_basis(const Projection &p_basis, const Vector4 &p_origin) {
	TRANSFORM4D_BIND_RETURN_REF(::Transform4D(p_basis, p_origin));
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::from_numbers(const real_t p_xx, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yx, const real_t p_yy, const real_t p_yz, const real_t p_yw, const real_t p_zx, const real_t p_zy, const real_t p_zz, const real_t p_zw, const real_t p_wx, const real_t p_wy, const real_t p_wz, const real_t p_ww, const real_t p_ox, const real_t p_oy, const real_t p_oz, const real_t p_ow) {
	TRANSFORM4D_BIND_RETURN_REF(::Transform4D(p_xx, p_xy, p_xz, p_xw, p_yx, p_yy, p_yz, p_yw, p_zx, p_zy, p_zz, p_zw, p_wx, p_wy, p_wz, p_ww, p_ox, p_oy, p_oz, p_ow));
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::from_vectors(const Vector4 &p_x, const Vector4 &p_y, const Vector4 &p_z, const Vector4 &p_w, const Vector4 &p_origin) {
	TRANSFORM4D_BIND_RETURN_REF(::Transform4D(p_x, p_y, p_z, p_w, p_origin));
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Transform4D::copy() const {
	TRANSFORM4D_BIND_RETURN_REF(transform);
}

#if GDEXTENSION
String godot_4d_bind::Transform4D::_to_string() const
#elif GODOT_MODULE
String godot_4d_bind::Transform4D::to_string()
#endif
{
	return transform.operator String();
}

// Static methods, allowing the user to use PackedRealArray or Projection+Vector4 to represent a Transform4D, which saves memory and the latter is passed by value.

// Misc methods.

PackedRealArray godot_4d_bind::Transform4D::array_compose(const PackedRealArray &p_parent_array, const PackedRealArray &p_child_array) {
	return ::Transform4D::from_array(p_parent_array).compose(::Transform4D::from_array(p_child_array)).to_array();
}

PackedRealArray godot_4d_bind::Transform4D::proj_compose_to_array(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_child_basis, const Vector4 &p_child_origin) {
	return ::Transform4D(p_parent_basis, p_parent_origin).compose(::Transform4D(p_child_basis, p_child_origin)).to_array();
}

bool godot_4d_bind::Transform4D::proj_is_equal_approx(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_child_basis, const Vector4 &p_child_origin) {
	return ::Transform4D(p_parent_basis, p_parent_origin).is_equal_approx(::Transform4D(p_child_basis, p_child_origin));
}

bool godot_4d_bind::Transform4D::array_is_equal_approx(const PackedRealArray &p_array_a, const PackedRealArray &p_array_b) {
	return ::Transform4D::from_array(p_array_a).is_equal_approx(::Transform4D::from_array(p_array_b));
}

Vector4 godot_4d_bind::Transform4D::proj_translated_local(const Projection &p_basis, const Vector4 &p_origin, const Vector4 &p_translation) {
	return ::Transform4D(::Basis4D(p_basis), p_origin).translated_local(p_translation).origin;
}

PackedRealArray godot_4d_bind::Transform4D::array_translated_local(const PackedRealArray &p_array, const Vector4 &p_translation) {
	return ::Transform4D::from_array(p_array).translated_local(p_translation).to_array();
}

PackedRealArray godot_4d_bind::Transform4D::proj_translated_local_to_array(const Projection &p_basis, const Vector4 &p_origin, const Vector4 &p_translation) {
	return ::Transform4D(::Basis4D(p_basis), p_origin).translated_local(p_translation).to_array();
}

// Transformation methods.

Vector4 godot_4d_bind::Transform4D::proj_xform(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_child_vector) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform(p_child_vector);
}

Projection godot_4d_bind::Transform4D::proj_xform_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_child_basis) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_basis(Basis4D(p_child_basis));
}

PackedVector4Array godot_4d_bind::Transform4D::proj_xform_many(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const PackedVector4Array &p_child_vectors) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_many(p_child_vectors);
}

Vector4 godot_4d_bind::Transform4D::array_xform(const PackedRealArray &p_transform, const Vector4 &p_child_vector) {
	return ::Transform4D::from_array(p_transform).xform(p_child_vector);
}

Projection godot_4d_bind::Transform4D::array_xform_basis(const PackedRealArray &p_transform, const Projection &p_child_basis) {
	return ::Transform4D::from_array(p_transform).xform_basis(Basis4D(p_child_basis));
}

PackedVector4Array godot_4d_bind::Transform4D::array_xform_many(const PackedRealArray &p_transform, const PackedVector4Array &p_child_vectors) {
	return ::Transform4D::from_array(p_transform).xform_many(p_child_vectors);
}

Vector4 godot_4d_bind::Transform4D::proj_xform_inv(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_vector) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_inv(p_vector);
}

Projection godot_4d_bind::Transform4D::proj_xform_inv_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_basis) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_inv_basis(Basis4D(p_basis));
}

PackedVector4Array godot_4d_bind::Transform4D::proj_xform_inv_many(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const PackedVector4Array &p_vectors) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_inv_many(p_vectors);
}

Vector4 godot_4d_bind::Transform4D::array_xform_inv(const PackedRealArray &p_transform, const Vector4 &p_vector) {
	return ::Transform4D::from_array(p_transform).xform_inv(p_vector);
}

Projection godot_4d_bind::Transform4D::array_xform_inv_basis(const PackedRealArray &p_transform, const Projection &p_basis) {
	return ::Transform4D::from_array(p_transform).xform_inv_basis(Basis4D(p_basis));
}

PackedVector4Array godot_4d_bind::Transform4D::array_xform_inv_many(const PackedRealArray &p_transform, const PackedVector4Array &p_vectors) {
	return ::Transform4D::from_array(p_transform).xform_inv_many(p_vectors);
}

Vector4 godot_4d_bind::Transform4D::proj_xform_transposed(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_vector) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_transposed(p_vector);
}

Projection godot_4d_bind::Transform4D::proj_xform_transposed_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_basis) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_transposed_basis(Basis4D(p_basis));
}

PackedVector4Array godot_4d_bind::Transform4D::proj_xform_transposed_many(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const PackedVector4Array &p_vectors) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_transposed_many(p_vectors);
}

Vector4 godot_4d_bind::Transform4D::array_xform_transposed(const PackedRealArray &p_transform, const Vector4 &p_vector) {
	return ::Transform4D::from_array(p_transform).xform_transposed(p_vector);
}

Projection godot_4d_bind::Transform4D::array_xform_transposed_basis(const PackedRealArray &p_transform, const Projection &p_basis) {
	return ::Transform4D::from_array(p_transform).xform_transposed_basis(Basis4D(p_basis));
}

PackedVector4Array godot_4d_bind::Transform4D::array_xform_transposed_many(const PackedRealArray &p_transform, const PackedVector4Array &p_vectors) {
	return ::Transform4D::from_array(p_transform).xform_transposed_many(p_vectors);
}

// Inversion methods.

PackedRealArray godot_4d_bind::Transform4D::array_inverse(const PackedRealArray &p_transform) {
	return ::Transform4D::from_array(p_transform).inverse().to_array();
}

PackedRealArray godot_4d_bind::Transform4D::proj_inverse_to_array(const Projection &p_basis, const Vector4 &p_origin) {
	return ::Transform4D(p_basis, p_origin).inverse().to_array();
}

Vector4 godot_4d_bind::Transform4D::proj_inverse_origin_with_inverse_basis(const Projection &p_inverse_basis, const Vector4 &p_origin) {
	return ::Basis4D(p_inverse_basis).xform(-p_origin);
}

Vector4 godot_4d_bind::Transform4D::proj_inverse_origin_with_regular_basis(const Projection &p_basis, const Vector4 &p_origin) {
	return ::Basis4D(p_basis).inverse().xform(-p_origin);
}

// Conversion.

Projection godot_4d_bind::Transform4D::proj_from_3d_basis(const Transform3D &p_from_3d) {
	return ::Basis4D::from_3d(p_from_3d.basis);
}

Vector4 godot_4d_bind::Transform4D::proj_from_3d_origin(const Transform3D &p_from_3d) {
	return Vector4(p_from_3d.origin.x, p_from_3d.origin.y, p_from_3d.origin.z, 0.0);
}

PackedRealArray godot_4d_bind::Transform4D::array_from_3d(const Transform3D &p_from_3d) {
	return ::Transform4D::from_3d(p_from_3d).to_array();
}

Transform3D godot_4d_bind::Transform4D::proj_to_3d(const Projection &p_from_basis, const Vector4 &p_from_origin, const bool p_orthonormalized) {
	return ::Transform4D(::Basis4D(p_from_basis), p_from_origin).to_3d(p_orthonormalized);
}

Transform3D godot_4d_bind::Transform4D::array_to_3d(const PackedRealArray &p_from_array, const bool p_orthonormalized) {
	return ::Transform4D::from_array(p_from_array).to_3d(p_orthonormalized);
}

Projection godot_4d_bind::Transform4D::proj_from_array_basis(const PackedRealArray &p_from_array) {
	return ::Basis4D::from_array(p_from_array);
}

Vector4 godot_4d_bind::Transform4D::proj_from_array_origin(const PackedRealArray &p_from_array) {
	return Vector4(p_from_array[16], p_from_array[17], p_from_array[18], p_from_array[19]);
}

PackedRealArray godot_4d_bind::Transform4D::proj_to_array(const Projection &p_from_basis, const Vector4 &p_from_origin) {
	return ::Transform4D(p_from_basis, p_from_origin).to_array();
}

// Bindings.

void godot_4d_bind::Transform4D::_bind_methods() {
	// Instance method bindings.
	// Misc methods.
	ClassDB::bind_method(D_METHOD("determinant"), &godot_4d_bind::Transform4D::determinant);
	ClassDB::bind_method(D_METHOD("lerp", "transform", "weight"), &godot_4d_bind::Transform4D::lerp);
	ClassDB::bind_method(D_METHOD("is_equal_approx", "transform"), &godot_4d_bind::Transform4D::is_equal_approx);
	ClassDB::bind_method(D_METHOD("is_equal_exact", "transform"), &godot_4d_bind::Transform4D::is_equal_exact);
	ClassDB::bind_method(D_METHOD("translate_local", "translation"), &godot_4d_bind::Transform4D::translate_local);
	ClassDB::bind_method(D_METHOD("translated_local", "translation"), &godot_4d_bind::Transform4D::translated_local);
	// Transformation methods.
	ClassDB::bind_method(D_METHOD("compose", "child_transform"), &godot_4d_bind::Transform4D::compose);
	ClassDB::bind_method(D_METHOD("transform_to", "to"), &godot_4d_bind::Transform4D::transform_to);
	ClassDB::bind_method(D_METHOD("xform", "vector"), &godot_4d_bind::Transform4D::xform);
	ClassDB::bind_method(D_METHOD("xform_many", "vectors"), &godot_4d_bind::Transform4D::xform_many);
	ClassDB::bind_method(D_METHOD("xform_basis", "basis"), &godot_4d_bind::Transform4D::xform_basis);
	ClassDB::bind_method(D_METHOD("xform_inv", "vector"), &godot_4d_bind::Transform4D::xform_inv);
	ClassDB::bind_method(D_METHOD("xform_inv_many", "vectors"), &godot_4d_bind::Transform4D::xform_inv_many);
	ClassDB::bind_method(D_METHOD("xform_inv_basis", "basis"), &godot_4d_bind::Transform4D::xform_inv_basis);
	ClassDB::bind_method(D_METHOD("xform_transposed", "vector"), &godot_4d_bind::Transform4D::xform_transposed);
	ClassDB::bind_method(D_METHOD("xform_transposed_many", "vectors"), &godot_4d_bind::Transform4D::xform_transposed_many);
	ClassDB::bind_method(D_METHOD("xform_transposed_basis", "basis"), &godot_4d_bind::Transform4D::xform_transposed_basis);
	// Inversion methods.
	ClassDB::bind_method(D_METHOD("inverse"), &godot_4d_bind::Transform4D::inverse);
	ClassDB::bind_method(D_METHOD("inverse_basis"), &godot_4d_bind::Transform4D::inverse_basis);
	ClassDB::bind_method(D_METHOD("inverse_transposed"), &godot_4d_bind::Transform4D::inverse_transposed);
	// Rotation methods.
	ClassDB::bind_method(D_METHOD("rotate_global", "euler"), &godot_4d_bind::Transform4D::rotate_global);
	ClassDB::bind_method(D_METHOD("rotated_global", "euler"), &godot_4d_bind::Transform4D::rotated_global);
	ClassDB::bind_method(D_METHOD("rotate_local", "euler"), &godot_4d_bind::Transform4D::rotate_local);
	ClassDB::bind_method(D_METHOD("rotated_local", "euler"), &godot_4d_bind::Transform4D::rotated_local);
	ClassDB::bind_method(D_METHOD("get_rotation"), &godot_4d_bind::Transform4D::get_rotation);
	ClassDB::bind_method(D_METHOD("set_rotation", "euler"), &godot_4d_bind::Transform4D::set_rotation);
	ClassDB::bind_method(D_METHOD("get_rotation_degrees"), &godot_4d_bind::Transform4D::get_rotation_degrees);
	ClassDB::bind_method(D_METHOD("set_rotation_degrees", "euler"), &godot_4d_bind::Transform4D::set_rotation_degrees);
	// Geometric algebra rotation altering methods.
	ClassDB::bind_method(D_METHOD("rotate_bivector_magnitude", "bivector"), &godot_4d_bind::Transform4D::rotate_bivector_magnitude);
	ClassDB::bind_method(D_METHOD("rotate_bivector_magnitude_local", "bivector_local"), &godot_4d_bind::Transform4D::rotate_bivector_magnitude_local);
	ClassDB::bind_method(D_METHOD("rotate_rotor", "rotor"), &godot_4d_bind::Transform4D::rotate_rotor);
	ClassDB::bind_method(D_METHOD("rotate_rotor_local", "rotor_local"), &godot_4d_bind::Transform4D::rotate_rotor_local);
	// Geometric algebra rotation properties.
	ClassDB::bind_method(D_METHOD("set_rotation_bivector_magnitude", "bivector"), &godot_4d_bind::Transform4D::set_rotation_bivector_magnitude);
	ClassDB::bind_method(D_METHOD("set_rotation_rotor", "rotor"), &godot_4d_bind::Transform4D::set_rotation_rotor);
	// Scale methods.
	ClassDB::bind_method(D_METHOD("scale_global", "scale"), &godot_4d_bind::Transform4D::scale_global);
	ClassDB::bind_method(D_METHOD("scaled_global", "scale"), &godot_4d_bind::Transform4D::scaled_global);
	ClassDB::bind_method(D_METHOD("scale_local", "scale"), &godot_4d_bind::Transform4D::scale_local);
	ClassDB::bind_method(D_METHOD("scaled_local", "scale"), &godot_4d_bind::Transform4D::scaled_local);
	ClassDB::bind_method(D_METHOD("scale_uniform_global", "scale"), &godot_4d_bind::Transform4D::scale_uniform_global);
	ClassDB::bind_method(D_METHOD("scaled_uniform_global", "scale"), &godot_4d_bind::Transform4D::scaled_uniform_global);
	ClassDB::bind_method(D_METHOD("scale_uniform_local", "scale"), &godot_4d_bind::Transform4D::scale_uniform_local);
	ClassDB::bind_method(D_METHOD("scaled_uniform_local", "scale"), &godot_4d_bind::Transform4D::scaled_uniform_local);
	ClassDB::bind_method(D_METHOD("get_scale"), &godot_4d_bind::Transform4D::get_scale);
	ClassDB::bind_method(D_METHOD("get_scale_abs"), &godot_4d_bind::Transform4D::get_scale_abs);
	ClassDB::bind_method(D_METHOD("set_scale", "scale"), &godot_4d_bind::Transform4D::set_scale);
	ClassDB::bind_method(D_METHOD("get_uniform_scale"), &godot_4d_bind::Transform4D::get_uniform_scale);
	ClassDB::bind_method(D_METHOD("set_uniform_scale", "uniform_scale"), &godot_4d_bind::Transform4D::set_uniform_scale);
	// Validation methods.
	ClassDB::bind_method(D_METHOD("orthonormalize"), &godot_4d_bind::Transform4D::orthonormalize);
	ClassDB::bind_method(D_METHOD("orthonormalized"), &godot_4d_bind::Transform4D::orthonormalized);
	ClassDB::bind_method(D_METHOD("orthogonalize"), &godot_4d_bind::Transform4D::orthogonalize);
	ClassDB::bind_method(D_METHOD("orthogonalized"), &godot_4d_bind::Transform4D::orthogonalized);
	// Helper setters/getters.
	ClassDB::bind_method(D_METHOD("get_basis"), &godot_4d_bind::Transform4D::get_basis);
	ClassDB::bind_method(D_METHOD("set_basis", "basis"), &godot_4d_bind::Transform4D::set_basis);
	ClassDB::bind_method(D_METHOD("get_origin"), &godot_4d_bind::Transform4D::get_origin);
	ClassDB::bind_method(D_METHOD("set_origin", "origin"), &godot_4d_bind::Transform4D::set_origin);
	ADD_PROPERTY(PropertyInfo(Variant::PROJECTION, "basis"), "set_basis", "get_basis");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "origin"), "set_origin", "get_origin");
	// Conversion.
	ClassDB::bind_static_method("Transform4D", D_METHOD("from_3d", "from_3d"), &godot_4d_bind::Transform4D::from_3d);
	ClassDB::bind_method(D_METHOD("to_3d", "orthonormalized"), &godot_4d_bind::Transform4D::to_3d);
	ClassDB::bind_static_method("Transform4D", D_METHOD("from_array", "from_array"), &godot_4d_bind::Transform4D::from_array);
	ClassDB::bind_method(D_METHOD("to_array"), &godot_4d_bind::Transform4D::to_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("from_basis", "basis", "origin"), &godot_4d_bind::Transform4D::from_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("from_numbers", "xx", "xy", "xz", "xw", "yx", "yy", "yz", "yw", "zx", "zy", "zz", "zw", "wx", "wy", "wz", "ww", "ox", "oy", "oz", "ow"), &godot_4d_bind::Transform4D::from_numbers);
	ClassDB::bind_static_method("Transform4D", D_METHOD("from_vectors", "x", "y", "z", "w", "origin"), &godot_4d_bind::Transform4D::from_vectors);
	ClassDB::bind_method(D_METHOD("copy"), &godot_4d_bind::Transform4D::copy);

	// Static method bindings.
	// Misc methods.
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_compose", "parent_array", "child_array"), &godot_4d_bind::Transform4D::array_compose);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_compose_to_array", "parent_basis", "parent_origin", "child_basis", "child_origin"), &godot_4d_bind::Transform4D::proj_compose_to_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_is_equal_approx", "parent_basis", "parent_origin", "child_basis", "child_origin"), &godot_4d_bind::Transform4D::proj_is_equal_approx);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_is_equal_approx", "array_a", "array_b"), &godot_4d_bind::Transform4D::array_is_equal_approx);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_translated_local", "basis", "origin", "translation"), &godot_4d_bind::Transform4D::proj_translated_local);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_translated_local", "array", "translation"), &godot_4d_bind::Transform4D::array_translated_local);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_translated_local_to_array", "basis", "origin", "translation"), &godot_4d_bind::Transform4D::proj_translated_local_to_array);
	// Transformation methods.
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_xform", "parent_basis", "parent_origin", "child_vector"), &godot_4d_bind::Transform4D::proj_xform);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_xform_basis", "parent_basis", "parent_origin", "child_basis"), &godot_4d_bind::Transform4D::proj_xform_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_xform_many", "parent_basis", "parent_origin", "child_vectors"), &godot_4d_bind::Transform4D::proj_xform_many);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_xform", "transform", "child_vector"), &godot_4d_bind::Transform4D::array_xform);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_xform_basis", "transform", "child_basis"), &godot_4d_bind::Transform4D::array_xform_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_xform_many", "transform", "child_vectors"), &godot_4d_bind::Transform4D::array_xform_many);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_xform_inv", "parent_basis", "parent_origin", "vector"), &godot_4d_bind::Transform4D::proj_xform_inv);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_xform_inv_basis", "parent_basis", "parent_origin", "basis"), &godot_4d_bind::Transform4D::proj_xform_inv_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_xform_inv_many", "parent_basis", "parent_origin", "vectors"), &godot_4d_bind::Transform4D::proj_xform_inv_many);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_xform_inv", "transform", "vector"), &godot_4d_bind::Transform4D::array_xform_inv);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_xform_inv_basis", "transform", "basis"), &godot_4d_bind::Transform4D::array_xform_inv_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_xform_inv_many", "transform", "vectors"), &godot_4d_bind::Transform4D::array_xform_inv_many);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_xform_transposed", "parent_basis", "parent_origin", "vector"), &godot_4d_bind::Transform4D::proj_xform_transposed);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_xform_transposed_basis", "parent_basis", "parent_origin", "basis"), &godot_4d_bind::Transform4D::proj_xform_transposed_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_xform_transposed_many", "parent_basis", "parent_origin", "vectors"), &godot_4d_bind::Transform4D::proj_xform_transposed_many);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_xform_transposed", "transform", "vector"), &godot_4d_bind::Transform4D::array_xform_transposed);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_xform_transposed_basis", "transform", "basis"), &godot_4d_bind::Transform4D::array_xform_transposed_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_xform_transposed_many", "transform", "vectors"), &godot_4d_bind::Transform4D::array_xform_transposed_many);
	// Inversion methods.
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_inverse", "transform"), &godot_4d_bind::Transform4D::array_inverse);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_inverse_to_array", "basis", "origin"), &godot_4d_bind::Transform4D::proj_inverse_to_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_inverse_origin_with_inverse_basis", "inverse_basis", "origin"), &godot_4d_bind::Transform4D::proj_inverse_origin_with_inverse_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_inverse_origin_with_regular_basis", "basis", "origin"), &godot_4d_bind::Transform4D::proj_inverse_origin_with_regular_basis);
	// Conversion.
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_from_3d_basis", "from_3d"), &godot_4d_bind::Transform4D::proj_from_3d_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_from_3d_origin", "from_3d"), &godot_4d_bind::Transform4D::proj_from_3d_origin);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_from_3d", "from_3d"), &godot_4d_bind::Transform4D::array_from_3d);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_to_3d", "from_basis", "from_origin", "orthonormalized"), &godot_4d_bind::Transform4D::proj_to_3d);
	ClassDB::bind_static_method("Transform4D", D_METHOD("array_to_3d", "from_array", "orthonormalized"), &godot_4d_bind::Transform4D::array_to_3d);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_from_array_basis", "from_array"), &godot_4d_bind::Transform4D::proj_from_array_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_from_array_origin", "from_array"), &godot_4d_bind::Transform4D::proj_from_array_origin);
	ClassDB::bind_static_method("Transform4D", D_METHOD("proj_to_array", "from_basis", "from_origin"), &godot_4d_bind::Transform4D::proj_to_array);
}
