#include "cubinder_shape_4d.h"

real_t CubinderShape4D::get_height() const {
	return _height;
}

void CubinderShape4D::set_height(const real_t p_height) {
	_height = p_height;
}

real_t CubinderShape4D::get_radius() const {
	return _radius;
}

void CubinderShape4D::set_radius(const real_t p_radius) {
	_radius = p_radius;
}

real_t CubinderShape4D::get_thickness() const {
	return _thickness;
}

void CubinderShape4D::set_thickness(const real_t p_thickness) {
	_thickness = p_thickness;
}

Vector2 CubinderShape4D::get_size_wy() const {
	return Vector2(_thickness, _height);
}

void CubinderShape4D::set_size_wy(const Vector2 &p_size_wy) {
	_thickness = p_size_wy.x;
	_height = p_size_wy.y;
}

bool CubinderShape4D::has_point(const Vector4 &p_point) const {
	Vector4 abs_point = p_point.abs();
	if (abs_point.y > _height * 0.5f) {
		return false;
	}
	abs_point.y = 0.0f;
	if (abs_point.w > _thickness * 0.5f) {
		return false;
	}
	abs_point.w = 0.0f;
	return abs_point.length_squared() <= _radius * _radius;
}

void CubinderShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_height"), &CubinderShape4D::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &CubinderShape4D::set_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_NONE, "suffix:m"), "set_height", "get_height");

	ClassDB::bind_method(D_METHOD("get_radius"), &CubinderShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CubinderShape4D::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_NONE, "suffix:m"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("get_thickness"), &CubinderShape4D::get_thickness);
	ClassDB::bind_method(D_METHOD("set_thickness", "thickness"), &CubinderShape4D::set_thickness);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "thickness", PROPERTY_HINT_NONE, "suffix:m"), "set_thickness", "get_thickness");

	ClassDB::bind_method(D_METHOD("get_size_wy"), &CubinderShape4D::get_size_wy);
	ClassDB::bind_method(D_METHOD("set_size_wy", "size_wy"), &CubinderShape4D::set_size_wy);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "size_wy", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_size_wy", "get_size_wy");
}
