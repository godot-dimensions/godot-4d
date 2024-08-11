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
