#pragma once

#include "../nodes/node_4d.h"
#include "shapes/shape_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/physics_material.hpp>
#elif GODOT_MODULE
#include "scene/resources/physics_material.h"
#endif

class CollisionObject4D;
class PhysicsServer4D;

class CollisionShape4D : public Node4D {
	GDCLASS(CollisionShape4D, Node4D);

	Ref<Shape4D> _shape;
	Ref<PhysicsMaterial> _physics_material;
	CollisionObject4D *_collision_object = nullptr;
	uint32_t _collision_layer = 1;
	uint32_t _collision_mask = 1;

	CollisionObject4D *_get_ancestor_collision_object() const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// Layers and masks are per-shape.
	void set_collision_layer(uint32_t p_layer) { _collision_layer = p_layer; }
	uint32_t get_collision_layer() const { return _collision_layer; }
	void set_collision_mask(uint32_t p_mask) { _collision_mask = p_mask; }
	uint32_t get_collision_mask() const { return _collision_mask; }
	void set_collision_layer_value(int p_layer_number, bool p_value);
	bool get_collision_layer_value(int p_layer_number) const;
	void set_collision_mask_value(int p_layer_number, bool p_value);
	bool get_collision_mask_value(int p_layer_number) const;

	virtual Rect4 get_rect_bounds(const Transform4D &p_inv_relative_to = Transform4D()) const override;
	Transform4D get_transform_to_collision_object() const;

	Ref<PhysicsMaterial> get_physics_material() const;
	void set_physics_material(const Ref<PhysicsMaterial> &p_physics_material);

	Ref<Shape4D> get_shape() const;
	void set_shape(const Ref<Shape4D> &p_shape);

	real_t get_hypervolume() const;
	Vector4 get_nearest_global_point(const Vector4 &p_point) const;
	Vector4 get_support_global_point(const Vector4 &p_direction) const;
	real_t get_surface_volume() const;
	bool has_global_point(const Vector4 &p_point) const;
};
