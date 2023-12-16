#pragma once

#include "../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/variant/projection.hpp>
#elif GODOT_MODULE
#include "core/math/projection.h"
#endif

namespace godot_4d_bind {
// Stateless helper class to bind methods for Transform4D.
class Transform4D : public Object {
	GDCLASS(Transform4D, Object);

protected:
	static Transform4D *singleton;
	static void _bind_methods();

public:
	// Misc methods.
	static PackedRealArray compose_array(const PackedRealArray &p_parent_array, const PackedRealArray &p_child_array);
	static PackedRealArray compose_to_array(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_child_basis, const Vector4 &p_child_origin);
	static bool is_equal_approx(const Projection &p_basis_a, const Vector4 &p_origin_a, const Projection &p_basis_b, const Vector4 &p_origin_b);
	static bool is_equal_approx_array(const PackedRealArray &p_array_a, const PackedRealArray &p_array_b);
	static Vector4 translated_local(const Projection &p_basis, const Vector4 &p_origin, const Vector4 &p_translation);
	static PackedRealArray translated_local_array(const PackedRealArray &p_array, const Vector4 &p_translation);
	static PackedRealArray translated_local_to_array(const Projection &p_basis, const Vector4 &p_origin, const Vector4 &p_translation);

	// Transformation methods.
	static Vector4 xform(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_child_vector);
	static Projection xform_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_child_basis);
	static Vector4 xform_array(const PackedRealArray &p_transform, const Vector4 &p_child_vector);
	static Projection xform_basis_array(const PackedRealArray &p_transform, const Projection &p_child_basis);

	static Vector4 xform_inv(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_vector);
	static Projection xform_inv_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_basis);
	static Vector4 xform_inv_array(const PackedRealArray &p_transform, const Vector4 &p_vector);
	static Projection xform_inv_basis_array(const PackedRealArray &p_transform, const Projection &p_basis);

	static Vector4 xform_transposed(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Vector4 &p_vector);
	static Projection xform_transposed_basis(const Projection &p_parent_basis, const Vector4 &p_parent_origin, const Projection &p_basis);
	static Vector4 xform_transposed_array(const PackedRealArray &p_transform, const Vector4 &p_vector);
	static Projection xform_transposed_basis_array(const PackedRealArray &p_transform, const Projection &p_basis);

	// Inversion methods.
	static PackedRealArray inverse_array(const PackedRealArray &p_transform);
	static PackedRealArray inverse_to_array(const Projection &p_basis, const Vector4 &p_origin);
	static Vector4 inverse_origin_with_inverse_basis(const Projection &p_inverse_basis, const Vector4 &p_origin);
	static Vector4 inverse_origin_with_regular_basis(const Projection &p_basis, const Vector4 &p_origin);

	// Conversion.
	static Projection from_3d_basis(const Transform3D &p_from_3d);
	static Vector4 from_3d_origin(const Transform3D &p_from_3d);
	static PackedRealArray from_3d_array(const Transform3D &p_from_3d);
	static Transform3D to_3d(const Projection &p_from_basis, const Vector4 &p_from_origin, const bool p_orthonormalized = false);
	static Transform3D to_3d_array(const PackedRealArray &p_from_array, const bool p_orthonormalized = false);
	static Projection from_array_basis(const PackedRealArray &p_from_array);
	static Vector4 from_array_origin(const PackedRealArray &p_from_array);
	static PackedRealArray to_array(const Projection &p_from_basis, const Vector4 &p_from_origin);

	Transform4D() { singleton = this; }
	~Transform4D() { singleton = nullptr; }
};
} // namespace godot_4d_bind
