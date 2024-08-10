#include "sphere_shape_4d.h"

real_t SphereShape4D::get_radius() const {
	return _radius;
}

void SphereShape4D::set_radius(const real_t p_radius) {
	_radius = p_radius;
}

bool SphereShape4D::has_point(const Vector4 &p_point) const {
	return p_point.length_squared() <= _radius * _radius;
}

void SphereShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius"), &SphereShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SphereShape4D::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_NONE, "suffix:m"), "set_radius", "get_radius");
}
