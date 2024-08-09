#pragma once

#include "transform_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/projection.hpp>
#elif GODOT_MODULE
#include "core/math/projection.h"
#include "core/object/ref_counted.h"
#endif

namespace godot_4d_bind {
class Euler4D;
// Helper class to bind Transform4D to GDScript via RefCounted.
class Transform4D : public RefCounted {
	GDCLASS(Transform4D, RefCounted);

	::Transform4D transform;

protected:
	static void _bind_methods();

public:
	// Instance methods, providing a friendly readable Transform4D object, which unfortunately uses lots of memory and is passed by reference.
	// Misc methods.
	real_t determinant() const;
	Ref<Transform4D> lerp(const Ref<Transform4D> &p_transform, real_t p_weight) const;
	bool is_equal_approx(const Ref<Transform4D> &p_transform) const;
	bool is_equal_exact(const Ref<Transform4D> &p_transform) const;
	void translate_local(const Vector4 &p_translation);
	Ref<Transform4D> translated_local(const Vector4 &p_translation) const;

	// Transformation methods.
	Ref<Transform4D> compose(const Ref<Transform4D> &p_child_transform) const;
	Vector4 xform(const Vector4 &p_vector) const;
	PackedVector4Array xform_many(const PackedVector4Array &p_vectors) const;
	Projection xform_basis(const Projection &p_basis) const;

	Vector4 xform_inv(const Vector4 &p_vector) const;
	PackedVector4Array xform_inv_many(const PackedVector4Array &p_vectors) const;
	Projection xform_inv_basis(const Projection &p_basis) const;

	Vector4 xform_transposed(const Vector4 &p_vector) const;
	PackedVector4Array xform_transposed_many(const PackedVector4Array &p_vectors) const;
	Projection xform_transposed_basis(const Projection &p_basis) const;

	// Inversion methods.
	Ref<Transform4D> inverse() const;
	Ref<Transform4D> inverse_basis() const;
	Ref<Transform4D> inverse_transposed() const;

	// Rotation methods.
	void rotate_global(const Ref<godot_4d_bind::Euler4D> &p_euler);
	Ref<Transform4D> rotated_global(const Ref<godot_4d_bind::Euler4D> &p_euler) const;
	void rotate_local(const Ref<godot_4d_bind::Euler4D> &p_euler);
	Ref<Transform4D> rotated_local(const Ref<godot_4d_bind::Euler4D> &p_euler) const;
	Ref<godot_4d_bind::Euler4D> get_rotation() const;
	void set_rotation(const Ref<godot_4d_bind::Euler4D> &p_euler);
	Ref<godot_4d_bind::Euler4D> get_rotation_degrees() const;
	void set_rotation_degrees(const Ref<godot_4d_bind::Euler4D> &p_euler);

	// Scale methods.
	void scale_global(const Vector4 &p_scale);
	Ref<Transform4D> scaled_global(const Vector4 &p_scale) const;
	void scale_local(const Vector4 &p_scale);
	Ref<Transform4D> scaled_local(const Vector4 &p_scale) const;
	void scale_uniform_global(const real_t p_scale);
	Ref<Transform4D> scaled_uniform_global(const real_t p_scale) const;
	void scale_uniform_local(const real_t p_scale);
	Ref<Transform4D> scaled_uniform_local(const real_t p_scale) const;
	Vector4 get_scale() const;
	Vector4 get_scale_abs() const;
	void set_scale(const Vector4 &p_scale);
	real_t get_uniform_scale() const;
	void set_uniform_scale(const real_t p_uniform_scale);

	// Validation methods.
	void orthonormalize();
	Ref<Transform4D> orthonormalized() const;
	void orthogonalize();
	Ref<Transform4D> orthogonalized() const;

	// Helper setters/getters.
	::Transform4D get_transform() const { return transform; }
	void set_transform(const ::Transform4D &p_transform) { transform = p_transform; }

	Projection get_basis() const { return transform.basis; }
	void set_basis(const Projection &p_basis) { transform.basis = p_basis; }

	Vector4 get_origin() const { return transform.origin; }
	void set_origin(const Vector4 &p_origin) { transform.origin = p_origin; }

	// Conversion.
	static Ref<Transform4D> from_3d(const Transform3D &p_from_3d);
	Transform3D to_3d(const bool p_orthonormalized = false) const;
	static Ref<Transform4D> from_array(const PackedRealArray &p_array);
	PackedRealArray to_array() const;
	static Ref<Transform4D> from_basis(const Projection &p_basis, const Vector4 &p_origin = Vector4());
	static Ref<Transform4D> from_numbers(const real_t p_xx, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yx, const real_t p_yy, const real_t p_yz, const real_t p_yw, const real_t p_zx, const real_t p_zy, const real_t p_zz, const real_t p_zw, const real_t p_wx, const real_t p_wy, const real_t p_wz, const real_t p_ww, const real_t p_ox = 0.0f, const real_t p_oy = 0.0f, const real_t p_oz = 0.0f, const real_t p_ow = 0.0f);
	static Ref<Transform4D> from_vectors(const Vector4 &p_x, const Vector4 &p_y, const Vector4 &p_z, const Vector4 &p_w, const Vector4 &p_origin = Vector4());
	Ref<Transform4D> copy() const;

#if GDEXTENSION
	String _to_string() const;
#elif GODOT_MODULE
	virtual String to_string() override;
#endif

	// Constructors.
	Transform4D() {}
	Transform4D(const ::Basis4D &p_basis, const Vector4 &p_origin = Vector4()) { transform = ::Transform4D(p_basis, p_origin); }
	Transform4D(const Projection &p_basis, const Vector4 &p_origin = Vector4()) { transform = ::Transform4D(p_basis, p_origin); }
	Transform4D(const Vector4 &p_x, const Vector4 &p_y, const Vector4 &p_z, const Vector4 &p_w, const Vector4 &p_origin = Vector4()) {
		transform = ::Transform4D(p_x, p_y, p_z, p_w, p_origin);
	}
	Transform4D(const real_t p_xx, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yx, const real_t p_yy, const real_t p_yz, const real_t p_yw, const real_t p_zx, const real_t p_zy, const real_t p_zz, const real_t p_zw, const real_t p_wx, const real_t p_wy, const real_t p_wz, const real_t p_ww, const real_t p_ox = 0.0f, const real_t p_oy = 0.0f, const real_t p_oz = 0.0f, const real_t p_ow = 0.0f) {
		transform = ::Transform4D(p_xx, p_xy, p_xz, p_xw, p_yx, p_yy, p_yz, p_yw, p_zx, p_zy, p_zz, p_zw, p_wx, p_wy, p_wz, p_ww, p_ox, p_oy, p_oz, p_ow);
	}

	// Static methods, allowing the user to use PackedRealArray or Projection+Vector4 to represent a Transform4D, which saves memory and the latter is passed by value.
	// Misc methods.
	static PackedRealArray array_compose(const PackedRealArray &p_parent_array, const PackedRealArray &p_child_array);
	static PackedRealArray proj_compose_to_array(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_child_basis, const Vector4 &p_child_origin);
	static bool proj_is_equal_approx(const Projection &p_basis_a, const Vector4 &p_origin_a, const Projection &p_basis_b, const Vector4 &p_origin_b);
	static bool array_is_equal_approx(const PackedRealArray &p_array_a, const PackedRealArray &p_array_b);
	static Vector4 proj_translated_local(const Projection &p_basis, const Vector4 &p_origin, const Vector4 &p_translation);
	static PackedRealArray array_translated_local(const PackedRealArray &p_array, const Vector4 &p_translation);
	static PackedRealArray proj_translated_local_to_array(const Projection &p_basis, const Vector4 &p_origin, const Vector4 &p_translation);

	// Transformation methods.
	static Vector4 proj_xform(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_child_vector);
	static Projection proj_xform_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_child_basis);
	static Vector4 array_xform(const PackedRealArray &p_transform, const Vector4 &p_child_vector);
	static Projection array_xform_basis(const PackedRealArray &p_transform, const Projection &p_child_basis);

	static Vector4 proj_xform_inv(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_vector);
	static Projection proj_xform_inv_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_basis);
	static Vector4 array_xform_inv(const PackedRealArray &p_transform, const Vector4 &p_vector);
	static Projection array_xform_inv_basis(const PackedRealArray &p_transform, const Projection &p_basis);

	static Vector4 proj_xform_transposed(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_vector);
	static Projection proj_xform_transposed_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_basis);
	static Vector4 array_xform_transposed(const PackedRealArray &p_transform, const Vector4 &p_vector);
	static Projection array_xform_transposed_basis(const PackedRealArray &p_transform, const Projection &p_basis);

	// Inversion methods.
	static PackedRealArray array_inverse(const PackedRealArray &p_transform);
	static PackedRealArray proj_inverse_to_array(const Projection &p_basis, const Vector4 &p_origin);
	static Vector4 proj_inverse_origin_with_inverse_basis(const Projection &p_inverse_basis, const Vector4 &p_origin);
	static Vector4 proj_inverse_origin_with_regular_basis(const Projection &p_basis, const Vector4 &p_origin);

	// Conversion.
	static Projection proj_from_3d_basis(const Transform3D &p_from_3d);
	static Vector4 proj_from_3d_origin(const Transform3D &p_from_3d);
	static PackedRealArray array_from_3d(const Transform3D &p_from_3d);
	static Transform3D proj_to_3d(const Projection &p_from_basis, const Vector4 &p_from_origin, const bool p_orthonormalized = false);
	static Transform3D array_to_3d(const PackedRealArray &p_from_array, const bool p_orthonormalized = false);
	static Projection proj_from_array_basis(const PackedRealArray &p_from_array);
	static Vector4 proj_from_array_origin(const PackedRealArray &p_from_array);
	static PackedRealArray proj_to_array(const Projection &p_from_basis, const Vector4 &p_from_origin);
};
} // namespace godot_4d_bind
