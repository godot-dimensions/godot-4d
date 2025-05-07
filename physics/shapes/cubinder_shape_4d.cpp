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

real_t CubinderShape4D::get_hypervolume() const {
	// http://hi.gher.space/wiki/Cubinder
	return Math_PI * _radius * _radius * _height * _thickness;
}

real_t CubinderShape4D::get_surface_volume() const {
	// Wiki describes the surcell volume as 2π*r(r*h + h^2) but that only applies when height == thickness.
	// Distributing gives us 2π*h*r^2 + 2π*r*h^2, where:
	// * The 2π*r*h^2 term is the surcell volume of the square prism, 2π*r long with a h*h base (square).
	// * The 2π*h*r^2 term is the surcell volume of the cylinders, h long with an 2π*r^2 base (circle).
	// With different height and thickness, 2π*r*h^2 becomes 2π*r*h*t, and the cylinders become π*h*r^2 + π*t*r^2 so π*r^2 * (h+t).
	// Proof: Try substituting t with h and it should collapse to the original formulas.
	return Math_TAU * _radius * _height * _thickness + Math_PI * _radius * _radius * (_height + _thickness);
}

Vector4 CubinderShape4D::get_nearest_point(const Vector4 &p_point) const {
	const real_t half_height = _height * 0.5f;
	const real_t half_thickness = _thickness * 0.5f;
	Vector4 nearest = Vector4(p_point.x, 0.0f, p_point.z, 0.0f);
	const real_t length_sq = nearest.length_squared();
	if (length_sq > _radius * _radius) {
		nearest = nearest * _radius / Math::sqrt(length_sq);
	}
	nearest.y = CLAMP(p_point.y, -half_height, half_height);
	nearest.w = CLAMP(p_point.w, -half_thickness, half_thickness);
	return nearest;
}

Vector4 CubinderShape4D::get_support_point(const Vector4 &p_direction) const {
	const real_t half_height = _height * 0.5f;
	const real_t half_thickness = _thickness * 0.5f;
	Vector4 support = Vector4(p_direction.x, 0.0f, p_direction.z, 0.0f).normalized() * _radius;
	support.y = (p_direction.y > 0.0f) ? half_height : -half_height;
	support.w = (p_direction.w > 0.0f) ? half_thickness : -half_thickness;
	return support;
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

bool CubinderShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	Ref<CubinderShape4D> cubinder = p_shape;
	if (cubinder.is_null()) {
		return false;
	}
	return _height == cubinder->_height && _radius == cubinder->_radius && _thickness == cubinder->_thickness;
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
