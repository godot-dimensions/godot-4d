#pragma once

#include "../math/euler_4d_bind.h"
#include "../math/geometric_algebra/rotor_4d_bind.h"
#include "../math/rect4.h"
#include "../math/transform_4d_bind.h"

#if GDEXTENSION
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#elif GODOT_MODULE
#include "scene/main/node.h"
#endif

#ifdef TOOLS_ENABLED
#define CACHE_ROTATION_AND_SCALE
#endif // TOOLS_ENABLED

class Node4D : public Node {
	GDCLASS(Node4D, Node);

public:
	enum RotationEditMode {
		ROTATION_EDIT_MODE_EULER4D,
		ROTATION_EDIT_MODE_EULER4D_UNIFORM,
		ROTATION_EDIT_MODE_BASIS4D,
		ROTATION_EDIT_MODE_ROTOR4D,
		ROTATION_EDIT_MODE_ROTOR4D_UNIFORM,
		ROTATION_EDIT_MODE_BIVECTOR4D,
		ROTATION_EDIT_MODE_BIVECTOR4D_UNIFORM,
	};

private:
	// Transform private fields.
	Transform4D _transform;
	RotationEditMode _rotation_edit_mode = ROTATION_EDIT_MODE_EULER4D;
#ifdef CACHE_ROTATION_AND_SCALE
	Euler4D _euler_cache;
	Vector4 _scale_cache;
	bool _euler_cache_dirty = true;
	bool _scale_cache_dirty = true;
#endif // CACHE_ROTATION_AND_SCALE

	bool _is_visible = true;

	void _propagate_visibility_changed();

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	RotationEditMode get_rotation_edit_mode() const;
	void set_rotation_edit_mode(const RotationEditMode p_rotation_edit_mode);

	// Transform altering methods.
	void apply_scale(const Vector4 &p_amount);
	void translate_local(const Vector4 &p_amount);
	void look_at(const Vector4 &p_global_target, const Vector4 &p_up = Vector4(0, 1, 0, 0), const bool p_use_model_front = false);

	void rotate_euler(const Euler4D &p_euler);
	void rotate_euler_bind(const AABB &p_euler);
	void rotate_euler_local(const Euler4D &p_euler_local);
	void rotate_euler_local_bind(const AABB &p_euler_local);

	// Geometric algebra rotation altering methods.
	void rotate_bivector_magnitude(const Bivector4D &p_bivector);
	void rotate_bivector_magnitude_bind(const AABB &p_bivector);
	void rotate_bivector_magnitude_local(const Bivector4D &p_bivector_local);
	void rotate_bivector_magnitude_local_bind(const AABB &p_bivector_local);

	void rotate_rotor(const Rotor4D &p_rotor);
	void rotate_rotor_bind(const Ref<godot_4d_bind::Rotor4D> &p_rotor);
	void rotate_rotor_local(const Rotor4D &p_rotor_local);
	void rotate_rotor_local_bind(const Ref<godot_4d_bind::Rotor4D> &p_rotor_local);

	// Local transform and basis.
	Transform4D get_transform() const;
	void set_transform(const Transform4D &p_transform);
	PackedRealArray get_transform_array() const;
	void set_transform_array(const PackedRealArray &p_transform_array);
	Ref<godot_4d_bind::Transform4D> get_transform_bind() const;
	void set_transform_bind(const Ref<godot_4d_bind::Transform4D> &p_transform);
	Basis4D get_basis() const;
	void set_basis(const Basis4D &p_basis);
	Projection get_basis_bind() const;
	void set_basis_bind(const Projection &p_basis);

	// Local transform components.
	Vector4 get_position() const;
	void set_position(const Vector4 &p_position);
#ifdef CACHE_ROTATION_AND_SCALE
	Euler4D get_rotation();
	AABB get_rotation_bind();
	Ref<godot_4d_bind::Euler4D> get_rotation_euler_bind();
	Euler4D get_rotation_degrees();
	AABB get_rotation_degrees_bind();
	Ref<godot_4d_bind::Euler4D> get_rotation_degrees_euler_bind();
	Vector4 get_scale();
#else
	Euler4D get_rotation() const;
	AABB get_rotation_bind() const;
	Ref<godot_4d_bind::Euler4D> get_rotation_euler_bind() const;
	Euler4D get_rotation_degrees() const;
	AABB get_rotation_degrees_bind() const;
	Ref<godot_4d_bind::Euler4D> get_rotation_degrees_euler_bind() const;
	Vector4 get_scale() const;
#endif // CACHE_ROTATION_AND_SCALE
	real_t get_uniform_scale() const;
	void set_rotation(const Euler4D &p_euler);
	void set_rotation_bind(const AABB &p_euler);
	void set_rotation_euler_bind(const Ref<godot_4d_bind::Euler4D> &p_euler);
	void set_rotation_degrees(const Euler4D &p_euler);
	void set_rotation_degrees_bind(const AABB &p_euler);
	void set_rotation_degrees_euler_bind(const Ref<godot_4d_bind::Euler4D> &p_euler);
	void set_scale(const Vector4 &p_scale);
	void set_uniform_scale(const real_t p_scale);

	// Geometric algebra local rotation properties.
	// Bivector4D get_rotation_bivector_magnitude() const;
	void set_rotation_bivector_magnitude(const Bivector4D &p_bivector);
	// AABB get_rotation_bivector_magnitude_bind() const;
	void set_rotation_bivector_magnitude_bind(const AABB &p_bivector);
	Rotor4D get_rotation_rotor() const;
	void set_rotation_rotor(const Rotor4D &p_rotor);
	Ref<godot_4d_bind::Rotor4D> get_rotation_rotor_bind() const;
	void set_rotation_rotor_bind(const Ref<godot_4d_bind::Rotor4D> &p_rotor);
	PackedRealArray get_rotation_rotor_array() const;
	void set_rotation_rotor_array(const PackedRealArray &p_rotor_array);
	Bivector4D get_rotor_bivector() const;
	void set_rotor_bivector(const Bivector4D &p_bivector);
	AABB get_rotor_bivector_bind() const;
	void set_rotor_bivector_bind(const AABB &p_bivector);
	real_t get_rotor_scalar() const;
	void set_rotor_scalar(const real_t p_scalar);
	real_t get_rotor_pseudoscalar() const;
	void set_rotor_pseudoscalar(const real_t p_pseudoscalar);
	SplitComplex4D get_rotor_split_complex() const;
	void set_rotor_split_complex(const SplitComplex4D &p_split);
	Vector2 get_rotor_split_complex_bind() const;
	void set_rotor_split_complex_bind(const Vector2 &p_split);

	// Global transform and basis.
	Transform4D get_global_transform() const;
	void set_global_transform(const Transform4D &p_transform);
	PackedRealArray get_global_transform_array() const;
	void set_global_transform_array(const PackedRealArray &p_transform_array);
	Ref<godot_4d_bind::Transform4D> get_global_transform_bind() const;
	void set_global_transform_bind(const Ref<godot_4d_bind::Transform4D> &p_transform);
	Basis4D get_global_basis() const;
	void set_global_basis(const Basis4D &p_basis);
	Projection get_global_basis_bind() const;
	void set_global_basis_bind(const Projection &p_basis);

	// Global transform components.
	Vector4 get_global_position() const;
	void set_global_position(const Vector4 &p_global_position);

#ifdef CACHE_ROTATION_AND_SCALE
	Euler4D get_global_rotation();
	AABB get_global_rotation_bind();
	Ref<godot_4d_bind::Euler4D> get_global_rotation_euler_bind();
	Euler4D get_global_rotation_degrees();
	AABB get_global_rotation_degrees_bind();
	Ref<godot_4d_bind::Euler4D> get_global_rotation_degrees_euler_bind();
	Vector4 get_global_scale();
#else
	Euler4D get_global_rotation() const;
	AABB get_global_rotation_bind() const;
	Ref<godot_4d_bind::Euler4D> get_global_rotation_euler_bind() const;
	Euler4D get_global_rotation_degrees() const;
	AABB get_global_rotation_degrees_bind() const;
	Ref<godot_4d_bind::Euler4D> get_global_rotation_degrees_euler_bind() const;
	Vector4 get_global_scale() const;
#endif // CACHE_ROTATION_AND_SCALE
	real_t get_global_uniform_scale() const;
	void set_global_rotation(const Euler4D &p_global_euler);
	void set_global_rotation_bind(const AABB &p_global_euler);
	void set_global_rotation_euler_bind(const Ref<godot_4d_bind::Euler4D> &p_global_euler);
	void set_global_rotation_degrees(const Euler4D &p_global_euler);
	void set_global_rotation_degrees_bind(const AABB &p_global_euler);
	void set_global_rotation_degrees_euler_bind(const Ref<godot_4d_bind::Euler4D> &p_global_euler);
	void set_global_scale(const Vector4 &p_global_scale);
	void set_global_uniform_scale(const real_t p_global_scale);

	// Geometric algebra global rotation properties.
	// Bivector4D get_global_rotation_bivector_magnitude() const;
	void set_global_rotation_bivector_magnitude(const Bivector4D &p_bivector);
	// AABB get_global_rotation_bivector_magnitude_bind() const;
	void set_global_rotation_bivector_magnitude_bind(const AABB &p_bivector);
	Rotor4D get_global_rotation_rotor() const;
	void set_global_rotation_rotor(const Rotor4D &p_rotor);
	Ref<godot_4d_bind::Rotor4D> get_global_rotation_rotor_bind() const;
	void set_global_rotation_rotor_bind(const Ref<godot_4d_bind::Rotor4D> &p_rotor);
	PackedRealArray get_global_rotation_rotor_array() const;
	void set_global_rotation_rotor_array(const PackedRealArray &p_rotor_array);
	Bivector4D get_global_rotor_bivector() const;
	void set_global_rotor_bivector(const Bivector4D &p_bivector);
	AABB get_global_rotor_bivector_bind() const;
	void set_global_rotor_bivector_bind(const AABB &p_bivector);
	real_t get_global_rotor_scalar() const;
	void set_global_rotor_scalar(const real_t p_scalar);
	real_t get_global_rotor_pseudoscalar() const;
	void set_global_rotor_pseudoscalar(const real_t p_pseudoscalar);
	SplitComplex4D get_global_rotor_split_complex() const;
	void set_global_rotor_split_complex(const SplitComplex4D &p_split);
	Vector2 get_global_rotor_split_complex_bind() const;
	void set_global_rotor_split_complex_bind(const Vector2 &p_split);

	// Visibility.
	bool is_visible() const;
	bool is_visible_in_tree() const;
	void set_visible(const bool p_visible);

	// Rect bounds.
	virtual Rect4 get_rect_bounds(const Transform4D &p_inv_relative_to = Transform4D()) const;
	PackedVector4Array get_rect_bounds_bind(const Projection &p_basis = Projection(), const Vector4 &p_offset = Vector4()) const;
	Rect4 get_rect_bounds_recursive(const Transform4D &p_inv_relative_to = Transform4D()) const;
	PackedVector4Array get_rect_bounds_recursive_bind(const Projection &p_basis = Projection(), const Vector4 &p_offset = Vector4()) const;
	GDVIRTUAL2RC(PackedVector4Array, _get_rect_bounds, const Projection &, const Vector4 &);
};

VARIANT_ENUM_CAST(Node4D::RotationEditMode)
