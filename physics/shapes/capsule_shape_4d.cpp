#include "capsule_shape_4d.h"

real_t CapsuleShape4D::get_radius() const {
	return _radius;
}

void CapsuleShape4D::set_radius(const real_t p_radius) {
	_radius = p_radius;
}

real_t CapsuleShape4D::get_full_height() const {
	if (_height_is_full) {
		return _height;
	}
	return _height + _radius * 2.0f;
}

void CapsuleShape4D::set_full_height(const real_t p_full_height) {
	if (_height_is_full) {
		_height = p_full_height;
	} else {
		_height = p_full_height - _radius * 2.0f;
	}
}

real_t CapsuleShape4D::get_mid_height() const {
	if (_height_is_full) {
		return _height - _radius * 2.0f;
	}
	return _height;
}

void CapsuleShape4D::set_mid_height(const real_t p_mid_height) {
	if (_height_is_full) {
		_height = p_mid_height + _radius * 2.0f;
	} else {
		_height = p_mid_height;
	}
}

bool CapsuleShape4D::get_height_is_full() const {
	return _height_is_full;
}

void CapsuleShape4D::set_height_is_full(const bool p_height_is_full) {
	if (p_height_is_full == _height_is_full) {
		return;
	}
	_height_is_full = p_height_is_full;
	if (p_height_is_full) {
		// Convert from mid-height to full height.
		_height += _radius * 2.0f;
	} else {
		// Convert from full height to mid-height.
		_height -= _radius * 2.0f;
	}
	notify_property_list_changed();
}

Dictionary CapsuleShape4D::raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance, const bool p_inside_is_zero) const {
	Dictionary result;
	result["hit"] = false;
	ERR_FAIL_COND_V_MSG(!p_local_direction.is_normalized(), result, "CapsuleShape4D::raycast_intersects: Ray direction must be normalized.");
	if (p_inside_is_zero && has_point(p_local_from)) {
		result["hit"] = true;
		result["distance"] = 0.0f;
		result["normal"] = Vector4(0.0, 0.0, 0.0, 0.0);
		result["point"] = p_local_from;
		return result;
	}
	// Capsule raycasting: sweep a sphere along the Y axis.
	// The capsule is a cylinder of height (h - 2r) with hemispherical caps.
	// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
	const real_t half_mid_height = get_mid_height() * 0.5f;
	const real_t radius_squared = _radius * _radius;
	real_t best_distance = p_max_distance;
	Vector4 best_normal = Vector4();
	// Ray-cylinder intersection (the middle part).
	const Vector4 radial_point = Vector4(p_local_from.x, 0.0f, p_local_from.z, p_local_from.w);
	const Vector4 radial_dir = Vector4(p_local_direction.x, 0.0f, p_local_direction.z, p_local_direction.w);
	const real_t radial_dir_len_sq = radial_dir.length_squared();
	const real_t radial_point_dot_radial_dir = radial_point.dot(radial_dir);
	const real_t radial_point_len_sq = radial_point.length_squared();
	// Check ray-infinite-cylinder intersection (the middle section).
	if (radial_dir_len_sq > CMP_EPSILON) {
		const real_t discriminant = radial_point_dot_radial_dir * radial_point_dot_radial_dir - radial_dir_len_sq * (radial_point_len_sq - radius_squared);
		if (discriminant >= 0.0f) {
			const real_t sqrt_discriminant = Math::sqrt(discriminant);
			const real_t distance_near = (-radial_point_dot_radial_dir - sqrt_discriminant) / radial_dir_len_sq;
			const real_t distance_far = (-radial_point_dot_radial_dir + sqrt_discriminant) / radial_dir_len_sq;
			const real_t cylinder_distance = (distance_near >= 0.0f) ? distance_near : distance_far;
			if (cylinder_distance >= 0.0f) {
				const Vector4 hit_point = p_local_from + p_local_direction * cylinder_distance;
				if (Math::abs(hit_point.y) <= half_mid_height) {
					if (cylinder_distance < best_distance) {
						best_distance = cylinder_distance;
						const Vector4 hit_radial = Vector4(hit_point.x, 0.0f, hit_point.z, hit_point.w);
						best_normal = hit_radial.normalized();
					}
				}
			}
		}
	}
	// Check ray-sphere intersection at the top cap (center at y = half_mid_height).
	{
		const Vector4 sphere_center_offset_top = p_local_from - Vector4(0, half_mid_height, 0, 0);
		const real_t direction_length_squared = p_local_direction.length_squared();
		const real_t direction_dot_offset_top = p_local_direction.dot(sphere_center_offset_top);
		const real_t offset_length_squared_top = sphere_center_offset_top.length_squared();
		const real_t discriminant_top = direction_dot_offset_top * direction_dot_offset_top - direction_length_squared * (offset_length_squared_top - radius_squared);
		if (discriminant_top >= 0.0f) {
			const real_t sqrt_discriminant_top = Math::sqrt(discriminant_top);
			const real_t distance_near_top = (-direction_dot_offset_top - sqrt_discriminant_top) / direction_length_squared;
			const real_t distance_far_top = (-direction_dot_offset_top + sqrt_discriminant_top) / direction_length_squared;
			// Check near distance first, then far distance.
			real_t distance_top = -1.0f;
			if (distance_near_top >= 0.0f) {
				const Vector4 hit_point_near = p_local_from + p_local_direction * distance_near_top;
				if (hit_point_near.y >= half_mid_height) {
					distance_top = distance_near_top;
				}
			}
			// If near distance didn't work, try far distance.
			if (distance_top < 0.0f && distance_far_top >= 0.0f) {
				const Vector4 hit_point_far = p_local_from + p_local_direction * distance_far_top;
				if (hit_point_far.y >= half_mid_height) {
					distance_top = distance_far_top;
				}
			}

			if (distance_top >= 0.0f && distance_top < best_distance) {
				const Vector4 hit_point_top = p_local_from + p_local_direction * distance_top;
				best_distance = distance_top;
				best_normal = (hit_point_top - Vector4(0, half_mid_height, 0, 0)).normalized();
			}
		}
	}
	// Check ray-sphere intersection at the bottom cap (center at y = -half_mid_height).
	{
		const Vector4 sphere_center_offset_bottom = p_local_from - Vector4(0, -half_mid_height, 0, 0);
		const real_t direction_length_squared = p_local_direction.length_squared();
		const real_t direction_dot_offset_bottom = p_local_direction.dot(sphere_center_offset_bottom);
		const real_t offset_length_squared_bottom = sphere_center_offset_bottom.length_squared();
		const real_t discriminant_bottom = direction_dot_offset_bottom * direction_dot_offset_bottom - direction_length_squared * (offset_length_squared_bottom - radius_squared);
		if (discriminant_bottom >= 0.0f) {
			const real_t sqrt_discriminant_bottom = Math::sqrt(discriminant_bottom);
			const real_t distance_near_bottom = (-direction_dot_offset_bottom - sqrt_discriminant_bottom) / direction_length_squared;
			const real_t distance_far_bottom = (-direction_dot_offset_bottom + sqrt_discriminant_bottom) / direction_length_squared;
			// Check near distance first, then far distance.
			real_t distance_bottom = -1.0f;
			if (distance_near_bottom >= 0.0f) {
				const Vector4 hit_point_near = p_local_from + p_local_direction * distance_near_bottom;
				if (hit_point_near.y <= -half_mid_height) {
					distance_bottom = distance_near_bottom;
				}
			}
			if (distance_bottom < 0.0f && distance_far_bottom >= 0.0f) {
				// If near distance didn't work, try far distance.
				// Both inside and outside hit at far, so it should be checked after the near check.
				const Vector4 hit_point_far = p_local_from + p_local_direction * distance_far_bottom;
				if (hit_point_far.y <= -half_mid_height) {
					distance_bottom = distance_far_bottom;
				}
			}
			if (distance_bottom >= 0.0f && distance_bottom < best_distance) {
				const Vector4 hit_point_bottom = p_local_from + p_local_direction * distance_bottom;
				best_distance = distance_bottom;
				best_normal = (hit_point_bottom - Vector4(0, -half_mid_height, 0, 0)).normalized();
			}
		}
	}
	if (best_distance < p_max_distance) {
		result["hit"] = true;
		result["distance"] = best_distance;
		result["normal"] = best_normal;
		result["point"] = p_local_from + p_local_direction * best_distance;
	}
	return result;
}

real_t CapsuleShape4D::get_hypervolume() const {
	const real_t mid_height = get_mid_height();
	return (0.125 * Math_TAU * Math_TAU) * (_radius * _radius * _radius * _radius) + (2.0f * Math_TAU / 3.0f) * (_radius * _radius * _radius * mid_height);
}

real_t CapsuleShape4D::get_signed_distance_to_surface(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_surface) const {
	const real_t half_mid_height = get_mid_height() * 0.5f;
	Vector4 sphere_relative = p_local_point;
	if (p_local_point.y > half_mid_height) {
		sphere_relative.y -= half_mid_height;
	} else if (p_local_point.y < -half_mid_height) {
		sphere_relative.y += half_mid_height;
	} else {
		sphere_relative.y = 0.0f;
	}
	const real_t sphere_center_distance = sphere_relative.length();
	if (r_nearest_point_on_surface != nullptr) {
		if (sphere_center_distance == 0.0f) {
			sphere_relative = Vector4(_radius, 0.0f, 0.0f, 0.0f); // Arbitrary point on the surface.
		} else {
			sphere_relative *= _radius / sphere_center_distance;
		}
		if (p_local_point.y > half_mid_height) {
			sphere_relative.y += half_mid_height;
		} else if (p_local_point.y < -half_mid_height) {
			sphere_relative.y -= half_mid_height;
		} else {
			sphere_relative.y = p_local_point.y;
		}
		*r_nearest_point_on_surface = sphere_relative;
	}
	return sphere_center_distance - _radius;
}

Vector4 CapsuleShape4D::get_nearest_point(const Vector4 &p_local_point) const {
	const real_t half_mid_height = get_mid_height() * 0.5f;
	if (p_local_point.y > half_mid_height) {
		Vector4 nearest = Vector4(p_local_point.x, p_local_point.y - half_mid_height, p_local_point.z, p_local_point.w);
		const real_t near_len_sq = nearest.length_squared();
		if (near_len_sq > _radius * _radius) {
			nearest *= _radius / Math::sqrt(near_len_sq);
		}
		nearest.y += half_mid_height;
		return nearest;
	}
	if (p_local_point.y < -half_mid_height) {
		Vector4 nearest = Vector4(p_local_point.x, p_local_point.y + half_mid_height, p_local_point.z, p_local_point.w);
		const real_t near_len_sq = nearest.length_squared();
		if (near_len_sq > _radius * _radius) {
			nearest *= _radius / Math::sqrt(near_len_sq);
		}
		nearest.y -= half_mid_height;
		return nearest;
	}
	// Point's Y coordinate is within the capsule's height.
	Vector4 nearest = Vector4(p_local_point.x, 0.0f, p_local_point.z, p_local_point.w);
	const real_t near_len_sq = nearest.length_squared();
	if (near_len_sq > _radius * _radius) {
		nearest *= _radius / Math::sqrt(near_len_sq);
	}
	nearest.y = p_local_point.y;
	return nearest;
}

Vector4 CapsuleShape4D::get_support_point(const Vector4 &p_local_direction) const {
	const real_t half_mid_height = get_mid_height() * 0.5f;
	Vector4 nearest = p_local_direction.normalized() * _radius;
	nearest.y += (p_local_direction.y > 0.0f) ? half_mid_height : -half_mid_height;
	return nearest;
}

real_t CapsuleShape4D::get_surface_volume() const {
	const real_t mid_height = get_mid_height();
	return (0.5 * Math_TAU * Math_TAU) * (_radius * _radius * _radius) + (2.0 * Math_TAU) * (_radius * _radius * mid_height);
}

bool CapsuleShape4D::has_point(const Vector4 &p_local_point) const {
	Vector4 abs_point = p_local_point.abs();
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
	return _height == capsule_shape->_height && _radius == capsule_shape->_radius && _height_is_full == capsule_shape->_height_is_full;
}

void CapsuleShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius"), &CapsuleShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CapsuleShape4D::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_NONE, "suffix:m"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("get_full_height"), &CapsuleShape4D::get_full_height);
	ClassDB::bind_method(D_METHOD("set_full_height", "full_height"), &CapsuleShape4D::set_full_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "full_height", PROPERTY_HINT_NONE, "suffix:m"), "set_full_height", "get_full_height");

	ClassDB::bind_method(D_METHOD("get_mid_height"), &CapsuleShape4D::get_mid_height);
	ClassDB::bind_method(D_METHOD("set_mid_height", "mid_height"), &CapsuleShape4D::set_mid_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mid_height", PROPERTY_HINT_NONE, "suffix:m"), "set_mid_height", "get_mid_height");

	ClassDB::bind_method(D_METHOD("get_height_is_full"), &CapsuleShape4D::get_height_is_full);
	ClassDB::bind_method(D_METHOD("set_height_is_full", "height_is_full"), &CapsuleShape4D::set_height_is_full);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "height_is_full"), "set_height_is_full", "get_height_is_full");
}

void CapsuleShape4D::_validate_property(PropertyInfo &p_property) const {
	if (_height_is_full) {
		if (p_property.name == StringName("mid_height")) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	} else {
		if (p_property.name == StringName("full_height")) {
			p_property.usage = PROPERTY_USAGE_NONE;
		}
	}
}
