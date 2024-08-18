#include "orthoplex_shape_4d.h"

#include "../../mesh/tetra/orthoplex_tetra_mesh_4d.h"
#include "../../mesh/wire/orthoplex_wire_mesh_4d.h"

Vector4 OrthoplexShape4D::get_half_extents() const {
	return _size * 0.5f;
}

void OrthoplexShape4D::set_half_extents(const Vector4 &p_half_extents) {
	_size = p_half_extents * 2.0f;
}

Vector4 OrthoplexShape4D::get_size() const {
	return _size;
}

void OrthoplexShape4D::set_size(const Vector4 &p_size) {
	_size = p_size;
}

bool OrthoplexShape4D::has_point(const Vector4 &p_point) const {
	const Vector4 abs_scaled_point = p_point.abs() / _size;
	return (abs_scaled_point.x + abs_scaled_point.y + abs_scaled_point.z + abs_scaled_point.w) <= 1.0f;
}

Ref<TetraMesh4D> OrthoplexShape4D::to_tetra_mesh() const {
	Ref<OrthoplexTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(_size);
	return tetra_mesh;
}

Ref<WireMesh4D> OrthoplexShape4D::to_wire_mesh() const {
	Ref<OrthoplexWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	return wire_mesh;
}

void OrthoplexShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &OrthoplexShape4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &OrthoplexShape4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &OrthoplexShape4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &OrthoplexShape4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");
}
