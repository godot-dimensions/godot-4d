#include "collision_shape_4d.h"

#include "bodies/collision_object_4d.h"

CollisionObject4D *CollisionShape4D::_get_ancestor_collision_object() const {
	Node *parent = get_parent();
	while (parent) {
		Node4D *parent_4d = Object::cast_to<Node4D>(parent);
		if (unlikely(!parent_4d)) {
			return nullptr;
		}
		CollisionObject4D *co = Object::cast_to<CollisionObject4D>(parent);
		if (likely(co)) {
			return co;
		}
		parent = parent->get_parent();
	}
	return nullptr;
}

void CollisionShape4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_collision_object = _get_ancestor_collision_object();
			if (_collision_object) {
				_collision_object->register_collision_shape(this);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (_collision_object) {
				_collision_object->unregister_collision_shape(this);
			}
			_collision_object = nullptr;
		} break;
	}
}

Rect4 CollisionShape4D::get_rect_bounds(const Transform4D &p_inv_relative_to) const {
	if (_shape.is_null()) {
		return Rect4(p_inv_relative_to * get_global_position(), Vector4());
	}
	return _shape->get_rect_bounds(p_inv_relative_to * get_global_transform());
}

Transform4D CollisionShape4D::get_transform_to_collision_object() const {
	Transform4D transform_to_col_obj = get_transform();
	Node *parent = get_parent();
	while (parent != _collision_object) {
		Node4D *parent_4d = Object::cast_to<Node4D>(parent);
		if (unlikely(!parent_4d)) {
			break;
		}
		transform_to_col_obj = parent_4d->get_transform() * transform_to_col_obj;
		parent = parent->get_parent();
	}
	return transform_to_col_obj;
}

Ref<PhysicsMaterial> CollisionShape4D::get_physics_material() const {
	return _physics_material;
}

void CollisionShape4D::set_physics_material(const Ref<PhysicsMaterial> &p_physics_material) {
	_physics_material = p_physics_material;
}

Ref<Shape4D> CollisionShape4D::get_shape() const {
	return _shape;
}

void CollisionShape4D::set_shape(const Ref<Shape4D> &p_shape) {
	_shape = p_shape;
}

real_t CollisionShape4D::get_hypervolume() const {
	ERR_FAIL_COND_V(_shape.is_null(), 0.0);
	const real_t scale = get_global_basis().get_uniform_scale();
	return scale * _shape->get_hypervolume();
}

Vector4 CollisionShape4D::get_nearest_global_point(const Vector4 &p_point) const {
	ERR_FAIL_COND_V(_shape.is_null(), get_global_position());
	const Transform4D local_to_global = get_global_transform();
	const Transform4D global_to_local = local_to_global.inverse();
	return local_to_global * _shape->get_nearest_point(global_to_local * p_point);
}

Vector4 CollisionShape4D::get_support_global_point(const Vector4 &p_direction) const {
	ERR_FAIL_COND_V(_shape.is_null(), get_global_position());
	const Basis4D local_to_global = get_global_basis();
	const Basis4D global_to_local = local_to_global.inverse();
	return local_to_global * _shape->get_support_point(global_to_local * p_direction);
}

real_t CollisionShape4D::get_surface_volume() const {
	ERR_FAIL_COND_V(_shape.is_null(), 0.0);
	const real_t scale = get_global_basis().get_uniform_scale();
	return scale * _shape->get_surface_volume();
}

bool CollisionShape4D::has_global_point(const Vector4 &p_point) const {
	ERR_FAIL_COND_V(_shape.is_null(), false);
	const Transform4D global_to_local = get_global_transform().inverse();
	return _shape->has_point(global_to_local * p_point);
}

void CollisionShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_hypervolume"), &CollisionShape4D::get_hypervolume);
	ClassDB::bind_method(D_METHOD("get_nearest_global_point", "point"), &CollisionShape4D::get_nearest_global_point);
	ClassDB::bind_method(D_METHOD("get_support_global_point", "direction"), &CollisionShape4D::get_support_global_point);
	ClassDB::bind_method(D_METHOD("get_surface_volume"), &CollisionShape4D::get_surface_volume);
	ClassDB::bind_method(D_METHOD("has_global_point", "point"), &CollisionShape4D::has_global_point);

	ClassDB::bind_method(D_METHOD("get_shape"), &CollisionShape4D::get_shape);
	ClassDB::bind_method(D_METHOD("set_shape", "shape"), &CollisionShape4D::set_shape);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "Shape4D"), "set_shape", "get_shape");

	ClassDB::bind_method(D_METHOD("get_physics_material"), &CollisionShape4D::get_physics_material);
	ClassDB::bind_method(D_METHOD("set_physics_material", "physics_material"), &CollisionShape4D::set_physics_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "physics_material", PROPERTY_HINT_RESOURCE_TYPE, "PhysicsMaterial"), "set_physics_material", "get_physics_material");

	// Reuse 3D physics layer names for 4D to make use of Godot's built-in layer settings.
	ClassDB::bind_method(D_METHOD("get_collision_layer"), &CollisionShape4D::get_collision_layer);
	ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &CollisionShape4D::set_collision_layer);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_layer", "get_collision_layer");

	ClassDB::bind_method(D_METHOD("get_collision_mask"), &CollisionShape4D::get_collision_mask);
	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &CollisionShape4D::set_collision_mask);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");
}
