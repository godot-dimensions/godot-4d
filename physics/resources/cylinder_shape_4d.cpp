#include "cylinder_shape_4d.h"

real_t CylinderShape4D::get_height() const {
	return _height;
}

void CylinderShape4D::set_height(const real_t p_height) {
	_height = p_height;
}

real_t CylinderShape4D::get_radius() const {
	return _radius;
}

void CylinderShape4D::set_radius(const real_t p_radius) {
	_radius = p_radius;
}

bool CylinderShape4D::has_point(const Vector4 &p_point) const {
	Vector4 abs_point = p_point.abs();
	if (abs_point.y > _height * 0.5f) {
		return false;
	}
	abs_point.y = 0.0f;
	return abs_point.length_squared() <= _radius * _radius;
}

void CylinderShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_height"), &CylinderShape4D::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &CylinderShape4D::set_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_NONE, "suffix:m"), "set_height", "get_height");

	ClassDB::bind_method(D_METHOD("get_radius"), &CylinderShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CylinderShape4D::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_NONE, "suffix:m"), "set_radius", "get_radius");
}
