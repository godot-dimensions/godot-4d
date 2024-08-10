#include "collision_shape_4d.h"

Ref<Shape4D> CollisionShape4D::get_shape() const {
	return _shape;
}

void CollisionShape4D::set_shape(const Ref<Shape4D> &p_shape) {
	_shape = p_shape;
}

bool CollisionShape4D::has_global_point(const Vector4 &p_point) const {
	ERR_FAIL_COND_V(_shape.is_null(), false);
	Transform4D global_to_local = get_global_transform().inverse();
	return _shape->has_point(global_to_local * p_point);
}

void CollisionShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_shape"), &CollisionShape4D::get_shape);
	ClassDB::bind_method(D_METHOD("set_shape", "shape"), &CollisionShape4D::set_shape);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "Shape4D"), "set_shape", "get_shape");

	ClassDB::bind_method(D_METHOD("has_global_point", "point"), &CollisionShape4D::has_global_point);
}
