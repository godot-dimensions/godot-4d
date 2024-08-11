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

bool DuocylinderShape4D::has_point(const Vector4 &p_point) const {
	const bool in_xy = p_point.x * p_point.x + p_point.y * p_point.y <= _radius_xy * _radius_xy;
	const bool in_zw = p_point.z * p_point.z + p_point.w * p_point.w <= _radius_zw * _radius_zw;
	return in_xy && in_zw;
}

void DuocylinderShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius_xy"), &DuocylinderShape4D::get_radius_xy);
	ClassDB::bind_method(D_METHOD("set_radius_xy", "radius_xy"), &DuocylinderShape4D::set_radius_xy);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius_xy", PROPERTY_HINT_NONE, "suffix:m"), "set_radius_xy", "get_radius_xy");

	ClassDB::bind_method(D_METHOD("get_radius_zw"), &DuocylinderShape4D::get_radius_zw);
	ClassDB::bind_method(D_METHOD("set_radius_zw", "radius_zw"), &DuocylinderShape4D::set_radius_zw);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius_zw", PROPERTY_HINT_NONE, "suffix:m"), "set_radius_zw", "get_radius_zw");
}
