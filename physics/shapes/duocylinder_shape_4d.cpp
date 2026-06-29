#include "duocylinder_shape_4d.h"

real_t DuocylinderShape4D::get_radius_xy() const {
	return _radius_xy;
}

void DuocylinderShape4D::set_radius_xy(const real_t p_radius_xy) {
	_radius_xy = p_radius_xy;
}

real_t DuocylinderShape4D::get_radius_zw() const {
	return _radius_zw;
}

void DuocylinderShape4D::set_radius_zw(const real_t p_radius_zw) {
	_radius_zw = p_radius_zw;
}

real_t DuocylinderShape4D::get_hypervolume() const {
	// http://hi.gher.space/wiki/Duocylinder
	return (Math_PI * Math_PI) * _radius_xy * _radius_xy * _radius_zw * _radius_zw;
}

Vector4 DuocylinderShape4D::get_nearest_point(const Vector4 &p_point) const {
	Vector2 xy = Vector2(p_point.x, p_point.y);
	const real_t xy_len_sq = xy.length_squared();
	if (xy_len_sq > _radius_xy * _radius_xy) {
		xy *= _radius_xy / Math::sqrt(xy_len_sq);
	}
	Vector2 zw = Vector2(p_point.z, p_point.w);
	const real_t zw_len_sq = zw.length_squared();
	if (zw_len_sq > _radius_zw * _radius_zw) {
		zw *= _radius_zw / Math::sqrt(zw_len_sq);
	}
	return Vector4(xy.x, xy.y, zw.x, zw.y);
}

Vector4 DuocylinderShape4D::get_support_point(const Vector4 &p_direction) const {
	const Vector2 support_xy = Vector2(p_direction.x, p_direction.y).normalized() * _radius_xy;
	const Vector2 support_zw = Vector2(p_direction.z, p_direction.w).normalized() * _radius_zw;
	return Vector4(support_xy.x, support_xy.y, support_zw.x, support_zw.y);
}

real_t DuocylinderShape4D::get_surface_volume() const {
	return (2.0 * Math_PI * Math_PI) * _radius_xy * _radius_zw * (_radius_xy + _radius_zw);
}

Dictionary DuocylinderShape4D::raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction) const {
	Dictionary result;
	result["hit"] = false;
	ERR_FAIL_COND_V_MSG(!p_local_direction.is_normalized(), result, "DuocylinderShape4D::raycast_intersects: Ray direction must be normalized.");
	// Perform two ray-circle intersection tests, one for the XY circle and one for the ZW circle.
	Vector4 best_normal = Vector4();
	real_t best_distance = Math_INF;
	// Check XY circle intersection.
	const Vector2 xy_point = Vector2(p_local_from.x, p_local_from.y);
	const Vector2 xy_dir = Vector2(p_local_direction.x, p_local_direction.y);
	const real_t xy_dir_len_sq = xy_dir.length_squared();
	if (xy_dir_len_sq < CMP_EPSILON2) {
		// Ray is parallel to XY plane, check if the starting point is inside the XY circle.
		if (xy_point.length_squared() >= _radius_xy * _radius_xy) {
			return result; // No intersection, ray is outside the XY circle.
		}
	} else {
		// Ray moves along the XY plane, perform ray-circle intersection test.
		// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
		const real_t xy_point_dot_xy_dir = xy_point.dot(xy_dir);
		const real_t xy_point_dot_xy_point = xy_point.dot(xy_point);
		const real_t discriminant_xy = xy_point_dot_xy_dir * xy_point_dot_xy_dir - xy_dir_len_sq * (xy_point_dot_xy_point - _radius_xy * _radius_xy);
		if (discriminant_xy >= 0.0f) {
			const real_t sqrt_discriminant_xy = Math::sqrt(discriminant_xy);
			const real_t xy_distance = (-xy_point_dot_xy_dir - sqrt_discriminant_xy) / xy_dir_len_sq;
			if (xy_distance >= 0.0f) {
				const Vector4 hit_point = p_local_from + p_local_direction * xy_distance;
				const Vector2 zw_check = Vector2(hit_point.z, hit_point.w);
				if (zw_check.length_squared() <= _radius_zw * _radius_zw) {
					if (xy_distance < best_distance) {
						best_distance = xy_distance;
						const Vector2 xy_hit = Vector2(hit_point.x, hit_point.y);
						best_normal = Vector4(xy_hit.normalized().x, xy_hit.normalized().y, 0.0f, 0.0f);
					}
				}
			}
		}
	}
	// Check ZW circle intersection.
	const Vector2 zw_point = Vector2(p_local_from.z, p_local_from.w);
	const Vector2 zw_dir = Vector2(p_local_direction.z, p_local_direction.w);
	const real_t zw_dir_len_sq = zw_dir.length_squared();
	if (zw_dir_len_sq < CMP_EPSILON2) {
		// Ray is parallel to ZW plane, check if the starting point is inside the ZW circle.
		if (zw_point.length_squared() >= _radius_zw * _radius_zw) {
			return result; // No intersection, ray is outside the ZW circle.
		}
	} else {
		// Ray moves along the ZW plane, perform ray-circle intersection test.
		// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
		const real_t zw_point_dot_zw_dir = zw_point.dot(zw_dir);
		const real_t zw_point_dot_zw_point = zw_point.dot(zw_point);
		const real_t discriminant_zw = zw_point_dot_zw_dir * zw_point_dot_zw_dir - zw_dir_len_sq * (zw_point_dot_zw_point - _radius_zw * _radius_zw);
		if (discriminant_zw >= 0.0f) {
			const real_t sqrt_discriminant_zw = Math::sqrt(discriminant_zw);
			const real_t zw_distance = (-zw_point_dot_zw_dir - sqrt_discriminant_zw) / zw_dir_len_sq;
			if (zw_distance >= 0.0f && zw_distance < best_distance) {
				const Vector4 hit_point = p_local_from + p_local_direction * zw_distance;
				const Vector2 xy_check = Vector2(hit_point.x, hit_point.y);
				if (xy_check.length_squared() <= _radius_xy * _radius_xy) {
					best_distance = zw_distance;
					const Vector2 zw_hit = Vector2(hit_point.z, hit_point.w);
					best_normal = Vector4(0.0f, 0.0f, zw_hit.normalized().x, zw_hit.normalized().y);
				}
			}
		}
	}
	if (best_distance < Math_INF) {
		result["hit"] = true;
		result["distance"] = best_distance;
		result["normal"] = best_normal;
	}
	return result;
}

bool DuocylinderShape4D::has_point(const Vector4 &p_point) const {
	const bool in_xy = p_point.x * p_point.x + p_point.y * p_point.y <= _radius_xy * _radius_xy;
	const bool in_zw = p_point.z * p_point.z + p_point.w * p_point.w <= _radius_zw * _radius_zw;
	return in_xy && in_zw;
}

bool DuocylinderShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<DuocylinderShape4D> shape = p_shape;
	if (shape.is_null()) {
		return false;
	}
	return _radius_xy == shape->_radius_xy && _radius_zw == shape->_radius_zw;
}

void DuocylinderShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius_xy"), &DuocylinderShape4D::get_radius_xy);
	ClassDB::bind_method(D_METHOD("set_radius_xy", "radius_xy"), &DuocylinderShape4D::set_radius_xy);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius_xy", PROPERTY_HINT_NONE, "suffix:m"), "set_radius_xy", "get_radius_xy");

	ClassDB::bind_method(D_METHOD("get_radius_zw"), &DuocylinderShape4D::get_radius_zw);
	ClassDB::bind_method(D_METHOD("set_radius_zw", "radius_zw"), &DuocylinderShape4D::set_radius_zw);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius_zw", PROPERTY_HINT_NONE, "suffix:m"), "set_radius_zw", "get_radius_zw");
}
