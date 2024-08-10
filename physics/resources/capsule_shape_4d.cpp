#include "capsule_shape_4d.h"

real_t CapsuleShape4D::get_height() const {
	return _height;
}

void CapsuleShape4D::set_height(const real_t p_height) {
	_height = p_height;
}

real_t CapsuleShape4D::get_mid_height() const {
	return _height - _radius * 2.0f;
}

void CapsuleShape4D::set_mid_height(const real_t p_mid_height) {
	_height = p_mid_height + _radius * 2.0f;
}

real_t CapsuleShape4D::get_radius() const {
	return _radius;
}

void CapsuleShape4D::set_radius(const real_t p_radius) {
	_radius = p_radius;
}

bool CapsuleShape4D::has_point(const Vector4 &p_point) const {
	Vector4 abs_point = p_point.abs();
	const real_t half_mid_height = get_mid_height() * 0.5f;
	if (abs_point.y > half_mid_height) {
		abs_point.y -= half_mid_height;
	} else {
		abs_point.y = 0.0f;
	}
	return abs_point.length_squared() <= _radius * _radius;
}

void CapsuleShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_height"), &CapsuleShape4D::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &CapsuleShape4D::set_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_NONE, "suffix:m"), "set_height", "get_height");

	ClassDB::bind_method(D_METHOD("get_mid_height"), &CapsuleShape4D::get_mid_height);
	ClassDB::bind_method(D_METHOD("set_mid_height", "mid_height"), &CapsuleShape4D::set_mid_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mid_height", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_mid_height", "get_mid_height");

	ClassDB::bind_method(D_METHOD("get_radius"), &CapsuleShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CapsuleShape4D::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_NONE, "suffix:m"), "set_radius", "get_radius");
}
