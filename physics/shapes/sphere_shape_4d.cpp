#include "sphere_shape_4d.h"

real_t SphereShape4D::get_radius() const {
	return _radius;
}

void SphereShape4D::set_radius(const real_t p_radius) {
	_radius = p_radius;
}

real_t SphereShape4D::get_hypervolume() const {
	return (0.125 * Math_TAU * Math_TAU) * (_radius * _radius * _radius * _radius);
}

Vector4 SphereShape4D::get_nearest_point(const Vector4 &p_point) const {
	const real_t length_sq = p_point.length_squared();
	if (length_sq <= _radius * _radius) {
		return p_point;
	}
	return p_point * (_radius / Math::sqrt(length_sq));
}

Vector4 SphereShape4D::get_support_point(const Vector4 &p_direction) const {
	return p_direction.normalized() * _radius;
}

real_t SphereShape4D::get_surface_volume() const {
	return (0.5 * Math_TAU * Math_TAU) * (_radius * _radius * _radius);
}

bool SphereShape4D::has_point(const Vector4 &p_point) const {
	return p_point.length_squared() <= _radius * _radius;
}

bool SphereShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<SphereShape4D> sphere_shape = p_shape;
	if (sphere_shape.is_null()) {
		return false;
	}
	return _radius == sphere_shape->get_radius();
}

void SphereShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius"), &SphereShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SphereShape4D::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_NONE, "suffix:m"), "set_radius", "get_radius");
}
