#include "orthoplex_poly_mesh_4d.h"

#include "../orthoplex_mesh_data.inc"
#include "../tetra/orthoplex_tetra_mesh_4d.h"
#include "../wire/orthoplex_wire_mesh_4d.h"

void OrthoplexPolyMesh4D::_clear_caches() {
	_simplex_positions_cache.clear();
	_vertices_cache.clear();
	poly_mesh_clear_cache();
}

Vector4 OrthoplexPolyMesh4D::get_half_extents() const {
	return _size * 0.5f;
}

void OrthoplexPolyMesh4D::set_half_extents(const Vector4 &p_half_extents) {
	_size = p_half_extents * 2.0f;
	_clear_caches();
}

Vector4 OrthoplexPolyMesh4D::get_size() const {
	return _size;
}

void OrthoplexPolyMesh4D::set_size(const Vector4 &p_size) {
	_size = p_size;
	_clear_caches();
}

Vector<Vector<PackedInt32Array>> OrthoplexPolyMesh4D::get_poly_cell_indices() {
	return ORTHOPLEX_POLY_CELL_INDICES;
}

PackedVector4Array OrthoplexPolyMesh4D::get_poly_cell_vertices() {
	return get_vertices();
}

PackedVector4Array OrthoplexPolyMesh4D::get_poly_cell_boundary_normals() {
	return ORTHOPLEX_CELL_BOUNDARY_NORMALS;
}

Vector<PackedVector4Array> OrthoplexPolyMesh4D::get_poly_cell_vertex_normals() {
	return ORTHOPLEX_POLY_CELL_VERTEX_NORMALS;
}

Vector<PackedVector3Array> OrthoplexPolyMesh4D::get_poly_cell_texture_map() {
	return ORTHOPLEX_POLY_CELL_POLY_TEXTURE_MAP;
}

PackedInt32Array OrthoplexPolyMesh4D::get_edge_indices() {
	return ORTHOPLEX_EDGE_INDICES;
}

PackedVector4Array OrthoplexPolyMesh4D::get_vertices() {
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

Ref<OrthoplexPolyMesh4D> OrthoplexPolyMesh4D::from_orthoplex_tetra_mesh(const Ref<OrthoplexTetraMesh4D> &p_tetra_mesh) {
	Ref<OrthoplexPolyMesh4D> poly_mesh;
	poly_mesh.instantiate();
	poly_mesh->set_size(p_tetra_mesh->get_size());
	poly_mesh->set_material(p_tetra_mesh->get_material());
	return poly_mesh;
}

Ref<OrthoplexPolyMesh4D> OrthoplexPolyMesh4D::from_orthoplex_wire_mesh(const Ref<OrthoplexWireMesh4D> &p_wire_mesh) {
	Ref<OrthoplexPolyMesh4D> poly_mesh;
	poly_mesh.instantiate();
	poly_mesh->set_size(p_wire_mesh->get_size());
	poly_mesh->set_material(p_wire_mesh->get_material());
	return poly_mesh;
}

Ref<OrthoplexTetraMesh4D> OrthoplexPolyMesh4D::to_orthoplex_tetra_mesh() const {
	Ref<OrthoplexTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(_size);
	tetra_mesh->set_material(get_material());
	return tetra_mesh;
}

Ref<OrthoplexWireMesh4D> OrthoplexPolyMesh4D::to_orthoplex_wire_mesh() const {
	Ref<OrthoplexWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

Ref<TetraMesh4D> OrthoplexPolyMesh4D::to_tetra_mesh() {
	return to_orthoplex_tetra_mesh();
}

Ref<WireMesh4D> OrthoplexPolyMesh4D::to_wire_mesh() {
	return to_orthoplex_wire_mesh();
}

void OrthoplexPolyMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &OrthoplexPolyMesh4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &OrthoplexPolyMesh4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &OrthoplexPolyMesh4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &OrthoplexPolyMesh4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");

	ClassDB::bind_static_method("OrthoplexPolyMesh4D", D_METHOD("from_orthoplex_tetra_mesh", "tetra_mesh"), &OrthoplexPolyMesh4D::from_orthoplex_tetra_mesh);
	ClassDB::bind_static_method("OrthoplexPolyMesh4D", D_METHOD("from_orthoplex_wire_mesh", "wire_mesh"), &OrthoplexPolyMesh4D::from_orthoplex_wire_mesh);
	ClassDB::bind_method(D_METHOD("to_orthoplex_tetra_mesh"), &OrthoplexPolyMesh4D::to_orthoplex_tetra_mesh);
	ClassDB::bind_method(D_METHOD("to_orthoplex_wire_mesh"), &OrthoplexPolyMesh4D::to_orthoplex_wire_mesh);
}
