#include "shape_4d.h"

#include "../../mesh/tetra/tetra_mesh_4d.h"
#include "../../mesh/wire/wire_mesh_4d.h"

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
	ClassDB::bind_method(D_METHOD("has_point", "point"), &Shape4D::has_point);
	ClassDB::bind_method(D_METHOD("to_tetra_mesh"), &Shape4D::to_tetra_mesh);
	ClassDB::bind_method(D_METHOD("to_wire_mesh"), &Shape4D::to_wire_mesh);

	GDVIRTUAL_BIND(_has_point, "point");
	GDVIRTUAL_BIND(_to_tetra_mesh);
	GDVIRTUAL_BIND(_to_wire_mesh);
}
