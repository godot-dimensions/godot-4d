#include "concave_mesh_shape_4d.h"

#include "../../math/vector_4d.h"
#include "../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../model/mesh/wire/array_wire_mesh_4d.h"

void ConcaveMeshShape4D::_calculate_normals() {
	const int64_t normal_count = _cells.size() / 4;
	_normals.resize(normal_count);
	for (int64_t i = 0; i < normal_count; i++) {
		const Vector4 pivot = _cells[i * 4];
		const Vector4 a = _cells[1 + i * 4];
		const Vector4 b = _cells[2 + i * 4];
		const Vector4 c = _cells[3 + i * 4];
		const Vector4 perp = Vector4D::perpendicular(a - pivot, b - pivot, c - pivot);
		_normals.set(i, perp);
	}
}

void ConcaveMeshShape4D::set_cells(const PackedVector4Array &p_cells) {
	ERR_FAIL_COND_MSG(p_cells.size() % 4 != 0, "ConcaveMeshShape4D: Cells must be a multiple of 4 (every 4 vertices form a cell).");
	_cells = p_cells;
	_calculate_normals();
}

bool ConcaveMeshShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<ConcaveMeshShape4D> other_shape = p_shape;
	if (other_shape.is_null()) {
		return false;
	}
	const PackedVector4Array other_cells = other_shape->get_cells();
	return _cells == other_cells;
}

Ref<TetraMesh4D> ConcaveMeshShape4D::to_tetra_mesh() const {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	mesh->append_vertices(_cells);
	return mesh;
}

Ref<WireMesh4D> ConcaveMeshShape4D::to_wire_mesh() const {
	return to_tetra_mesh()->to_wire_mesh();
}

Ref<ConcaveMeshShape4D> ConcaveMeshShape4D::create_from_mesh(const Ref<TetraMesh4D> &p_mesh) {
	Ref<ConcaveMeshShape4D> shape;
	shape.instantiate();
	shape->set_cells(p_mesh->get_cell_positions());
	return shape;
}

void ConcaveMeshShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_cells"), &ConcaveMeshShape4D::get_cells);
	ClassDB::bind_method(D_METHOD("set_cells", "cells"), &ConcaveMeshShape4D::set_cells);

	ClassDB::bind_static_method("ConcaveMeshShape4D", D_METHOD("create_from_mesh", "mesh"), &ConcaveMeshShape4D::create_from_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "cells", PROPERTY_HINT_NONE, "suffix:m"), "set_cells", "get_cells");
}
