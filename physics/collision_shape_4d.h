#pragma once

#include "../nodes/node_4d.h"
#include "shapes/shape_4d.h"

class CollisionObject4D;

class CollisionShape4D : public Node4D {
	GDCLASS(CollisionShape4D, Node4D);

	Ref<Shape4D> _shape;
	CollisionObject4D *_collision_object = nullptr;
	uint32_t _collision_layer = 1;
	uint32_t _collision_mask = 1;

	CollisionObject4D *_get_ancestor_collision_object() const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// Layers and masks are per-shape.
	void set_collision_layer(uint32_t p_layer);
	uint32_t get_collision_layer() const;
	void set_collision_mask(uint32_t p_mask);
	uint32_t get_collision_mask() const;
	void set_collision_layer_value(int p_layer_number, bool p_value);
	bool get_collision_layer_value(int p_layer_number) const;
	void set_collision_mask_value(int p_layer_number, bool p_value);
	bool get_collision_mask_value(int p_layer_number) const;

	Transform4D get_transform_to_collision_object() const;

	Ref<Shape4D> get_shape() const;
	void set_shape(const Ref<Shape4D> &p_shape);

	real_t get_hypervolume() const;
	Vector4 get_nearest_global_point(const Vector4 &p_point) const;
	Vector4 get_support_global_point(const Vector4 &p_direction) const;
	real_t get_surface_volume() const;
	bool has_global_point(const Vector4 &p_point) const;
};
