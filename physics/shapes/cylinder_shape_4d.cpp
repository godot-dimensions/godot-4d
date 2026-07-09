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

Dictionary CylinderShape4D::raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction) const {
	Dictionary result;
	result["hit"] = false;
	ERR_FAIL_COND_V_MSG(!p_local_direction.is_normalized(), result, "CylinderShape4D::raycast_intersects: Ray direction must be normalized.");
	// Cylinder raycasting: infinite cylinder along Y axis, truncated by height.
	// See also the similar code in `CubinderShape4D::raycast_intersects`.
	// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
	const real_t half_height = _height * 0.5f;
	// Ray-infinite-cylinder intersection in the XZW plane.
	const Vector4 radial_point = Vector4(p_local_from.x, 0.0f, p_local_from.z, p_local_from.w);
	const Vector4 radial_dir = Vector4(p_local_direction.x, 0.0f, p_local_direction.z, p_local_direction.w);
	const real_t radial_dir_len_sq = radial_dir.length_squared();
	const real_t radial_point_dot_radial_dir = radial_point.dot(radial_dir);
	const real_t radial_point_len_sq = radial_point.length_squared();
	const real_t radius_sq = _radius * _radius;
	// Check if starting point is inside the cylinder (radially and vertically).
	const bool start_inside = radial_point_len_sq <= radius_sq && Math::abs(p_local_from.y) <= half_height;
	if (radial_dir_len_sq <= CMP_EPSILON2) {
		// Ray moves only along Y axis, parallel to the cylinder axis.
		// Check if we're inside the cylinder radially.
		if (radial_point_len_sq <= radius_sq && Math::abs(p_local_direction.y) > CMP_EPSILON2) {
			// We're inside radially, so we'll hit a cap.
			const real_t distance_to_cap = (p_local_direction.y > 0.0f) ? (half_height - p_local_from.y) / p_local_direction.y : (-half_height - p_local_from.y) / p_local_direction.y;
			if (distance_to_cap >= 0.0f) {
				const Vector4 normal = (p_local_direction.y > 0.0f) ? Vector4(0, 1, 0, 0) : Vector4(0, -1, 0, 0);
				result["hit"] = true;
				result["distance"] = distance_to_cap;
				result["normal"] = normal;
			}
		}
		// If we're outside radially, we won't hit the cylinder at all.
		return result;
	}
	const real_t discriminant = radial_point_dot_radial_dir * radial_point_dot_radial_dir - radial_dir_len_sq * (radial_point_len_sq - radius_sq);
	if (discriminant < 0.0f) {
		return result; // No intersection with infinite cylinder.
	}
	const real_t sqrt_discriminant = Math::sqrt(discriminant);
	if (start_inside) {
		// Ray starts inside, use exit point.
		const real_t distance_to_exit = (-radial_point_dot_radial_dir + sqrt_discriminant) / radial_dir_len_sq;
		if (distance_to_exit >= 0.0f) {
			const Vector4 exit_point = p_local_from + p_local_direction * distance_to_exit;
			if (Math::abs(exit_point.y) <= half_height) {
				// Exit through curved surface.
				const Vector4 hit_radial = Vector4(exit_point.x, 0.0f, exit_point.z, exit_point.w);
				const Vector4 normal = hit_radial.normalized();
				result["hit"] = true;
				result["distance"] = distance_to_exit;
				result["normal"] = normal;
				return result;
			}
		}
		// Exit point is outside height bounds, check cap.
		if (Math::abs(p_local_direction.y) > CMP_EPSILON2) {
			// Since we started inside, which cap to use is determined by the direction of the ray.
			const real_t distance_to_cap = (p_local_direction.y > 0.0f) ? (half_height - p_local_from.y) / p_local_direction.y : (-half_height - p_local_from.y) / p_local_direction.y;
			if (distance_to_cap >= 0.0f) {
				const Vector4 cap_point = p_local_from + p_local_direction * distance_to_cap;
				const Vector4 cap_radial = Vector4(cap_point.x, 0.0f, cap_point.z, cap_point.w);
				if (cap_radial.length_squared() <= radius_sq) {
					const Vector4 normal = (p_local_direction.y > 0.0f) ? Vector4(0, 1, 0, 0) : Vector4(0, -1, 0, 0);
					result["hit"] = true;
					result["distance"] = distance_to_cap;
					result["normal"] = normal;
				}
			}
		}
	} else {
		// Ray starts outside.
		real_t best_distance = Math_INF;
		// Check both caps.
		if (Math::abs(p_local_direction.y) > CMP_EPSILON2) {
			// Try top cap.
			const real_t distance_to_top_cap = (half_height - p_local_from.y) / p_local_direction.y;
			if (distance_to_top_cap >= 0.0f) {
				const Vector4 cap_point = p_local_from + p_local_direction * distance_to_top_cap;
				const Vector4 cap_radial = Vector4(cap_point.x, 0.0f, cap_point.z, cap_point.w);
				if (cap_radial.length_squared() <= radius_sq) {
					best_distance = distance_to_top_cap;
					result["hit"] = true;
					result["distance"] = distance_to_top_cap;
					result["normal"] = Vector4(0, 1, 0, 0);
				}
			}
			// Try bottom cap.
			const real_t distance_to_bottom_cap = (-half_height - p_local_from.y) / p_local_direction.y;
			if (distance_to_bottom_cap >= 0.0f && distance_to_bottom_cap < best_distance) {
				const Vector4 cap_point = p_local_from + p_local_direction * distance_to_bottom_cap;
				const Vector4 cap_radial = Vector4(cap_point.x, 0.0f, cap_point.z, cap_point.w);
				if (cap_radial.length_squared() <= radius_sq) {
					best_distance = distance_to_bottom_cap;
					result["hit"] = true;
					result["distance"] = distance_to_bottom_cap;
					result["normal"] = Vector4(0, -1, 0, 0);
				}
			}
		}
		// Check curved surface entrance (only if no cap hit or curved surface is closer).
		const real_t distance_to_entrance = (-radial_point_dot_radial_dir - sqrt_discriminant) / radial_dir_len_sq;
		if (distance_to_entrance >= 0.0f) {
			const Vector4 entrance_point = p_local_from + p_local_direction * distance_to_entrance;
			if (Math::abs(entrance_point.y) <= half_height && distance_to_entrance < best_distance) {
				// Entrance through curved surface.
				const Vector4 hit_radial = Vector4(entrance_point.x, 0.0f, entrance_point.z, entrance_point.w);
				const Vector4 normal = hit_radial.normalized();
				result["hit"] = true;
				result["distance"] = distance_to_entrance;
				result["normal"] = normal;
			}
		}
	}
	return result;
}

real_t CylinderShape4D::get_signed_distance_to_surface(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_surface) const {
	const real_t half_height = _height * 0.5f;
	Vector4 nearest = Vector4(p_local_point.x, 0.0f, p_local_point.z, p_local_point.w);
	const real_t flat_length = nearest.length();
	const real_t radial_signed_distance = flat_length - _radius;
	const real_t vertical_signed_distance = Math::abs(p_local_point.y) - half_height;
	const real_t abs_radial_signed_distance = Math::abs(radial_signed_distance);
	const real_t abs_vertical_signed_distance = Math::abs(vertical_signed_distance);
	if (r_nearest_point_on_surface != nullptr) {
		if (radial_signed_distance > 0.0f || abs_radial_signed_distance >= abs_vertical_signed_distance) {
			if (flat_length == 0.0f) {
				nearest = Vector4(_radius, 0.0f, 0.0f, 0.0f);
			} else {
				nearest *= _radius / flat_length;
			}
		}
		if (vertical_signed_distance > 0.0f || abs_vertical_signed_distance > abs_radial_signed_distance) {
			nearest.y = (p_local_point.y > 0.0f) ? half_height : -half_height;
		} else {
			nearest.y = p_local_point.y;
		}
		*r_nearest_point_on_surface = nearest;
	}
	// Return the smallest signed distance, which corresponds to the closest surface.
	if (abs_vertical_signed_distance < abs_radial_signed_distance) {
		return vertical_signed_distance;
	}
	return radial_signed_distance;
}

Vector4 CylinderShape4D::get_nearest_point(const Vector4 &p_local_point) const {
	const real_t half_height = _height * 0.5f;
	Vector4 nearest = Vector4(p_local_point.x, 0.0f, p_local_point.z, p_local_point.w);
	const real_t near_len_sq = nearest.length_squared();
	if (near_len_sq > _radius * _radius) {
		nearest *= _radius / Math::sqrt(near_len_sq);
	}
	if (p_local_point.y > half_height) {
		nearest.y = half_height;
	} else if (p_local_point.y < -half_height) {
		nearest.y = -half_height;
	} else {
		nearest.y = p_local_point.y;
	}
	return nearest;
}

Vector4 CylinderShape4D::get_support_point(const Vector4 &p_local_direction) const {
	const real_t half_height = _height * 0.5f;
	Vector4 support = Vector4(p_local_direction.x, 0.0f, p_local_direction.z, p_local_direction.w).normalized() * _radius;
	support.y = (p_local_direction.y > 0.0f) ? half_height : -half_height;
	return support;
}

real_t CylinderShape4D::get_surface_volume() const {
	return (8.0 * Math_PI / 3.0) * (_radius * _radius * _radius) + (4.0 * Math_PI) * (_radius * _radius * _height);
}

bool CylinderShape4D::has_point(const Vector4 &p_local_point) const {
	Vector4 abs_point = p_local_point.abs();
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
