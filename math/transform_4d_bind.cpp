#include "transform_4d_bind.h"

#include "transform_4d.h"

// Misc methods.

PackedRealArray godot_4d_bind::Transform4D::compose_array(const PackedRealArray &p_parent_array, const PackedRealArray &p_child_array) {
	return ::Transform4D::from_array(p_parent_array).compose(::Transform4D::from_array(p_child_array)).to_array();
}

PackedRealArray godot_4d_bind::Transform4D::compose_to_array(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_child_basis, const Vector4 &p_child_origin) {
	return ::Transform4D(p_parent_basis, p_parent_origin).compose(::Transform4D(p_child_basis, p_child_origin)).to_array();
}

bool godot_4d_bind::Transform4D::is_equal_approx(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_child_basis, const Vector4 &p_child_origin) {
	return ::Transform4D(p_parent_basis, p_parent_origin).is_equal_approx(::Transform4D(p_child_basis, p_child_origin));
}

bool godot_4d_bind::Transform4D::is_equal_approx_array(const PackedRealArray &p_array_a, const PackedRealArray &p_array_b) {
	return ::Transform4D::from_array(p_array_a).is_equal_approx(::Transform4D::from_array(p_array_b));
}

Vector4 godot_4d_bind::Transform4D::translated_local(const Projection &p_basis, const Vector4 &p_origin, const Vector4 &p_translation) {
	return ::Transform4D(::Basis4D(p_basis), p_origin).translated_local(p_translation).origin;
}

PackedRealArray godot_4d_bind::Transform4D::translated_local_array(const PackedRealArray &p_array, const Vector4 &p_translation) {
	return ::Transform4D::from_array(p_array).translated_local(p_translation).to_array();
}

PackedRealArray godot_4d_bind::Transform4D::translated_local_to_array(const Projection &p_basis, const Vector4 &p_origin, const Vector4 &p_translation) {
	return ::Transform4D(::Basis4D(p_basis), p_origin).translated_local(p_translation).to_array();
}

// Transformation methods.

Vector4 godot_4d_bind::Transform4D::xform(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_child_vector) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform(p_child_vector);
}

Projection godot_4d_bind::Transform4D::xform_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_child_basis) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_basis(Basis4D(p_child_basis));
}

Vector4 godot_4d_bind::Transform4D::xform_array(const PackedRealArray &p_transform, const Vector4 &p_child_vector) {
	return ::Transform4D::from_array(p_transform).xform(p_child_vector);
}

Projection godot_4d_bind::Transform4D::xform_basis_array(const PackedRealArray &p_transform, const Projection &p_child_basis) {
	return ::Transform4D::from_array(p_transform).xform_basis(Basis4D(p_child_basis));
}

Vector4 godot_4d_bind::Transform4D::xform_inv(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_vector) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_inv(p_vector);
}

Projection godot_4d_bind::Transform4D::xform_inv_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_basis) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_inv_basis(Basis4D(p_basis));
}

Vector4 godot_4d_bind::Transform4D::xform_inv_array(const PackedRealArray &p_transform, const Vector4 &p_vector) {
	return ::Transform4D::from_array(p_transform).xform_inv(p_vector);
}

Projection godot_4d_bind::Transform4D::xform_inv_basis_array(const PackedRealArray &p_transform, const Projection &p_basis) {
	return ::Transform4D::from_array(p_transform).xform_inv_basis(Basis4D(p_basis));
}

Vector4 godot_4d_bind::Transform4D::xform_transposed(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_vector) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_transposed(p_vector);
}

Projection godot_4d_bind::Transform4D::xform_transposed_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_basis) {
	return ::Transform4D(::Basis4D(p_parent_basis), p_parent_origin).xform_transposed_basis(Basis4D(p_basis));
}

Vector4 godot_4d_bind::Transform4D::xform_transposed_array(const PackedRealArray &p_transform, const Vector4 &p_vector) {
	return ::Transform4D::from_array(p_transform).xform_transposed(p_vector);
}

Projection godot_4d_bind::Transform4D::xform_transposed_basis_array(const PackedRealArray &p_transform, const Projection &p_basis) {
	return ::Transform4D::from_array(p_transform).xform_transposed_basis(Basis4D(p_basis));
}

// Inversion methods.

PackedRealArray godot_4d_bind::Transform4D::inverse_array(const PackedRealArray &p_transform) {
	return ::Transform4D::from_array(p_transform).inverse().to_array();
}

PackedRealArray godot_4d_bind::Transform4D::inverse_to_array(const Projection &p_basis, const Vector4 &p_origin) {
	return ::Transform4D(p_basis, p_origin).inverse().to_array();
}

Vector4 godot_4d_bind::Transform4D::inverse_origin_with_inverse_basis(const Projection &p_inverse_basis, const Vector4 &p_origin) {
	return ::Basis4D(p_inverse_basis).xform(-p_origin);
}

Vector4 godot_4d_bind::Transform4D::inverse_origin_with_regular_basis(const Projection &p_basis, const Vector4 &p_origin) {
	return ::Basis4D(p_basis).inverse().xform(-p_origin);
}

// Conversion.

Projection godot_4d_bind::Transform4D::from_3d_basis(const Transform3D &p_from_3d) {
	return ::Basis4D::from_3d(p_from_3d.basis);
}

Vector4 godot_4d_bind::Transform4D::from_3d_origin(const Transform3D &p_from_3d) {
	return Vector4(p_from_3d.origin.x, p_from_3d.origin.y, p_from_3d.origin.z, 0.0);
}

PackedRealArray godot_4d_bind::Transform4D::from_3d_array(const Transform3D &p_from_3d) {
	return ::Transform4D::from_3d(p_from_3d).to_array();
}

Transform3D godot_4d_bind::Transform4D::to_3d(const Projection &p_from_basis, const Vector4 &p_from_origin, const bool p_orthonormalized) {
	return ::Transform4D(::Basis4D(p_from_basis), p_from_origin).to_3d(p_orthonormalized);
}

Transform3D godot_4d_bind::Transform4D::to_3d_array(const PackedRealArray &p_from_array, const bool p_orthonormalized) {
	return ::Transform4D::from_array(p_from_array).to_3d(p_orthonormalized);
}

Projection godot_4d_bind::Transform4D::from_array_basis(const PackedRealArray &p_from_array) {
	return ::Basis4D::from_array(p_from_array);
}

Vector4 godot_4d_bind::Transform4D::from_array_origin(const PackedRealArray &p_from_array) {
	return Vector4(p_from_array[16], p_from_array[17], p_from_array[18], p_from_array[19]);
}

PackedRealArray godot_4d_bind::Transform4D::to_array(const Projection &p_from_basis, const Vector4 &p_from_origin) {
	return ::Transform4D(p_from_basis, p_from_origin).to_array();
}

// Bindings.

godot_4d_bind::Transform4D *godot_4d_bind::Transform4D::singleton = nullptr;

void godot_4d_bind::Transform4D::_bind_methods() {
	// Misc methods.
	ClassDB::bind_static_method("Transform4D", D_METHOD("compose_array", "parent_array", "child_array"), &godot_4d_bind::Transform4D::compose_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("compose_to_array", "parent_basis", "parent_origin", "child_basis", "child_origin"), &godot_4d_bind::Transform4D::compose_to_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("is_equal_approx", "parent_basis", "parent_origin", "child_basis", "child_origin"), &godot_4d_bind::Transform4D::is_equal_approx);
	ClassDB::bind_static_method("Transform4D", D_METHOD("is_equal_approx_array", "array_a", "array_b"), &godot_4d_bind::Transform4D::is_equal_approx_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("translated_local", "basis", "origin", "translation"), &godot_4d_bind::Transform4D::translated_local);
	ClassDB::bind_static_method("Transform4D", D_METHOD("translated_local_array", "array", "translation"), &godot_4d_bind::Transform4D::translated_local_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("translated_local_to_array", "basis", "origin", "translation"), &godot_4d_bind::Transform4D::translated_local_to_array);
	// Transformation methods.
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform", "parent_basis", "parent_origin", "child_vector"), &godot_4d_bind::Transform4D::xform);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_basis", "parent_basis", "parent_origin", "child_basis"), &godot_4d_bind::Transform4D::xform_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_array", "transform", "child_vector"), &godot_4d_bind::Transform4D::xform_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_basis_array", "transform", "child_basis"), &godot_4d_bind::Transform4D::xform_basis_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_inv", "parent_basis", "parent_origin", "vector"), &godot_4d_bind::Transform4D::xform_inv);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_inv_basis", "parent_basis", "parent_origin", "basis"), &godot_4d_bind::Transform4D::xform_inv_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_inv_array", "transform", "vector"), &godot_4d_bind::Transform4D::xform_inv_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_inv_basis_array", "transform", "basis"), &godot_4d_bind::Transform4D::xform_inv_basis_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_transposed", "parent_basis", "parent_origin", "vector"), &godot_4d_bind::Transform4D::xform_transposed);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_transposed_basis", "parent_basis", "parent_origin", "basis"), &godot_4d_bind::Transform4D::xform_transposed_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_transposed_array", "transform", "vector"), &godot_4d_bind::Transform4D::xform_transposed_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("xform_transposed_basis_array", "transform", "basis"), &godot_4d_bind::Transform4D::xform_transposed_basis_array);
	// Inversion methods.
	ClassDB::bind_static_method("Transform4D", D_METHOD("inverse_array", "transform"), &godot_4d_bind::Transform4D::inverse_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("inverse_to_array", "basis", "origin"), &godot_4d_bind::Transform4D::inverse_to_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("inverse_origin_with_inverse_basis", "inverse_basis", "origin"), &godot_4d_bind::Transform4D::inverse_origin_with_inverse_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("inverse_origin_with_regular_basis", "basis", "origin"), &godot_4d_bind::Transform4D::inverse_origin_with_regular_basis);
	// Conversion.
	ClassDB::bind_static_method("Transform4D", D_METHOD("from_3d_basis", "from_3d"), &godot_4d_bind::Transform4D::from_3d_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("from_3d_origin", "from_3d"), &godot_4d_bind::Transform4D::from_3d_origin);
	ClassDB::bind_static_method("Transform4D", D_METHOD("from_3d_array", "from_3d"), &godot_4d_bind::Transform4D::from_3d_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("to_3d", "from_basis", "from_origin", "orthonormalized"), &godot_4d_bind::Transform4D::to_3d);
	ClassDB::bind_static_method("Transform4D", D_METHOD("to_3d_array", "from_array", "orthonormalized"), &godot_4d_bind::Transform4D::to_3d_array);
	ClassDB::bind_static_method("Transform4D", D_METHOD("from_array_basis", "from_array"), &godot_4d_bind::Transform4D::from_array_basis);
	ClassDB::bind_static_method("Transform4D", D_METHOD("from_array_origin", "from_array"), &godot_4d_bind::Transform4D::from_array_origin);
	ClassDB::bind_static_method("Transform4D", D_METHOD("to_array", "from_basis", "from_origin"), &godot_4d_bind::Transform4D::to_array);
}
