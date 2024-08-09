#pragma once

#include "euler_4d.h"

#if GDEXTENSION
#include <godot_cpp/templates/vector.hpp>
#elif GODOT_MODULE
#include "core/templates/vector.h"
#endif

struct _NO_DISCARD_ Transform4D {
	Basis4D basis;
	Vector4 origin;

	// Misc methods.
	real_t determinant() const;
	Transform4D lerp(const Transform4D &p_transform, real_t p_weight) const;
	bool is_equal_approx(const Transform4D &p_transform) const;
	void translate_local(const Vector4 &p_translation);
	Transform4D translated_local(const Vector4 &p_translation) const;

	// Transformation methods.
	Transform4D compose(const Transform4D &p_child_transform) const;
	Vector4 xform(const Vector4 &p_vector) const;
	PackedVector4Array xform_many(const PackedVector4Array &p_vectors) const;
	Basis4D xform_basis(const Basis4D &p_basis) const;

	Vector4 xform_inv(const Vector4 &p_vector) const;
	PackedVector4Array xform_inv_many(const PackedVector4Array &p_vectors) const;
	Basis4D xform_inv_basis(const Basis4D &p_basis) const;

	Vector4 xform_transposed(const Vector4 &p_vector) const;
	PackedVector4Array xform_transposed_many(const PackedVector4Array &p_vectors) const;
	Basis4D xform_transposed_basis(const Basis4D &p_basis) const;

	// Inversion methods.
	Transform4D inverse() const;
	Transform4D inverse_basis() const;
	Transform4D inverse_transposed() const;

	// Rotation methods.
	void rotate_global(const Euler4D &p_euler);
	Transform4D rotated_global(const Euler4D &p_euler) const;
	void rotate_local(const Euler4D &p_euler);
	Transform4D rotated_local(const Euler4D &p_euler) const;
	Euler4D get_rotation() const;
	void set_rotation(const Euler4D &p_euler);
	Euler4D get_rotation_degrees() const;
	void set_rotation_degrees(const Euler4D &p_euler);

	// Scale methods.
	void scale_global(const Vector4 &p_scale);
	Transform4D scaled_global(const Vector4 &p_scale) const;
	void scale_local(const Vector4 &p_scale);
	Transform4D scaled_local(const Vector4 &p_scale) const;
	void scale_uniform_global(const real_t p_scale);
	Transform4D scaled_uniform_global(const real_t p_scale) const;
	void scale_uniform_local(const real_t p_scale);
	Transform4D scaled_uniform_local(const real_t p_scale) const;
	Vector4 get_scale() const;
	Vector4 get_scale_abs() const;
	void set_scale(const Vector4 &p_scale);
	real_t get_uniform_scale() const;
	void set_uniform_scale(const real_t p_uniform_scale);

	// Validation methods.
	void orthonormalize();
	Transform4D orthonormalized() const;
	void orthogonalize();
	Transform4D orthogonalized() const;

	// Helper setters/getters.
	const Basis4D &get_basis() const { return basis; }
	void set_basis(const Basis4D &p_basis) { basis = p_basis; }

	const Vector4 &get_origin() const { return origin; }
	void set_origin(const Vector4 &p_origin) { origin = p_origin; }

	// Operators.
	_FORCE_INLINE_ const Vector4 &operator[](int p_axis) const {
		DEV_ASSERT((unsigned int)p_axis < 5);
		if (p_axis == 4) {
			return origin;
		}
		return basis[p_axis];
	}

	_FORCE_INLINE_ Vector4 &operator[](int p_axis) {
		DEV_ASSERT((unsigned int)p_axis < 5);
		if (p_axis == 4) {
			return origin;
		}
		return basis[p_axis];
	}

	bool operator==(const Transform4D &p_transform) const;
	bool operator!=(const Transform4D &p_transform) const;
	void operator*=(const Transform4D &p_transform);
	Transform4D operator*(const Transform4D &p_transform) const;
	Basis4D operator*(const Basis4D &p_basis) const;
	Vector4 operator*(const Vector4 &p_vector) const;
	void operator*=(const real_t p_val);
	Transform4D operator*(const real_t p_val) const;
	operator String() const;

	// Conversion.
	static Transform4D from_3d(const Transform3D &p_from_3d);
	Transform3D to_3d(const bool p_orthonormalized = false) const;
	static Transform4D from_array(const PackedRealArray &p_array);
	PackedRealArray to_array() const;

	// Constructors.
	Transform4D() {}
	Transform4D(const Basis4D &p_basis, const Vector4 &p_origin = Vector4()) :
			basis(p_basis), origin(p_origin) {}
	Transform4D(const Projection &p_basis, const Vector4 &p_origin = Vector4()) :
			basis(p_basis), origin(p_origin) {}
	Transform4D(const Vector4 &p_x, const Vector4 &p_y, const Vector4 &p_z, const Vector4 &p_w, const Vector4 &p_origin = Vector4()) {
		basis = Basis4D(p_x, p_y, p_z, p_w);
		origin = p_origin;
	}
	Transform4D(const real_t p_xx, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yx, const real_t p_yy, const real_t p_yz, const real_t p_yw, const real_t p_zx, const real_t p_zy, const real_t p_zz, const real_t p_zw, const real_t p_wx, const real_t p_wy, const real_t p_wz, const real_t p_ww, const real_t p_ox = 0.0f, const real_t p_oy = 0.0f, const real_t p_oz = 0.0f, const real_t p_ow = 0.0f) {
		basis = Basis4D(p_xx, p_xy, p_xz, p_xw, p_yx, p_yy, p_yz, p_yw, p_zx, p_zy, p_zz, p_zw, p_wx, p_wy, p_wz, p_ww);
		origin = Vector4(p_ox, p_oy, p_oz, p_ow);
	}
};
