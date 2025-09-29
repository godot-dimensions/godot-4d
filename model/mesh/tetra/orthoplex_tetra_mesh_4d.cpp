#include "orthoplex_tetra_mesh_4d.h"

#include "../orthoplex_mesh_data.inc"
#include "../wire/orthoplex_wire_mesh_4d.h"

void OrthoplexTetraMesh4D::_clear_caches() {
	_cell_positions_cache.clear();
	_vertices_cache.clear();
	tetra_mesh_clear_cache();
}

Vector4 OrthoplexTetraMesh4D::get_half_extents() const {
	return _size * 0.5f;
}

void OrthoplexTetraMesh4D::set_half_extents(const Vector4 &p_half_extents) {
	_size = p_half_extents * 2.0f;
	_clear_caches();
}

Vector4 OrthoplexTetraMesh4D::get_size() const {
	return _size;
}

void OrthoplexTetraMesh4D::set_size(const Vector4 &p_size) {
	_size = p_size;
	_clear_caches();
}

PackedInt32Array OrthoplexTetraMesh4D::get_cell_indices() {
	return ORTHOPLEX_CELL_INDICES;
}

PackedVector4Array OrthoplexTetraMesh4D::get_cell_face_normals() {
	return ORTHOPLEX_CELL_FACE_NORMALS;
}

PackedVector4Array OrthoplexTetraMesh4D::get_cell_vertex_normals() {
	return ORTHOPLEX_CELL_VERTEX_NORMALS;
}

PackedVector3Array OrthoplexTetraMesh4D::get_cell_uvw_map() {
	return ORTHOPLEX_CELL_UVW_MAP;
}

PackedVector4Array OrthoplexTetraMesh4D::get_vertices() {
	if (_vertices_cache.is_empty()) {
		const Vector4 he = get_half_extents();
		_vertices_cache.append(Vector4(-he.x, 0.0f, 0.0f, 0.0f));
		_vertices_cache.append(Vector4(+he.x, 0.0f, 0.0f, 0.0f));
		_vertices_cache.append(Vector4(0.0f, -he.y, 0.0f, 0.0f));
		_vertices_cache.append(Vector4(0.0f, +he.y, 0.0f, 0.0f));
		_vertices_cache.append(Vector4(0.0f, 0.0f, -he.z, 0.0f));
		_vertices_cache.append(Vector4(0.0f, 0.0f, +he.z, 0.0f));
		_vertices_cache.append(Vector4(0.0f, 0.0f, 0.0f, -he.w));
		_vertices_cache.append(Vector4(0.0f, 0.0f, 0.0f, +he.w));
	}
	return _vertices_cache;
}

Ref<OrthoplexTetraMesh4D> OrthoplexTetraMesh4D::from_orthoplex_wire_mesh(const Ref<OrthoplexWireMesh4D> &p_wire_mesh) {
	Ref<OrthoplexTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(p_wire_mesh->get_size());
	tetra_mesh->set_material(p_wire_mesh->get_material());
	return tetra_mesh;
}

Ref<OrthoplexWireMesh4D> OrthoplexTetraMesh4D::to_orthoplex_wire_mesh() const {
	Ref<OrthoplexWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

Ref<WireMesh4D> OrthoplexTetraMesh4D::to_wire_mesh() {
	return to_orthoplex_wire_mesh();
}

void OrthoplexTetraMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &OrthoplexTetraMesh4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &OrthoplexTetraMesh4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &OrthoplexTetraMesh4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &OrthoplexTetraMesh4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");

	ClassDB::bind_static_method("OrthoplexTetraMesh4D", D_METHOD("from_orthoplex_wire_mesh", "wire_mesh"), &OrthoplexTetraMesh4D::from_orthoplex_wire_mesh);
	ClassDB::bind_method(D_METHOD("to_orthoplex_wire_mesh"), &OrthoplexTetraMesh4D::to_orthoplex_wire_mesh);
}
