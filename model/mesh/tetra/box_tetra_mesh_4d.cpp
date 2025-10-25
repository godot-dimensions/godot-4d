#include "box_tetra_mesh_4d.h"

#include "../box_mesh_data.inc"
#include "../wire/box_wire_mesh_4d.h"

void BoxTetraMesh4D::_clear_caches() {
	_cell_positions_cache.clear();
	_vertices_cache.clear();
	tetra_mesh_clear_cache();
}

Vector4 BoxTetraMesh4D::get_half_extents() const {
	return _size * 0.5f;
}

void BoxTetraMesh4D::set_half_extents(const Vector4 &p_half_extents) {
	_size = p_half_extents * 2.0f;
	_clear_caches();
}

Vector4 BoxTetraMesh4D::get_size() const {
	return _size;
}

void BoxTetraMesh4D::set_size(const Vector4 &p_size) {
	_size = p_size;
	_clear_caches();
}

BoxTetraMesh4D::BoxTetraDecomp BoxTetraMesh4D::get_tetra_decomp() const {
	return _tetra_decomp;
}

void BoxTetraMesh4D::set_tetra_decomp(const BoxTetraDecomp p_decomp) {
	_tetra_decomp = p_decomp;
	_clear_caches();
}

void BoxTetraMesh4D::set_cell_texture_map(const BoxCellTextureMap p_map) {
	_cell_texture_map = p_map;
	// Most of the caches can be kept, but the cross section mesh needs to be rebuilt.
	mark_cross_section_mesh_dirty();
}

Ref<ArrayMesh> BoxTetraMesh4D::export_uvw_map_mesh() {
	PackedVector3Array texture_map = get_cell_uvw_map();
	if (_cell_texture_map == BOX_CELL_TEXTURE_MAP_COMPACT_2X2X2_GRID) {
		// Special case: For 3D rendering of the resulting mesh, flip the normals of the negative sides.
		const int64_t map_size = texture_map.size();
		const int64_t side_size = map_size / 8;
		for (int64_t side_offset = 0; side_offset < map_size; side_offset += map_size / 4) {
			for (int64_t i = 1; i < side_size; i += 4) {
				const Vector3 temp = texture_map[side_offset + i];
				texture_map.set(side_offset + i, texture_map[side_offset + i + 1]);
				texture_map.set(side_offset + i + 1, temp);
			}
		}
	}
	return convert_texture_map_to_mesh(texture_map);
}

PackedInt32Array BoxTetraMesh4D::get_cell_indices() {
	return _tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_INDICES : BOX_48_CELL_INDICES;
}

PackedVector4Array BoxTetraMesh4D::get_cell_boundary_normals() {
	return _tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_BOUNDARY_NORMALS : BOX_48_CELL_BOUNDARY_NORMALS;
}

PackedVector4Array BoxTetraMesh4D::get_cell_vertex_normals() {
	return _tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_VERTEX_NORMALS : BOX_48_CELL_VERTEX_NORMALS;
}

PackedVector3Array BoxTetraMesh4D::get_cell_uvw_map() {
	switch (_cell_texture_map) {
		case BOX_CELL_TEXTURE_MAP_CROSS_ISLAND: {
			return _tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_TEXTURE_MAP_CROSS_ISLAND : BOX_48_CELL_TEXTURE_MAP_CROSS_ISLAND;
		} break;
		case BOX_CELL_TEXTURE_MAP_FILL_EACH_SIDE: {
			return _tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_TEXTURE_MAP_FILL_EACH_SIDE : BOX_48_CELL_TEXTURE_MAP_FILL_EACH_SIDE;
		} break;
		case BOX_CELL_TEXTURE_MAP_COMPACT_2X2X2_GRID: {
			return _tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_TEXTURE_MAP_COMPACT_2X2X2_GRID : BOX_48_CELL_TEXTURE_MAP_COMPACT_2X2X2_GRID;
		} break;
		case BOX_CELL_TEXTURE_MAP_LONG_CROSS: {
			return _tetra_decomp == BOX_TETRA_DECOMP_40_CELL ? BOX_40_CELL_TEXTURE_MAP_LONG_CROSS : BOX_48_CELL_TEXTURE_MAP_LONG_CROSS;
		} break;
	}
	ERR_FAIL_V_MSG(PackedVector3Array(), "Invalid cell texture map type.");
}

PackedInt32Array BoxTetraMesh4D::get_edge_indices() {
	if (_tetra_decomp == BOX_TETRA_DECOMP_48_CELL_POLYTOPE) {
		return BOX_EDGE_INDICES;
	}
	return TetraMesh4D::get_edge_indices();
}

PackedVector4Array BoxTetraMesh4D::get_vertices() {
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

Ref<BoxTetraMesh4D> BoxTetraMesh4D::from_box_wire_mesh(const Ref<BoxWireMesh4D> &p_wire_mesh) {
	Ref<BoxTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(p_wire_mesh->get_size());
	tetra_mesh->set_material(p_wire_mesh->get_material());
	return tetra_mesh;
}

Ref<BoxWireMesh4D> BoxTetraMesh4D::to_box_wire_mesh() const {
	Ref<BoxWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	wire_mesh->set_material(get_material());
	return wire_mesh;
}

Ref<TetraMesh4D> BoxTetraMesh4D::to_tetra_mesh() {
	Ref<BoxTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(_size);
	tetra_mesh->set_material(get_material());
	return tetra_mesh;
}

Ref<WireMesh4D> BoxTetraMesh4D::to_wire_mesh() {
	return to_box_wire_mesh();
}

void BoxTetraMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &BoxTetraMesh4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &BoxTetraMesh4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &BoxTetraMesh4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxTetraMesh4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");

	ClassDB::bind_method(D_METHOD("get_tetra_decomp"), &BoxTetraMesh4D::get_tetra_decomp);
	ClassDB::bind_method(D_METHOD("set_tetra_decomp", "decomp"), &BoxTetraMesh4D::set_tetra_decomp);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tetra_decomp", PROPERTY_HINT_ENUM, "40 Cell,48 Cell,48 Cell Polytope"), "set_tetra_decomp", "get_tetra_decomp");

	ClassDB::bind_method(D_METHOD("get_cell_texture_map"), &BoxTetraMesh4D::get_cell_texture_map);
	ClassDB::bind_method(D_METHOD("set_cell_texture_map", "texture_map"), &BoxTetraMesh4D::set_cell_texture_map);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cell_texture_map", PROPERTY_HINT_ENUM, "Cross and Island,Fill Each Side,Compact 2x2x2 Grid,Long Cross"), "set_cell_texture_map", "get_cell_texture_map");

	ClassDB::bind_static_method("BoxTetraMesh4D", D_METHOD("from_box_wire_mesh", "wire_mesh"), &BoxTetraMesh4D::from_box_wire_mesh);
	ClassDB::bind_method(D_METHOD("to_box_wire_mesh"), &BoxTetraMesh4D::to_box_wire_mesh);

	BIND_ENUM_CONSTANT(BOX_TETRA_DECOMP_40_CELL);
	BIND_ENUM_CONSTANT(BOX_TETRA_DECOMP_48_CELL);
	BIND_ENUM_CONSTANT(BOX_TETRA_DECOMP_48_CELL_POLYTOPE);

	BIND_ENUM_CONSTANT(BOX_CELL_TEXTURE_MAP_CROSS_ISLAND);
	BIND_ENUM_CONSTANT(BOX_CELL_TEXTURE_MAP_FILL_EACH_SIDE);
	BIND_ENUM_CONSTANT(BOX_CELL_TEXTURE_MAP_COMPACT_2X2X2_GRID);
	BIND_ENUM_CONSTANT(BOX_CELL_TEXTURE_MAP_LONG_CROSS);
}
