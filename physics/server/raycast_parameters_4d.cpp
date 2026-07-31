#include "raycast_parameters_4d.h"

#include "../../nodes/node_4d.h"

TypedArray<Node4D> RaycastParameters4D::get_exclude_nodes_bind() const {
	TypedArray<Node4D> ret;
	ret.resize(_exclude_nodes.size());
	for (int i = 0; i < _exclude_nodes.size(); i++) {
		// Must use the index operator here instead of `set()` due to `TypedArray`.
		Node4D *node_4d = Object::cast_to<Node4D>(ObjectDB::get_instance(_exclude_nodes[i]));
		ERR_CONTINUE_MSG(node_4d == nullptr, "RaycastParameters4D.get_exclude_nodes: The referenced node with ObjectID " + itos(_exclude_nodes[i]) + " is no longer valid.");
		ret[i] = node_4d;
	}
	return ret;
}

void RaycastParameters4D::set_exclude_nodes_bind(const TypedArray<Node4D> &p_exclude_nodes) {
	_exclude_nodes.resize(p_exclude_nodes.size());
	for (int i = 0; i < p_exclude_nodes.size(); i++) {
		Node4D *node_4d = Object::cast_to<Node4D>(p_exclude_nodes[i]);
		if (node_4d == nullptr) {
			ERR_PRINT("RaycastParameters4D.set_exclude_nodes: The passed node at index " + itos(i) + " is not a valid Node4D. Refusing all excluded nodes.");
			_exclude_nodes.clear();
			return;
		}
		// Must use `set()` here instead of the index operator due to `Vector`.
		_exclude_nodes.set(i, ObjectID(node_4d->get_instance_id()));
	}
}

void RaycastParameters4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_exclude_nodes"), &RaycastParameters4D::get_exclude_nodes_bind);
	ClassDB::bind_method(D_METHOD("set_exclude_nodes", "exclude_nodes"), &RaycastParameters4D::set_exclude_nodes_bind);
	// Don't bind a property for exclude_nodes, since we don't want to mislead users into thinking that
	// they can modify the returned array directly, and also this doesn't make sense to serialize.

	ClassDB::bind_method(D_METHOD("get_global_ray_direction"), &RaycastParameters4D::get_global_ray_direction);
	ClassDB::bind_method(D_METHOD("set_global_ray_direction", "global_ray_direction"), &RaycastParameters4D::set_global_ray_direction);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "global_ray_direction"), "set_global_ray_direction", "get_global_ray_direction");

	ClassDB::bind_method(D_METHOD("get_global_ray_origin"), &RaycastParameters4D::get_global_ray_origin);
	ClassDB::bind_method(D_METHOD("set_global_ray_origin", "global_ray_origin"), &RaycastParameters4D::set_global_ray_origin);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "global_ray_origin", PROPERTY_HINT_NONE, "suffix:m"), "set_global_ray_origin", "get_global_ray_origin");

	ClassDB::bind_method(D_METHOD("get_max_distance"), &RaycastParameters4D::get_max_distance);
	ClassDB::bind_method(D_METHOD("set_max_distance", "max_distance"), &RaycastParameters4D::set_max_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_distance", PROPERTY_HINT_RANGE, "0,1000000,0.01,or_greater,suffix:m"), "set_max_distance", "get_max_distance");

	// Reuse 3D physics layer names for 4D to make use of Godot's built-in layer settings.
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &RaycastParameters4D::get_collision_mask);
	ClassDB::bind_method(D_METHOD("set_collision_mask", "collision_mask"), &RaycastParameters4D::set_collision_mask);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");

	ClassDB::bind_method(D_METHOD("get_collide_with_areas"), &RaycastParameters4D::get_collide_with_areas);
	ClassDB::bind_method(D_METHOD("set_collide_with_areas", "collide_with_areas"), &RaycastParameters4D::set_collide_with_areas);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collide_with_areas"), "set_collide_with_areas", "get_collide_with_areas");

	ClassDB::bind_method(D_METHOD("get_collide_with_bodies"), &RaycastParameters4D::get_collide_with_bodies);
	ClassDB::bind_method(D_METHOD("set_collide_with_bodies", "collide_with_bodies"), &RaycastParameters4D::set_collide_with_bodies);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collide_with_bodies"), "set_collide_with_bodies", "get_collide_with_bodies");

	ClassDB::bind_method(D_METHOD("get_inside_is_skip"), &RaycastParameters4D::get_inside_is_skip);
	ClassDB::bind_method(D_METHOD("set_inside_is_skip", "inside_is_skip"), &RaycastParameters4D::set_inside_is_skip);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "inside_is_skip"), "set_inside_is_skip", "get_inside_is_skip");

	ClassDB::bind_method(D_METHOD("get_inside_is_zero"), &RaycastParameters4D::get_inside_is_zero);
	ClassDB::bind_method(D_METHOD("set_inside_is_zero", "inside_is_zero"), &RaycastParameters4D::set_inside_is_zero);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "inside_is_zero"), "set_inside_is_zero", "get_inside_is_zero");
}
