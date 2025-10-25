#include "box_poly_mesh_4d.h"

#include "../box_mesh_data.inc"
#include "../tetra/box_tetra_mesh_4d.h"
#include "../wire/box_wire_mesh_4d.h"

void BoxPolyMesh4D::_clear_caches() {
	_cell_positions_cache.clear();
	_vertices_cache.clear();
	poly_mesh_clear_cache();
}

Vector4 BoxPolyMesh4D::get_half_extents() const {
	return _size * 0.5f;
}

void BoxPolyMesh4D::set_half_extents(const Vector4 &p_half_extents) {
	_size = p_half_extents * 2.0f;
	_clear_caches();
}

Vector4 BoxPolyMesh4D::get_size() const {
	return _size;
}

void BoxPolyMesh4D::set_size(const Vector4 &p_size) {
	_size = p_size;
	_clear_caches();
}

void BoxPolyMesh4D::set_poly_texture_map(const BoxPolyTextureMap p_map) {
	_poly_texture_map = p_map;
	// Most of the caches can be kept, but the cross section mesh needs to be rebuilt.
	mark_cross_section_mesh_dirty();
}

Vector<Vector<PackedInt32Array>> BoxPolyMesh4D::get_poly_cell_indices() {
	return BOX_POLY_CELL_INDICES;
}

PackedVector4Array BoxPolyMesh4D::get_poly_cell_vertices() {
	return get_vertices();
}

PackedVector4Array BoxPolyMesh4D::get_poly_cell_boundary_normals() {
	return BOX_POLY_CELL_BOUNDARY_NORMALS;
}

Vector<PackedVector4Array> BoxPolyMesh4D::get_poly_cell_vertex_normals() {
	return BOX_POLY_CELL_VERTEX_NORMALS;
}

Vector<PackedVector3Array> BoxPolyMesh4D::get_poly_cell_texture_map() {
	switch (_poly_texture_map) {
		case BOX_POLY_TEXTURE_MAP_CROSS_ISLAND: {
			return BOX_POLY_CELL_POLY_TEXTURE_MAP_CROSS_ISLAND;
		} break;
		case BOX_POLY_TEXTURE_MAP_FILL_EACH_SIDE: {
			return BOX_POLY_CELL_POLY_TEXTURE_MAP_FILL_EACH_SIDE;
		} break;
		case BOX_POLY_TEXTURE_MAP_COMPACT_2X2X2_GRID: {
			return BOX_POLY_CELL_POLY_TEXTURE_MAP_COMPACT_2X2X2_GRID;
		} break;
		case BOX_POLY_TEXTURE_MAP_LONG_CROSS: {
			return BOX_POLY_CELL_POLY_TEXTURE_MAP_LONG_CROSS;
		} break;
	}
	ERR_FAIL_V_MSG(Vector<PackedVector3Array>(), "BoxPolyMesh4D: Invalid cell texture map type.");
}

PackedInt32Array BoxPolyMesh4D::get_edge_indices() {
	return BOX_EDGE_INDICES;
}

PackedVector4Array BoxPolyMesh4D::get_vertices() {
	if (_vertices_cache.is_empty()) {
		const Vector4 he = get_half_extents();
		_vertices_cache.append(Vector4(-he.x, -he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, -he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, +he.z, -he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, -he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, -he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, -he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(-he.x, +he.y, +he.z, +he.w));
		_vertices_cache.append(Vector4(+he.x, +he.y, +he.z, +he.w));
	}
	return _vertices_cache;
}

Ref<BoxPolyMesh4D> BoxPolyMesh4D::from_box_tetra_mesh(const Ref<BoxTetraMesh4D> &p_tetra_mesh) {
	Ref<BoxPolyMesh4D> poly_mesh;
	poly_mesh.instantiate();
	poly_mesh->set_size(p_tetra_mesh->get_size());
	poly_mesh->set_material(p_tetra_mesh->get_material());
	return poly_mesh;
}

Ref<BoxPolyMesh4D> BoxPolyMesh4D::from_box_wire_mesh(const Ref<BoxWireMesh4D> &p_wire_mesh) {
	Ref<BoxPolyMesh4D> poly_mesh;
	poly_mesh.instantiate();
	poly_mesh->set_size(p_wire_mesh->get_size());
	poly_mesh->set_material(p_wire_mesh->get_material());
	return poly_mesh;
}

Ref<BoxTetraMesh4D> BoxPolyMesh4D::to_box_tetra_mesh() const {
	Ref<BoxTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(_size);
	tetra_mesh->set_material(get_material());
	return tetra_mesh;
}

Ref<BoxWireMesh4D> BoxPolyMesh4D::to_box_wire_mesh() const {
	Ref<BoxWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

Ref<TetraMesh4D> BoxPolyMesh4D::to_tetra_mesh() {
	return to_box_tetra_mesh();
}

Ref<WireMesh4D> BoxPolyMesh4D::to_wire_mesh() {
	return to_box_wire_mesh();
}

void BoxPolyMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &BoxPolyMesh4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &BoxPolyMesh4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &BoxPolyMesh4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxPolyMesh4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");

	ClassDB::bind_method(D_METHOD("get_poly_texture_map"), &BoxPolyMesh4D::get_poly_texture_map);
	ClassDB::bind_method(D_METHOD("set_poly_texture_map", "texture_map"), &BoxPolyMesh4D::set_poly_texture_map);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "poly_texture_map", PROPERTY_HINT_ENUM, "Cross and Island,Fill Each Side,Compact 2x2x2 Grid,Long Cross"), "set_poly_texture_map", "get_poly_texture_map");

	ClassDB::bind_static_method("BoxPolyMesh4D", D_METHOD("from_box_tetra_mesh", "tetra_mesh"), &BoxPolyMesh4D::from_box_tetra_mesh);
	ClassDB::bind_static_method("BoxPolyMesh4D", D_METHOD("from_box_wire_mesh", "wire_mesh"), &BoxPolyMesh4D::from_box_wire_mesh);
	ClassDB::bind_method(D_METHOD("to_box_tetra_mesh"), &BoxPolyMesh4D::to_box_tetra_mesh);
	ClassDB::bind_method(D_METHOD("to_box_wire_mesh"), &BoxPolyMesh4D::to_box_wire_mesh);

	BIND_ENUM_CONSTANT(BOX_POLY_TEXTURE_MAP_CROSS_ISLAND);
	BIND_ENUM_CONSTANT(BOX_POLY_TEXTURE_MAP_FILL_EACH_SIDE);
	BIND_ENUM_CONSTANT(BOX_POLY_TEXTURE_MAP_COMPACT_2X2X2_GRID);
	BIND_ENUM_CONSTANT(BOX_POLY_TEXTURE_MAP_LONG_CROSS);
}
