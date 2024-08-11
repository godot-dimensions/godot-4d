#include "box_shape_4d.h"

Vector4 BoxShape4D::get_half_extents() const {
	return _size * 0.5f;
}

void BoxShape4D::set_half_extents(const Vector4 &p_half_extents) {
	_size = p_half_extents * 2.0f;
}

Vector4 BoxShape4D::get_size() const {
	return _size;
}

void BoxShape4D::set_size(const Vector4 &p_size) {
	_size = p_size;
}

bool BoxShape4D::has_point(const Vector4 &p_point) const {
	const Vector4 abs_point = p_point.abs();
	const Vector4 half_extents = get_half_extents();
	return abs_point.x <= half_extents.x && abs_point.y <= half_extents.y && abs_point.z <= half_extents.z && abs_point.w <= half_extents.w;
}

void BoxShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &BoxShape4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &BoxShape4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &BoxShape4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxShape4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");
}
