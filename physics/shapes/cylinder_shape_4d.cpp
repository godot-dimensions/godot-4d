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

real_t CylinderShape4D::get_hypervolume() const {
	return (4.0f * Math_PI / 3.0f) * (_radius * _radius * _radius * _height);
}

Vector4 CylinderShape4D::get_nearest_point(const Vector4 &p_point) const {
	const real_t half_height = _height * 0.5f;
	Vector4 nearest = Vector4(p_point.x, 0.0f, p_point.z, p_point.w);
	const real_t near_len_sq = nearest.length_squared();
	if (near_len_sq > _radius * _radius) {
		nearest *= _radius / Math::sqrt(near_len_sq);
	}
	if (p_point.y > half_height) {
		nearest.y = half_height;
	} else if (p_point.y < -half_height) {
		nearest.y = -half_height;
	} else {
		nearest.y = p_point.y;
	}
	return nearest;
}

Vector4 CylinderShape4D::get_support_point(const Vector4 &p_direction) const {
	const real_t half_height = _height * 0.5f;
	Vector4 support = Vector4(p_direction.x, 0.0f, p_direction.z, p_direction.w).normalized() * _radius;
	support.y = (p_direction.y > 0.0f) ? half_height : -half_height;
	return support;
}

real_t CylinderShape4D::get_surface_volume() const {
	return (8.0 * Math_PI / 3.0) * (_radius * _radius * _radius) + (4.0 * Math_PI) * (_radius * _radius * _height);
}

bool CylinderShape4D::has_point(const Vector4 &p_point) const {
	Vector4 abs_point = p_point.abs();
	if (abs_point.y > _height * 0.5f) {
		return false;
	}
	abs_point.y = 0.0f;
	return abs_point.length_squared() <= _radius * _radius;
}

bool CylinderShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<CylinderShape4D> cylinder_shape = p_shape;
	if (cylinder_shape.is_null()) {
		return false;
	}
	return _height == cylinder_shape->_height && _radius == cylinder_shape->_radius;
}

void CylinderShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_height"), &CylinderShape4D::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &CylinderShape4D::set_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_NONE, "suffix:m"), "set_height", "get_height");

	ClassDB::bind_method(D_METHOD("get_radius"), &CylinderShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CylinderShape4D::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_NONE, "suffix:m"), "set_radius", "get_radius");
}
