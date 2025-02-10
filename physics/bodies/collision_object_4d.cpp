#include "collision_object_4d.h"

#include "../collision_shape_4d.h"

void CollisionObject4D::register_collision_shape(CollisionShape4D *p_shape) {
	ERR_FAIL_COND(_collision_shapes.has(p_shape));
	_collision_shapes.push_back(p_shape);
}

void CollisionObject4D::unregister_collision_shape(CollisionShape4D *p_shape) {
	ERR_FAIL_COND(!_collision_shapes.has(p_shape));
	_collision_shapes.erase(p_shape);
}

TypedArray<CollisionShape4D> CollisionObject4D::get_collision_shapes() const {
	return _collision_shapes;
}

void CollisionObject4D::_bind_methods() {
	ADD_SIGNAL(MethodInfo("self_entered_area", PropertyInfo(Variant::OBJECT, "area", PROPERTY_HINT_RESOURCE_TYPE, "Area4D")));
	ADD_SIGNAL(MethodInfo("self_exited_area", PropertyInfo(Variant::OBJECT, "area", PROPERTY_HINT_RESOURCE_TYPE, "Area4D")));
}
