#include "box_shape_4d.h"

#include "../../model/mesh/tetra/box_tetra_mesh_4d.h"
#include "../../model/mesh/wire/box_wire_mesh_4d.h"

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

real_t BoxShape4D::get_hypervolume() const {
	return _size.x * _size.y * _size.z * _size.w;
}

real_t BoxShape4D::get_surface_volume() const {
	return 2.0f * (_size.x * _size.y * _size.z + _size.x * _size.y * _size.w + _size.x * _size.z * _size.w + _size.y * _size.z * _size.w);
}

Rect4 BoxShape4D::get_rect_bounds(const Transform4D &p_to_target) const {
	Rect4 bounds = Rect4(p_to_target.origin, Vector4());
	const Vector4 half_extents = get_half_extents();
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 2; z++) {
				for (int w = 0; w < 2; w++) {
					const Vector4 point = Vector4(
							x == 0 ? -half_extents.x : half_extents.x,
							y == 0 ? -half_extents.y : half_extents.y,
							z == 0 ? -half_extents.z : half_extents.z,
							w == 0 ? -half_extents.w : half_extents.w);
					bounds = bounds.expand_to_point(p_to_target * point);
				}
			}
		}
	}
	return bounds;
}

Vector4 BoxShape4D::get_nearest_point(const Vector4 &p_point) const {
	const Vector4 half_extents = get_half_extents();
	return Vector4(
			CLAMP(p_point.x, -half_extents.x, half_extents.x),
			CLAMP(p_point.y, -half_extents.y, half_extents.y),
			CLAMP(p_point.z, -half_extents.z, half_extents.z),
			CLAMP(p_point.w, -half_extents.w, half_extents.w));
}

Vector4 BoxShape4D::get_support_point(const Vector4 &p_direction) const {
	const Vector4 half_extents = get_half_extents();
	return Vector4(
			(p_direction.x > 0.0f) ? half_extents.x : -half_extents.x,
			(p_direction.y > 0.0f) ? half_extents.y : -half_extents.y,
			(p_direction.z > 0.0f) ? half_extents.z : -half_extents.z,
			(p_direction.w > 0.0f) ? half_extents.w : -half_extents.w);
}

bool BoxShape4D::has_point(const Vector4 &p_point) const {
	const Vector4 abs_point = p_point.abs();
	const Vector4 half_extents = get_half_extents();
	return abs_point.x <= half_extents.x && abs_point.y <= half_extents.y && abs_point.z <= half_extents.z && abs_point.w <= half_extents.w;
}

bool BoxShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<BoxShape4D> box_shape = p_shape;
	if (box_shape.is_null()) {
		return false;
	}
	return _size == box_shape->get_size();
}

Ref<TetraMesh4D> BoxShape4D::to_tetra_mesh() const {
	Ref<BoxTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(_size);
	return tetra_mesh;
}

Ref<WireMesh4D> BoxShape4D::to_wire_mesh() const {
	Ref<BoxWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	return wire_mesh;
}

void BoxShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &BoxShape4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &BoxShape4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &BoxShape4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxShape4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");
}
