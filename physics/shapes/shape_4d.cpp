#include "shape_4d.h"

#include "../../mesh/tetra/tetra_mesh_4d.h"
#include "../../mesh/wire/wire_mesh_4d.h"

real_t Shape4D::get_hypervolume() const {
	real_t bulk = 0.0;
	GDVIRTUAL_CALL(_get_hypervolume, bulk);
	return bulk;
}

Vector4 Shape4D::get_nearest_point(const Vector4 &p_point) const {
	Vector4 nearest_point;
	GDVIRTUAL_CALL(_get_nearest_point, p_point, nearest_point);
	return nearest_point;
}

Vector4 Shape4D::get_support_point(const Vector4 &p_direction) const {
	Vector4 support_point;
	GDVIRTUAL_CALL(_get_support_point, p_direction, support_point);
	return support_point;
}

real_t Shape4D::get_surface_volume() const {
	real_t surface_volume = 0.0;
	GDVIRTUAL_CALL(_get_surface_volume, surface_volume);
	return surface_volume;
}

bool Shape4D::has_point(const Vector4 &p_point) const {
	bool has = false;
	GDVIRTUAL_CALL(_has_point, p_point, has);
	return has;
}

Ref<TetraMesh4D> Shape4D::to_tetra_mesh() const {
	Ref<TetraMesh4D> tetra_mesh;
	GDVIRTUAL_CALL(_to_tetra_mesh, tetra_mesh);
	return tetra_mesh;
}

Ref<WireMesh4D> Shape4D::to_wire_mesh() const {
	Ref<WireMesh4D> wire_mesh;
	GDVIRTUAL_CALL(_to_wire_mesh, wire_mesh);
	return wire_mesh;
}

void Shape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_hypervolume"), &Shape4D::get_hypervolume);
	ClassDB::bind_method(D_METHOD("get_nearest_point", "point"), &Shape4D::get_nearest_point);
	ClassDB::bind_method(D_METHOD("get_support_point", "direction"), &Shape4D::get_support_point);
	ClassDB::bind_method(D_METHOD("get_surface_volume"), &Shape4D::get_surface_volume);
	ClassDB::bind_method(D_METHOD("has_point", "point"), &Shape4D::has_point);
	ClassDB::bind_method(D_METHOD("to_tetra_mesh"), &Shape4D::to_tetra_mesh);
	ClassDB::bind_method(D_METHOD("to_wire_mesh"), &Shape4D::to_wire_mesh);
	// Bind the virtual methods to allow users to make their own shapes.
	GDVIRTUAL_BIND(_get_hypervolume);
	GDVIRTUAL_BIND(_get_nearest_point, "point");
	GDVIRTUAL_BIND(_get_support_point, "direction");
	GDVIRTUAL_BIND(_get_surface_volume);
	GDVIRTUAL_BIND(_has_point, "point");
	GDVIRTUAL_BIND(_to_tetra_mesh);
	GDVIRTUAL_BIND(_to_wire_mesh);
}
