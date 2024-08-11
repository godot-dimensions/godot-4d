#include "orthoplex_shape_4d.h"

#include "../../math/vector_4d.h"
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

real_t OrthoplexShape4D::get_hypervolume() const {
	// http://hi.gher.space/wiki/16-cell
	// Bulk of regular orthoplex is (1/6) * edge_length ^ 4, but we have size instead of edge length.
	// To convert we need to divide by sqrt(2) 4 times, so 1/4, so (1/6)*(1/4) becomes 1/24.
	return (1.0 / 24.0) * _size.x * _size.y * _size.z * _size.w;
}

Vector4 OrthoplexShape4D::get_nearest_point(const Vector4 &p_point) const {
	return Vector4D::limit_length_taxicab(p_point / _size, 0.5) * _size;
}

Vector4 OrthoplexShape4D::get_support_point(const Vector4 &p_direction) const {
	const Vector4 abs_dir = p_direction.abs();
	const Vector4::Axis longest_axis = abs_dir.max_axis_index();
	Vector4 support = Vector4();
	support[longest_axis] = (p_direction[longest_axis] > 0.0f) ? _size[longest_axis] * 0.5f : -_size[longest_axis] * 0.5f;
	return support;
}

real_t OrthoplexShape4D::get_surface_volume() const {
	// Surcell volume of regular orthoplex is (4*sqrt(2)/3) * edge_length ^ 3, but we have size instead of edge length.
	// To convert we need to divide by sqrt(2) 3 times, so 1/(2*sqrt(2)), the sqrt(2)s cancel out, so (4/3)*(1/2) = 2/3.
	// Then since we have each axis separate, we need to add a quarter 4 times, so (2/3)*(1/4) = 1/6.
	return (1.0 / 6.0) * (_size.x * _size.y * _size.z + _size.x * _size.y * _size.w + _size.x * _size.z * _size.w + _size.y * _size.z * _size.w);
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
