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

real_t CapsuleShape4D::get_hypervolume() const {
	return (0.125 * Math_TAU * Math_TAU) * (_radius * _radius * _radius * _radius) + (2.0f * Math_TAU / 3.0f) * (_radius * _radius * _radius * _height);
}

Vector4 CapsuleShape4D::get_nearest_point(const Vector4 &p_point) const {
	const real_t half_mid_height = get_mid_height() * 0.5f;
	if (p_point.y > half_mid_height) {
		Vector4 nearest = Vector4(p_point.x, p_point.y - half_mid_height, p_point.z, p_point.w);
		const real_t near_len_sq = nearest.length_squared();
		if (near_len_sq > _radius * _radius) {
			nearest *= _radius / Math::sqrt(near_len_sq);
		}
		nearest.y += half_mid_height;
		return nearest;
	}
	if (p_point.y < -half_mid_height) {
		Vector4 nearest = Vector4(p_point.x, p_point.y + half_mid_height, p_point.z, p_point.w);
		const real_t near_len_sq = nearest.length_squared();
		if (near_len_sq > _radius * _radius) {
			nearest *= _radius / Math::sqrt(near_len_sq);
		}
		nearest.y -= half_mid_height;
		return nearest;
	}
	// Point's Y coordinate is within the capsule's height.
	Vector4 nearest = Vector4(p_point.x, 0.0f, p_point.z, p_point.w);
	const real_t near_len_sq = nearest.length_squared();
	if (near_len_sq > _radius * _radius) {
		nearest *= _radius / Math::sqrt(near_len_sq);
	}
	nearest.y = p_point.y;
	return nearest;
}

Vector4 CapsuleShape4D::get_support_point(const Vector4 &p_direction) const {
	const real_t half_mid_height = _height * 0.5f - _radius;
	Vector4 nearest = p_direction.normalized() * _radius;
	nearest.y += (p_direction.y > 0.0f) ? half_mid_height : -half_mid_height;
	return nearest;
}

real_t CapsuleShape4D::get_surface_volume() const {
	return (0.5 * Math_TAU * Math_TAU) * (_radius * _radius * _radius) + (2.0 * Math_TAU) * (_radius * _radius * _height);
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

bool CapsuleShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<CapsuleShape4D> capsule_shape = p_shape;
	if (capsule_shape.is_null()) {
		return false;
	}
	return _height == capsule_shape->get_height() && _radius == capsule_shape->get_radius();
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
