#include "g4mf_mesh_surface_4d.h"

#include "../../mesh/tetra/box_tetra_mesh_4d.h"
#include "../g4mf_state_4d.h"

G4MFMeshSurface4D::MeshSurfaceFormat G4MFMeshSurface4D::_get_compatible_mesh_surface_format(MeshSurfaceFormat p_preferred_mesh_surface_format) const {
	if (p_preferred_mesh_surface_format == MESH_SURFACE_FORMAT_POLYTOPE) {
		// Poly mesh data is populated by edges and geometry (polytopes). Prefer generating
		// poly meshes only when the geometry is defined, because otherwise the poly mesh
		// would just be wireframe, therefore we may as well generate wire meshes instead.
		if (get_geometry_accessor_indices().is_empty()) {
			p_preferred_mesh_surface_format = MESH_SURFACE_FORMAT_TETRAHEDRAL;
		}
	}
	if (p_preferred_mesh_surface_format == MESH_SURFACE_FORMAT_TETRAHEDRAL) {
		// Tetra mesh data is populated by simplexes (tetrahedra). However, we can also
		// decompose tetrahedra from geometry (polyhedra) if simplexes are not available,
		// but only if the dimension includes 3D cells (those which bound a 4D mesh).
		// Index 0 holds 2D faces made of 1D edges, and index 1 holds 3D cells made of those 2D faces.
		if (get_simplexes_accessor_index() < 0 && get_geometry_accessor_indices().size() < 2) {
			p_preferred_mesh_surface_format = MESH_SURFACE_FORMAT_WIREFRAME;
		}
	}
	return p_preferred_mesh_surface_format;
}

bool G4MFMeshSurface4D::is_equal_exact(const Ref<G4MFMeshSurface4D> &p_other) const {
	if (p_other.is_null()) {
		return false;
	}
	if (_edges_accessor_index != p_other->get_edges_accessor_index() ||
			_material_index != p_other->get_material_index() ||
			_seams_accessor_index != p_other->get_seams_accessor_index() ||
			_simplexes_accessor_index != p_other->get_simplexes_accessor_index() ||
			_polytope_simplexes != p_other->get_polytope_simplexes() ||
			_geometry_accessor_indices != p_other->get_geometry_accessor_indices() ||
			_normals_binding.is_valid() != p_other->get_normals_binding().is_valid() ||
			_texture_map_binding.is_valid() != p_other->get_texture_map_binding().is_valid()) {
		return false;
	}
	if (_normals_binding.is_valid() && !_normals_binding->is_equal_exact(p_other->get_normals_binding())) {
		return false;
	}
	if (_texture_map_binding.is_valid() && !_texture_map_binding->is_equal_exact(p_other->get_texture_map_binding())) {
		return false;
	}
	return true;
}

void G4MFMeshSurface4D::convert_separated_geometry_into_packed(const Ref<G4MFState4D> &p_g4mf_state, const Vector<Vector<PackedInt32Array>> &p_separated_geometry, const bool p_deduplicate) {
	const int64_t separated_geometry_size = p_separated_geometry.size();
	_geometry_accessor_indices.clear();
	for (int geom_index = 0; geom_index < separated_geometry_size; geom_index++) {
		const Vector<PackedInt32Array> &separated_geometry_data = p_separated_geometry[geom_index];
		if (separated_geometry_data.is_empty()) {
			return; // Nothing on this dimension, which also implies nothing on the higher dimensions.
		}
		PackedInt32Array packed_geometry_data;
		for (int cell_index = 0; cell_index < separated_geometry_data.size(); cell_index++) {
			const PackedInt32Array &cell_vertex_indices = separated_geometry_data[cell_index];
			packed_geometry_data.append(cell_vertex_indices.size());
			for (int i = 0; i < cell_vertex_indices.size(); i++) {
				packed_geometry_data.append(cell_vertex_indices[i]);
			}
		}
		const int geom_accessor_index = G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, packed_geometry_data, 1, p_deduplicate);
		ERR_FAIL_COND(geom_accessor_index == -1);
		_geometry_accessor_indices.append(geom_accessor_index);
	}
}

void G4MFMeshSurface4D::convert_separated_geometry_into_packed_bind(const Ref<G4MFState4D> &p_g4mf_state, const TypedArray<Array> &p_separated_geometry, const bool p_deduplicate) {
	const int64_t separated_geometry_size = p_separated_geometry.size();
	_geometry_accessor_indices.resize(separated_geometry_size);
	for (int geom_index = 0; geom_index < separated_geometry_size; geom_index++) {
		const Array separated_geometry_data = p_separated_geometry[geom_index];
		PackedInt32Array packed_geometry_data;
		for (int cell_index = 0; cell_index < separated_geometry_data.size(); cell_index++) {
			const PackedInt32Array cell_vertex_indices = separated_geometry_data[cell_index];
			packed_geometry_data.append(cell_vertex_indices.size());
			for (int i = 0; i < cell_vertex_indices.size(); i++) {
				packed_geometry_data.append(cell_vertex_indices[i]);
			}
		}
		const int geom_accessor_index = G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, packed_geometry_data, 1, p_deduplicate);
		ERR_FAIL_COND(geom_accessor_index == -1);
		_geometry_accessor_indices.set(geom_index, geom_accessor_index);
	}
}

Vector<Vector<PackedInt32Array>> G4MFMeshSurface4D::load_geometry_separated(const Ref<G4MFState4D> &p_g4mf_state) const {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), Vector<Vector<PackedInt32Array>>());
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	Vector<Vector<PackedInt32Array>> all_separated;
	all_separated.resize(_geometry_accessor_indices.size());
	for (int geom_index = 0; geom_index < _geometry_accessor_indices.size(); geom_index++) {
		const int accessor_index = _geometry_accessor_indices[geom_index];
		ERR_FAIL_INDEX_V(accessor_index, state_accessors.size(), Vector<Vector<PackedInt32Array>>());
		const Ref<G4MFAccessor4D> accessor = state_accessors[accessor_index];
		ERR_FAIL_COND_V(accessor.is_null(), Vector<Vector<PackedInt32Array>>());
		const PackedInt32Array packed_geometry_data = accessor->decode_int32s_from_bytes(p_g4mf_state);
		const int64_t packed_geometry_data_size = packed_geometry_data.size();
		Vector<PackedInt32Array> separated_geometry_data;
		int64_t cell_start = 0;
		while (cell_start < packed_geometry_data_size) {
			const int cell_vertex_count = packed_geometry_data[cell_start];
			ERR_FAIL_COND_V(cell_vertex_count <= 0, Vector<Vector<PackedInt32Array>>());
			ERR_FAIL_COND_V(cell_vertex_count > packed_geometry_data_size - cell_start - 1, Vector<Vector<PackedInt32Array>>());
			cell_start += 1; // Move past the vertex count to the start of the vertex indices.
			PackedInt32Array cell_vertex_indices;
			cell_vertex_indices.resize(cell_vertex_count);
			for (int i = 0; i < cell_vertex_count; i++) {
				cell_vertex_indices.set(i, packed_geometry_data[cell_start + i]);
			}
			separated_geometry_data.append(cell_vertex_indices);
			cell_start += cell_vertex_count;
		}
		all_separated.set(geom_index, separated_geometry_data);
	}
	return all_separated;
}

TypedArray<Array> G4MFMeshSurface4D::load_geometry_separated_bind(const Ref<G4MFState4D> &p_g4mf_state) const {
	const Vector<Vector<PackedInt32Array>> separated = load_geometry_separated(p_g4mf_state);
	TypedArray<Array> all_separated;
	all_separated.resize(separated.size());
	for (int geom_index = 0; geom_index < separated.size(); geom_index++) {
		// This should be TypedArray<PackedInt32Array> but Godot's type system doesn't allow nested typed arrays.
		Array separated_geometry_data;
		for (const PackedInt32Array &cell : separated[geom_index]) {
			separated_geometry_data.append(cell);
		}
		all_separated[geom_index] = separated_geometry_data;
	}
	return all_separated;
}

PackedInt32Array G4MFMeshSurface4D::load_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), PackedInt32Array());
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_edges_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_edges_accessor_index];
	ERR_FAIL_COND_V(accessor.is_null(), PackedInt32Array());
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

PackedInt32Array G4MFMeshSurface4D::load_seam_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), PackedInt32Array());
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_seams_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_seams_accessor_index];
	ERR_FAIL_COND_V(accessor.is_null(), PackedInt32Array());
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

PackedInt32Array G4MFMeshSurface4D::load_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), PackedInt32Array());
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_simplexes_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_simplexes_accessor_index];
	ERR_FAIL_COND_V(accessor.is_null(), PackedInt32Array());
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

bool G4MFMeshSurface4D::_import_decode_geometry_bindings(const Ref<G4MFState4D> &p_g4mf_state, const Ref<G4MFMeshSurfaceBinding4D> &p_binding, const int64_t p_vertex_count, const int64_t p_edge_count, const Vector<Vector<PackedInt32Array>> &p_separated_geometry, const int64_t p_value_count, HashMap<Vector2i, Vector<PackedInt32Array>> &r_indices, const String &p_binding_name) const {
	// G4MF stores indexed values, which is also how the runtime mesh classes store
	// their data, so the values and indices can be loaded with minimal conversion.
	// G4MF allows bindings to be shorter than their geometry item count, with the missing items having no data.
	// The dense layouts are auxiliary bindings, which the mesh classes allow to be short, so they are kept as-is.
	// The hierarchical layout includes the boundary cell vertex bindings, which need one record per cell, so it is padded.
	const TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings = p_binding->get_geometry_bindings();
	for (int bind_geom_index = 0; bind_geom_index < geometry_bindings.size(); bind_geom_index++) {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding = geometry_bindings[bind_geom_index];
		ERR_FAIL_COND_V_MSG(geometry_binding.is_null(), false, "G4MFMeshSurface4D: " + p_binding_name + " binding contains a null geometry binding.");
		const Vector2i key = geometry_binding->get_poly_mesh_key();
		ERR_FAIL_COND_V_MSG(key.x < key.y || key.y < 0, false, "G4MFMeshSurface4D: " + p_binding_name + " geometry binding " + String(key) + " must have a decompose dimension between zero and its geometry dimension.");
		// The number of geometry items of this binding's geometry dimension, used to validate the dense layouts.
		int64_t geometry_item_count = 0;
		if (key.x == 0) {
			geometry_item_count = p_vertex_count;
		} else if (key.x == 1) {
			geometry_item_count = p_edge_count;
		} else if (key.x - 2 < p_separated_geometry.size()) {
			geometry_item_count = p_separated_geometry[key.x - 2].size();
		}
		const PackedInt32Array packed_indices = geometry_binding->load_indices(p_g4mf_state);
		const int64_t packed_count = packed_indices.size();
		Vector<PackedInt32Array> poly_cell_indices;
		if (key.x == key.y || (key.x == 1 && key.y == 0)) {
			// The dense layouts store value indices directly, so check them all up front.
			for (const int32_t value_index : packed_indices) {
				ERR_FAIL_INDEX_V_MSG(value_index, p_value_count, false, "G4MFMeshSurface4D: " + p_binding_name + " geometry binding " + String(key) + " references value " + itos(value_index) + ", but there are only " + itos(p_value_count) + " values.");
			}
		}
		if (key.x == key.y) {
			// Geometry bindings that are not decomposed, meaning "decomposeDimension"
			// is equal to "geometryDimension", are stored as a dense array of indices,
			// where each index corresponds to a geometry item of the specified geometry dimension.
			// There is no need to store an amount of members, because it is always 1.
			// Fewer indices than geometry items is allowed, since trailing items may have no data.
			ERR_FAIL_COND_V_MSG(packed_count > geometry_item_count, false, "G4MFMeshSurface4D: " + p_binding_name + " geometry binding " + String(key) + " has " + itos(packed_count) + " indices, but the surface only has " + itos(geometry_item_count) + " geometry items of that dimension.");
			poly_cell_indices.append(packed_indices);
		} else if (key.x == 1 && key.y == 0) {
			// Geometry bindings referring to vertices of edges, meaning "decomposeDimension"
			// is 0 and "geometryDimension" is 1, are stored as a dense array of indices,
			// where every 2 indices correspond to the 2 vertices of each edge geometry item.
			// There is no need to store an amount of members, because it is always 2.
			// Godot 4D actually stores this as one 2-member array per edge, so expand it.
			// Fewer entries than edges is allowed, since trailing edges may have no data, but each edge needs both vertices.
			ERR_FAIL_COND_V_MSG(packed_count % 2 != 0 || packed_count > geometry_item_count * 2, false, "G4MFMeshSurface4D: " + p_binding_name + " edge vertex geometry binding has " + itos(packed_count) + " indices, but the surface has " + itos(geometry_item_count) + " edges, so it needs an even number of at most " + itos(geometry_item_count * 2) + " indices.");
			for (int64_t edge_start = 0; edge_start < packed_count; edge_start += 2) {
				PackedInt32Array edge_indices;
				edge_indices.append(packed_indices[edge_start]);
				edge_indices.append(packed_indices[edge_start + 1]);
				poly_cell_indices.append(edge_indices);
			}
		} else {
			// In all other cases, geometry binding accessor indices behave the same as
			// the mesh surface's geometry items. Meaning, the first number is the amount
			// of members in the first cell, followed by those members, then the amount
			// of members in the second cell, followed by those members, and so on.
			int64_t packed_index = 0;
			while (packed_index < packed_count) {
				const int64_t member_count = packed_indices[packed_index];
				packed_index++;
				// The count itself was just consumed, so all of the members must fit in what remains.
				ERR_FAIL_COND_V_MSG(member_count < 0 || packed_index + member_count > packed_count, false, "G4MFMeshSurface4D: " + p_binding_name + " geometry binding " + String(key) + " has a malformed or truncated packed cell record.");
				PackedInt32Array cell_indices;
				cell_indices.resize(member_count);
				for (int64_t member_index = 0; member_index < member_count; member_index++) {
					const int32_t value_index = packed_indices[packed_index];
					ERR_FAIL_INDEX_V_MSG(value_index, p_value_count, false, "G4MFMeshSurface4D: " + p_binding_name + " geometry binding " + String(key) + " references value " + itos(value_index) + ", but there are only " + itos(p_value_count) + " values.");
					cell_indices.set(member_index, value_index);
					packed_index++;
				}
				poly_cell_indices.append(cell_indices);
			}
			ERR_FAIL_COND_V_MSG(poly_cell_indices.size() > geometry_item_count, false, "G4MFMeshSurface4D: " + p_binding_name + " geometry binding " + String(key) + " has " + itos(poly_cell_indices.size()) + " cell records, but the surface only has " + itos(geometry_item_count) + " geometry items of that dimension.");
			// Pad the missing cells with empty records, meaning no data, so that bindings which need one record per cell are complete.
			if (poly_cell_indices.size() < geometry_item_count) {
				poly_cell_indices.resize(geometry_item_count);
			}
		}
		r_indices.insert(key, poly_cell_indices);
	}
	return true;
}

bool G4MFMeshSurface4D::_import_pad_simplex_corner_binding(PackedInt32Array &r_indices, const int64_t p_simplex_corner_count, const int32_t p_zero_value_index, const String &p_binding_name) {
	const int64_t index_count = r_indices.size();
	ERR_FAIL_COND_V_MSG(index_count % 4 != 0, false, "G4MFMeshSurface4D: " + p_binding_name + " simplex binding has " + itos(index_count) + " indices, which is not a multiple of the 4 corners per simplex.");
	ERR_FAIL_COND_V_MSG(index_count > p_simplex_corner_count, false, "G4MFMeshSurface4D: " + p_binding_name + " simplex binding has " + itos(index_count) + " indices, but the surface only has " + itos(p_simplex_corner_count) + " simplex corners.");
	// G4MF allows a simplex binding to be shorter than the number of corners, with the missing corners having
	// default values. Tetrahedral meshes need one index per corner, so the missing corners point at a zero value.
	if (index_count < p_simplex_corner_count) {
		r_indices.resize(p_simplex_corner_count);
		for (int64_t i = index_count; i < p_simplex_corner_count; i++) {
			r_indices.set(i, p_zero_value_index);
		}
	}
	return true;
}

Ref<ArrayPolyMesh4D> G4MFMeshSurface4D::import_generate_poly_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), Ref<ArrayPolyMesh4D>());
	Ref<ArrayPolyMesh4D> poly_mesh;
	poly_mesh.instantiate();
	poly_mesh->set_poly_cell_vertex_positions(p_vertices);
	if (_normals_binding.is_valid()) {
		poly_mesh->set_poly_cell_normal_values(_normals_binding->load_values_as_vector4s(p_g4mf_state));
	}
	if (_texture_map_binding.is_valid()) {
		poly_mesh->set_poly_cell_texture_map_values(_texture_map_binding->load_values_as_vector3s(p_g4mf_state));
	}
	PackedInt32Array edge_indices;
	if (_edges_accessor_index >= 0) {
		edge_indices = load_edge_indices(p_g4mf_state);
		poly_mesh->set_edge_vertex_indices(edge_indices);
	}
	Vector<Vector<PackedInt32Array>> separated_geometry;
	if (_geometry_accessor_indices.size() > 0) {
		ERR_FAIL_COND_V_MSG(_edges_accessor_index < 0, Ref<ArrayPolyMesh4D>(), "G4MFMeshSurface4D: Cannot import poly mesh surface geometry because edges are missing.");
		separated_geometry = load_geometry_separated(p_g4mf_state);
		ERR_FAIL_COND_V_MSG(separated_geometry.size() != _geometry_accessor_indices.size(), Ref<ArrayPolyMesh4D>(), "G4MFMeshSurface4D: Failed to decode poly mesh geometry.");
		poly_mesh->set_poly_cell_indices(separated_geometry);
		if (_seams_accessor_index >= 0) {
			poly_mesh->set_seam_face_indices_bind(load_seam_indices(p_g4mf_state));
		}
	}
	// Decode the geometry bindings for normals and texture maps into the poly mesh's binding structures.
	const int64_t edge_count = edge_indices.size() / 2;
	if (_normals_binding.is_valid()) {
		PackedVector4Array normal_values = poly_mesh->get_poly_cell_normal_values();
		HashMap<Vector2i, Vector<PackedInt32Array>> all_poly_cell_normal_indices;
		const bool decoded = _import_decode_geometry_bindings(p_g4mf_state, _normals_binding, p_vertices.size(), edge_count, separated_geometry, normal_values.size(), all_poly_cell_normal_indices, "Normals");
		ERR_FAIL_COND_V_MSG(!decoded, Ref<ArrayPolyMesh4D>(), "G4MFMeshSurface4D: Failed to decode the normals geometry bindings.");
		if (all_poly_cell_normal_indices.has(PolyMesh4D::PER_CELL_KEY) && separated_geometry.size() > 1) {
			// Boundary normals are the one dense binding the mesh classes need to be complete. Missing cells get a zero
			// normal, which means "no normal", and `calculate_boundary_normals` below fills those in from the geometry.
			Vector<PackedInt32Array> &per_cell = all_poly_cell_normal_indices[PolyMesh4D::PER_CELL_KEY];
			const int64_t boundary_cell_count = separated_geometry[1].size();
			if (per_cell.size() == 1 && per_cell[0].size() < boundary_cell_count) {
				const int32_t zero_normal_value_index = (int32_t)Vector4D::vector4_array_append_deduplicate(normal_values, Vector4());
				PackedInt32Array boundary_normal_indices = per_cell[0];
				const int64_t old_count = boundary_normal_indices.size();
				boundary_normal_indices.resize(boundary_cell_count);
				for (int64_t i = old_count; i < boundary_cell_count; i++) {
					boundary_normal_indices.set(i, zero_normal_value_index);
				}
				per_cell.set(0, boundary_normal_indices);
				poly_mesh->set_poly_cell_normal_values(normal_values);
			}
		}
		if (!all_poly_cell_normal_indices.is_empty()) {
			poly_mesh->set_all_poly_cell_normal_indices(all_poly_cell_normal_indices);
		}
	}
	if (_texture_map_binding.is_valid()) {
		HashMap<Vector2i, Vector<PackedInt32Array>> all_poly_cell_texture_map_indices;
		const bool decoded = _import_decode_geometry_bindings(p_g4mf_state, _texture_map_binding, p_vertices.size(), edge_count, separated_geometry, poly_mesh->get_poly_cell_texture_map_values().size(), all_poly_cell_texture_map_indices, "Texture map");
		ERR_FAIL_COND_V_MSG(!decoded, Ref<ArrayPolyMesh4D>(), "G4MFMeshSurface4D: Failed to decode the texture map geometry bindings.");
		if (!all_poly_cell_texture_map_indices.is_empty()) {
			poly_mesh->set_all_poly_cell_texture_map_indices(all_poly_cell_texture_map_indices);
		}
	}
	const bool is_valid = poly_mesh->is_mesh_data_valid();
	ERR_FAIL_COND_V_MSG(!is_valid, Ref<ArrayPolyMesh4D>(), "G4MFMeshSurface4D: The mesh data is not valid. Returning an empty mesh instead.");
	const Vector<Vector<PackedInt32Array>> poly_cell_indices = poly_mesh->get_poly_cell_indices();
	if (poly_cell_indices.size() > 1 && !poly_cell_indices[1].is_empty()) {
		// Keep any boundary normals already imported from the (3, 3) binding, only filling in the rest.
		poly_mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, true);
	}
	if (_material_index >= 0) {
		const TypedArray<G4MFMaterial4D> materials = p_g4mf_state->get_g4mf_materials();
		ERR_FAIL_INDEX_V(_material_index, materials.size(), poly_mesh);
		const Ref<G4MFMaterial4D> g4mf_material = materials[_material_index];
		ERR_FAIL_COND_V(g4mf_material.is_null(), poly_mesh);
		Ref<PolyMaterial4D> poly_material = g4mf_material->import_get_or_generate_poly_material(p_g4mf_state);
		poly_mesh->set_material(poly_material);
	}
	return poly_mesh;
}

Ref<ArrayTetraMesh4D> G4MFMeshSurface4D::import_generate_tetra_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), Ref<ArrayTetraMesh4D>());
	Ref<ArrayTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_vertex_positions(p_vertices);
	if (_simplexes_accessor_index >= 0) {
		const PackedInt32Array simplex_indices = load_simplex_indices(p_g4mf_state);
		tetra_mesh->set_simplex_cell_vertex_indices(simplex_indices);
		ERR_FAIL_COND_V_MSG(!tetra_mesh->is_mesh_data_valid(), Ref<ArrayTetraMesh4D>(), "G4MFMeshSurface4D: Invalid simplex geometry.");
		tetra_mesh->calculate_boundary_normals();
	}
	// G4MF stores indexed values, which is also how the runtime mesh classes store
	// their data, so the values and indices can be loaded with minimal conversion.
	const int64_t simplex_corner_count = tetra_mesh->get_simplex_cell_vertex_indices().size();
	if (_normals_binding.is_valid()) {
		PackedVector4Array normal_values = _normals_binding->load_values_as_vector4s(p_g4mf_state);
		if (_normals_binding->get_simplexes_accessor_index() >= 0) {
			PackedInt32Array normal_indices = _normals_binding->load_simplex_indices(p_g4mf_state);
			if (!normal_indices.is_empty()) {
				// Only append a zero value if it is needed, and reuse an existing zero if the pool already has one.
				const int32_t zero_normal_value_index = normal_indices.size() < simplex_corner_count ? (int32_t)Vector4D::vector4_array_append_deduplicate(normal_values, Vector4()) : -1;
				const bool padded = _import_pad_simplex_corner_binding(normal_indices, simplex_corner_count, zero_normal_value_index, "Normals");
				ERR_FAIL_COND_V(!padded, Ref<ArrayTetraMesh4D>());
				tetra_mesh->set_simplex_cell_normal_indices(normal_indices);
			}
		}
		tetra_mesh->set_normal_values(normal_values);
	}
	if (_texture_map_binding.is_valid()) {
		PackedVector3Array texture_map_values = _texture_map_binding->load_values_as_vector3s(p_g4mf_state);
		if (_texture_map_binding->get_simplexes_accessor_index() >= 0) {
			PackedInt32Array texture_map_indices = _texture_map_binding->load_simplex_indices(p_g4mf_state);
			if (!texture_map_indices.is_empty()) {
				const int32_t zero_texture_map_value_index = texture_map_indices.size() < simplex_corner_count ? (int32_t)Vector4D::vector3_array_append_deduplicate(texture_map_values, Vector3()) : -1;
				const bool padded = _import_pad_simplex_corner_binding(texture_map_indices, simplex_corner_count, zero_texture_map_value_index, "Texture map");
				ERR_FAIL_COND_V(!padded, Ref<ArrayTetraMesh4D>());
				tetra_mesh->set_simplex_cell_texture_map_indices(texture_map_indices);
			}
		}
		tetra_mesh->set_texture_map_values(texture_map_values);
	}
	const bool is_valid = tetra_mesh->is_mesh_data_valid();
	ERR_FAIL_COND_V_MSG(!is_valid, Ref<ArrayTetraMesh4D>(), "G4MFMeshSurface4D.generate_tetra_mesh_surface: The mesh data is not valid. Returning an empty mesh instead.");
	if (_material_index >= 0) {
		const TypedArray<G4MFMaterial4D> materials = p_g4mf_state->get_g4mf_materials();
		ERR_FAIL_INDEX_V(_material_index, materials.size(), tetra_mesh);
		const Ref<G4MFMaterial4D> g4mf_material = materials[_material_index];
		ERR_FAIL_COND_V(g4mf_material.is_null(), tetra_mesh);
		Ref<TetraMaterial4D> tetra_material = g4mf_material->import_get_or_generate_tetra_material(p_g4mf_state);
		tetra_mesh->set_material(tetra_material);
	}
	return tetra_mesh;
}

Ref<ArrayWireMesh4D> G4MFMeshSurface4D::import_generate_wire_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), Ref<ArrayWireMesh4D>());
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_vertex_positions(p_vertices);
	if (_edges_accessor_index >= 0) {
		const PackedInt32Array edge_indices = load_edge_indices(p_g4mf_state);
		wire_mesh->set_edge_indices(edge_indices);
	} else if (_simplexes_accessor_index >= 0) {
		// Calculate edges from simplex cells.
		const PackedInt32Array simplex_vertex_indices = load_simplex_indices(p_g4mf_state);
		ERR_FAIL_COND_V_MSG(simplex_vertex_indices.size() % 4 != 0, Ref<ArrayWireMesh4D>(), "G4MFMeshSurface4D: Invalid simplex geometry.");
		const PackedInt32Array edge_indices = TetraMesh4D::calculate_edge_indices_from_simplex_cell_vertex_indices(simplex_vertex_indices);
		wire_mesh->set_edge_indices(edge_indices);
	}
	const bool is_valid = wire_mesh->is_mesh_data_valid();
	ERR_FAIL_COND_V_MSG(!is_valid, Ref<ArrayWireMesh4D>(), "G4MFMeshSurface4D.generate_wire_mesh_surface: The mesh data is not valid. Returning an empty mesh instead.");
	if (_material_index >= 0) {
		const TypedArray<G4MFMaterial4D> materials = p_g4mf_state->get_g4mf_materials();
		ERR_FAIL_INDEX_V(_material_index, materials.size(), wire_mesh);
		const Ref<G4MFMaterial4D> g4mf_material = materials[_material_index];
		ERR_FAIL_COND_V(g4mf_material.is_null(), wire_mesh);
		Ref<WireMaterial4D> wire_material = g4mf_material->import_get_or_generate_wire_material(p_g4mf_state);
		wire_mesh->set_material(wire_material);
	}
	return wire_mesh;
}

Ref<SingleSurfaceMesh4D> G4MFMeshSurface4D::import_generate_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const {
	const G4MFMeshSurface4D::MeshSurfaceFormat compatible_mesh_surface_format = _get_compatible_mesh_surface_format(p_g4mf_state->get_preferred_mesh_surface_format());
	switch (compatible_mesh_surface_format) {
		case G4MFMeshSurface4D::MESH_SURFACE_FORMAT_POLYTOPE:
			return import_generate_poly_mesh_surface(p_g4mf_state, p_vertices);
		case G4MFMeshSurface4D::MESH_SURFACE_FORMAT_TETRAHEDRAL:
			return import_generate_tetra_mesh_surface(p_g4mf_state, p_vertices);
		case G4MFMeshSurface4D::MESH_SURFACE_FORMAT_WIREFRAME:
			return import_generate_wire_mesh_surface(p_g4mf_state, p_vertices);
	}
	ERR_FAIL_V_MSG(Ref<SingleSurfaceMesh4D>(), "G4MFMeshSurface4D::import_generate_mesh_surface: No compatible mesh format found for the mesh.");
}

TypedArray<G4MFMeshSurfaceBindingGeometry4D> G4MFMeshSurface4D::_export_encode_geometry_bindings(const Ref<G4MFState4D> &p_g4mf_state, const HashMap<Vector2i, Vector<PackedInt32Array>> &p_indices, const bool p_deduplicate) {
	// The runtime mesh classes store indexed values, which is also how G4MF stores
	// this data, so the indices can be packed and encoded with minimal conversion.
	TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings;
	for (const KeyValue<Vector2i, Vector<PackedInt32Array>> &pair : p_indices) {
		const Vector2i &key = pair.key;
		const Vector<PackedInt32Array> &value = pair.value;
		PackedInt32Array flat_array;
		int vector_size = 1;
		if (key.x == key.y) {
			// Geometry bindings that are not decomposed, meaning "decomposeDimension"
			// is equal to "geometryDimension", are stored as a dense array of indices,
			// where each index corresponds to a geometry item of the specified geometry dimension.
			// There is no need to store an amount of members, because it is always 1.
			// These are crashes because all problems should be caught by `is_mesh_data_valid()` before getting to the G4MF code.
			CRASH_COND(value.size() != 1);
			flat_array = value[0];
		} else if (key.x == 1 && key.y == 0) {
			// Geometry bindings referring to vertices of edges, meaning "decomposeDimension"
			// is 0 and "geometryDimension" is 1, are stored as a dense array of indices,
			// where every 2 indices correspond to the 2 vertices of each edge geometry item.
			// There is no need to store an amount of members, because it is always 2.
			// Godot 4D actually stores this as one 2-member array per edge, so flatten
			// those without a count prefix since the member count is always 2.
			for (int cell_index = 0; cell_index < value.size(); cell_index++) {
				CRASH_COND(value[cell_index].size() != 2);
				flat_array.append_array(value[cell_index]);
			}
			vector_size = 2;
		} else {
			// In all other cases, geometry binding accessor indices behave the same as
			// the mesh surface's geometry items. Meaning, the first number is the amount
			// of members in the first cell, followed by those members, then the amount
			// of members in the second cell, followed by those members, and so on.
			bool has_any_data = false;
			for (int cell_index = 0; cell_index < value.size(); cell_index++) {
				const PackedInt32Array &cell_indices = value[cell_index];
				has_any_data = has_any_data || !cell_indices.is_empty();
				// Zero counts preserve the positions of cells with no binding data.
				flat_array.append(cell_indices.size());
				flat_array.append_array(cell_indices);
			}
			if (!has_any_data) {
				// Every cell is missing data, so this binding carries no information. Skip it.
				continue;
			}
		}
		if (flat_array.is_empty()) {
			continue;
		}
		// Encode the flat array of indices into an accessor.
		const int indices_accessor_index = G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, flat_array, vector_size, p_deduplicate);
		ERR_FAIL_COND_V(indices_accessor_index < 0, geometry_bindings);
		// Save the indices and dimensions in a new binding geometry object.
		Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding;
		geometry_binding.instantiate();
		geometry_binding->set_geometry_dimension(key.x);
		geometry_binding->set_decompose_dimension(key.y);
		geometry_binding->set_indices_accessor_index(indices_accessor_index);
		geometry_bindings.append(geometry_binding);
	}
	return geometry_bindings;
}

void G4MFMeshSurface4D::_export_convert_poly_mesh_surface_for_state(const Ref<G4MFState4D> &p_g4mf_state, const Ref<PolyMesh4D> &p_poly_mesh, PackedVector4Array &r_normal_values, PackedVector3Array &r_texture_map_values, const bool p_deduplicate) {
	const Vector<Vector<PackedInt32Array>> separated_geometry = p_poly_mesh->get_poly_cell_indices();
	if (!separated_geometry.is_empty()) {
		convert_separated_geometry_into_packed(p_g4mf_state, separated_geometry, p_deduplicate);
	}
	// Normals: Gather the poly mesh's normal bindings, then convert them into G4MF geometry bindings.
	HashMap<Vector2i, Vector<PackedInt32Array>> all_poly_cell_normal_indices = p_poly_mesh->get_all_poly_cell_normal_indices();
	// Only for normals: Convert boundary normals into these bindings.
	// ArrayPolyMesh4D already does this internally and will return a (3, 3) key, but this code works as a fallback.
	if (!all_poly_cell_normal_indices.has(Vector2i(3, 3))) {
		const PackedVector4Array boundary_normals = p_poly_mesh->get_poly_cell_boundary_normals();
		const int64_t boundary_normal_count = boundary_normals.size();
		if (boundary_normal_count > 0) {
			PackedInt32Array boundary_normal_indices;
			boundary_normal_indices.resize_uninitialized(boundary_normal_count);
			for (int64_t i = 0; i < boundary_normal_count; i++) {
				boundary_normal_indices.set(i, Vector4D::vector4_array_append_deduplicate(r_normal_values, boundary_normals[i]));
			}
			// Geometry bindings that are not decomposed, meaning "decomposeDimension"
			// is equal to "geometryDimension", are stored as a dense array of indices.
			all_poly_cell_normal_indices.insert(Vector2i(3, 3), Vector<PackedInt32Array>{ boundary_normal_indices });
		}
	}
	// Only create the binding if there is something to put in it. An empty binding would
	// imply the surface has normal data, and force an unused values accessor to be written.
	const TypedArray<G4MFMeshSurfaceBindingGeometry4D> normal_geometry_bindings = _export_encode_geometry_bindings(p_g4mf_state, all_poly_cell_normal_indices, p_deduplicate);
	if (!normal_geometry_bindings.is_empty()) {
		if (_normals_binding.is_null()) {
			_normals_binding.instantiate();
		}
		TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings = _normals_binding->get_geometry_bindings();
		geometry_bindings.append_array(normal_geometry_bindings);
		_normals_binding->set_geometry_bindings(geometry_bindings);
	}
	// Texture maps: Do the same thing, except there is no equivalent for boundary normals.
	HashMap<Vector2i, Vector<PackedInt32Array>> all_poly_cell_texture_map_indices = p_poly_mesh->get_all_poly_cell_texture_map_indices();
	const TypedArray<G4MFMeshSurfaceBindingGeometry4D> texture_map_geometry_bindings = _export_encode_geometry_bindings(p_g4mf_state, all_poly_cell_texture_map_indices, p_deduplicate);
	if (!texture_map_geometry_bindings.is_empty()) {
		if (_texture_map_binding.is_null()) {
			_texture_map_binding.instantiate();
		}
		TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings = _texture_map_binding->get_geometry_bindings();
		geometry_bindings.append_array(texture_map_geometry_bindings);
		_texture_map_binding->set_geometry_bindings(geometry_bindings);
	}
}

void G4MFMeshSurface4D::_export_convert_tetra_mesh_surface_for_state(const Ref<G4MFState4D> &p_g4mf_state, const Ref<TetraMesh4D> &p_tetra_mesh, const bool p_deduplicate) {
	const PackedInt32Array simplex_vertex_indices = p_tetra_mesh->get_simplex_cell_vertex_indices();
	if (!simplex_vertex_indices.is_empty()) {
		Array simplex_indices_variants;
		simplex_indices_variants.resize(simplex_vertex_indices.size());
		for (int i = 0; i < simplex_vertex_indices.size(); i++) {
			simplex_indices_variants[i] = simplex_vertex_indices[i];
		}
		const String simplex_prim_type = G4MFAccessor4D::minimal_component_type_for_int32s(simplex_vertex_indices);
		const int simplexes_accessor = G4MFAccessor4D::encode_new_accessor_from_variants(p_g4mf_state, simplex_indices_variants, simplex_prim_type, 4, p_deduplicate);
		ERR_FAIL_COND_MSG(simplexes_accessor < 0, "G4MFMeshSurface4D: Failed to encode simplex cells into G4MFState4D.");
		set_simplexes_accessor_index(simplexes_accessor);
	}
	// The runtime mesh classes store indexed values, which is also how G4MF stores
	// this data, so the indices can be encoded without any conversion.
	const PackedInt32Array simplex_normal_indices = p_tetra_mesh->get_simplex_cell_normal_indices();
	if (!simplex_normal_indices.is_empty()) {
		if (_normals_binding.is_null()) {
			_normals_binding.instantiate();
		}
		_normals_binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, simplex_normal_indices, 4, p_deduplicate));
	}
	const PackedInt32Array simplex_texture_map_indices = p_tetra_mesh->get_simplex_cell_texture_map_indices();
	if (!simplex_texture_map_indices.is_empty()) {
		if (_texture_map_binding.is_null()) {
			_texture_map_binding.instantiate();
		}
		_texture_map_binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, simplex_texture_map_indices, 4, p_deduplicate));
	}
}

Ref<G4MFMeshSurface4D> G4MFMeshSurface4D::export_convert_mesh_surface_for_state(Ref<G4MFState4D> p_g4mf_state, const Ref<SingleSurfaceMesh4D> &p_surface_mesh, const bool p_deduplicate) {
	ERR_FAIL_COND_V_MSG(p_surface_mesh.is_null(), Ref<G4MFMeshSurface4D>(), "G4MFMeshSurface4D: Cannot convert a null mesh surface to G4MF.");
	// Validate first: the conversion below assumes consistent data, and only crashes (rather than erroring) on inconsistencies.
	ERR_FAIL_COND_V_MSG(!p_surface_mesh->is_mesh_data_valid(), Ref<G4MFMeshSurface4D>(), "G4MFMeshSurface4D: Cannot convert the mesh surface '" + p_surface_mesh->get_name() + "' to G4MF because its mesh data is invalid.");
	Ref<G4MFMeshSurface4D> surface;
	surface.instantiate();
	// Convert the material.
	const Ref<Material4D> material = p_surface_mesh->get_material();
	if (material.is_valid() && !material->is_default_material()) {
		const int material_index = G4MFMaterial4D::export_convert_material_into_state(p_g4mf_state, material, p_deduplicate);
		surface->set_material_index(material_index);
		if (material_index < 0) {
			ERR_PRINT("G4MFMeshSurface4D: Failed to encode material into G4MFState4D.");
		}
	}
	PackedVector4Array normal_values = p_surface_mesh->get_normal_values();
	PackedVector3Array texture_map_values = p_surface_mesh->get_texture_map_values();
	const Ref<PolyMesh4D> poly_mesh = p_surface_mesh;
	bool export_edges = true;
	if (poly_mesh.is_valid()) {
		// A poly mesh can have geometry bindings but no exposed simplexes. If that happens, these will be empty.
		// Otherwise, `get_normal_values` is defined to be a superset of `get_poly_cell_normal_values`.
		if (normal_values.is_empty()) {
			normal_values = poly_mesh->get_poly_cell_normal_values();
		}
		if (texture_map_values.is_empty()) {
			texture_map_values = poly_mesh->get_poly_cell_texture_map_values();
		}
		// For poly meshes, convert both poly cell geometry and tetrahedral simplex cells into accessors.
		// When simplex bindings exist, their values are a superset of the poly cell values,
		// so both representations can share one values accessor per binding.
		surface->_export_convert_poly_mesh_surface_for_state(p_g4mf_state, poly_mesh, normal_values, texture_map_values, p_deduplicate);
		surface->_export_convert_tetra_mesh_surface_for_state(p_g4mf_state, poly_mesh, p_deduplicate);
		// Don't return here: Always convert edges for poly meshes.
	} else {
		// For tetra meshes, convert tetrahedral simplex cells into an accessor.
		const Ref<TetraMesh4D> tetra_mesh = p_surface_mesh;
		if (tetra_mesh.is_valid()) {
			surface->_export_convert_tetra_mesh_surface_for_state(p_g4mf_state, tetra_mesh, p_deduplicate);
			// For BoxTetraMesh4D in polytope mode, use its explicitly defined edges.
			// Other meshes can skip saving this and rely on implicitly calculated ones.
			const Ref<BoxTetraMesh4D> box_tetra_mesh = p_surface_mesh;
			if (box_tetra_mesh.is_valid() && box_tetra_mesh->get_tetra_decomp() == BoxTetraMesh4D::BOX_TETRA_DECOMP_48_CELL_POLYTOPE) {
				surface->set_polytope_simplexes(true);
				// Don't return here, so that we keep the edge indices code below.
			} else {
				export_edges = false;
			}
		}
	}
	// Only encode value pools used by a geometry or simplex binding. An unused
	// pool does not imply that the surface has normal or texture map data.
	if (surface->_normals_binding.is_valid()) {
		const int normal_values_accessor = G4MFAccessor4D::encode_new_accessor_from_vector4s(p_g4mf_state, normal_values, p_deduplicate);
		ERR_FAIL_COND_V(normal_values_accessor < 0, surface);
		surface->_normals_binding->set_values_accessor_index(normal_values_accessor);
	}
	if (surface->_texture_map_binding.is_valid()) {
		const int texture_map_values_accessor = G4MFAccessor4D::encode_new_accessor_from_vector3s(p_g4mf_state, texture_map_values, p_deduplicate);
		ERR_FAIL_COND_V(texture_map_values_accessor < 0, surface);
		surface->_texture_map_binding->set_values_accessor_index(texture_map_values_accessor);
	}
	if (!export_edges) {
		return surface;
	}
	// Convert edges into an accessor.
	const PackedInt32Array edge_indices = p_surface_mesh->get_edge_indices();
	ERR_FAIL_COND_V_MSG(edge_indices.is_empty(), surface, "G4MFMeshSurface4D: Mesh4D has no edges.");
	Array edge_indices_variants;
	edge_indices_variants.resize(edge_indices.size());
	for (int i = 0; i < edge_indices.size(); i++) {
		edge_indices_variants[i] = edge_indices[i];
	}
	const String edge_prim_type = G4MFAccessor4D::minimal_component_type_for_int32s(edge_indices);
	const int edges_accessor = G4MFAccessor4D::encode_new_accessor_from_variants(p_g4mf_state, edge_indices_variants, edge_prim_type, 2, p_deduplicate);
	ERR_FAIL_COND_V_MSG(edges_accessor < 0, surface, "G4MFMeshSurface4D: Failed to encode edges into G4MFState4D.");
	surface->set_edges_accessor_index(edges_accessor);
	return surface;
}

Ref<G4MFMeshSurface4D> G4MFMeshSurface4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFMeshSurface4D> surface;
	surface.instantiate();
	surface->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("edges")) {
		surface->set_edges_accessor_index(p_dict["edges"]);
	}
	if (p_dict.has("geometry")) {
		const Array geometry_array = p_dict["geometry"];
		PackedInt32Array geometry_accessor_indices;
		geometry_accessor_indices.resize(geometry_array.size());
		for (int i = 0; i < geometry_array.size(); i++) {
			geometry_accessor_indices.set(i, geometry_array[i]);
		}
		surface->set_geometry_accessor_indices(geometry_accessor_indices);
	}
	if (p_dict.has("material")) {
		surface->set_material_index(p_dict["material"]);
	}
	if (p_dict.has("normals")) {
		surface->set_normals_binding(G4MFMeshSurfaceBinding4D::from_dictionary(p_dict["normals"]));
	}
	if (p_dict.has("polytopeSimplexes")) {
		surface->set_polytope_simplexes(p_dict["polytopeSimplexes"]);
	}
	if (p_dict.has("seams")) {
		surface->set_seams_accessor_index(p_dict["seams"]);
	}
	if (p_dict.has("simplexes")) {
		surface->set_simplexes_accessor_index(p_dict["simplexes"]);
	}
	if (p_dict.has("textureMap")) {
		surface->set_texture_map_binding(G4MFMeshSurfaceBinding4D::from_dictionary(p_dict["textureMap"]));
	}
	return surface;
}

Dictionary G4MFMeshSurface4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_edges_accessor_index >= 0) {
		dict["edges"] = _edges_accessor_index;
	}
	if (_geometry_accessor_indices.size() > 0) {
		Array geometry_array;
		geometry_array.resize(_geometry_accessor_indices.size());
		for (int i = 0; i < _geometry_accessor_indices.size(); i++) {
			geometry_array[i] = _geometry_accessor_indices[i];
		}
		dict["geometry"] = geometry_array;
	}
	if (_material_index >= 0) {
		dict["material"] = _material_index;
	}
	if (_normals_binding.is_valid()) {
		dict["normals"] = _normals_binding->to_dictionary();
	}
	if (_polytope_simplexes) {
		dict["polytopeSimplexes"] = _polytope_simplexes;
	}
	if (_seams_accessor_index >= 0) {
		dict["seams"] = _seams_accessor_index;
	}
	if (_simplexes_accessor_index >= 0) {
		dict["simplexes"] = _simplexes_accessor_index;
	}
	if (_texture_map_binding.is_valid()) {
		dict["textureMap"] = _texture_map_binding->to_dictionary();
	}
	return dict;
}

void G4MFMeshSurface4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_edges_accessor_index"), &G4MFMeshSurface4D::get_edges_accessor_index);
	ClassDB::bind_method(D_METHOD("set_edges_accessor_index", "edges_accessor_index"), &G4MFMeshSurface4D::set_edges_accessor_index);
	ClassDB::bind_method(D_METHOD("get_geometry_accessor_indices"), &G4MFMeshSurface4D::get_geometry_accessor_indices);
	ClassDB::bind_method(D_METHOD("set_geometry_accessor_indices", "geometry_accessor_indices"), &G4MFMeshSurface4D::set_geometry_accessor_indices);
	ClassDB::bind_method(D_METHOD("get_material_index"), &G4MFMeshSurface4D::get_material_index);
	ClassDB::bind_method(D_METHOD("set_material_index", "material_index"), &G4MFMeshSurface4D::set_material_index);
	ClassDB::bind_method(D_METHOD("get_normals_binding"), &G4MFMeshSurface4D::get_normals_binding);
	ClassDB::bind_method(D_METHOD("set_normals_binding", "normals_binding"), &G4MFMeshSurface4D::set_normals_binding);
	ClassDB::bind_method(D_METHOD("get_polytope_simplexes"), &G4MFMeshSurface4D::get_polytope_simplexes);
	ClassDB::bind_method(D_METHOD("set_polytope_simplexes", "polytope_simplexes"), &G4MFMeshSurface4D::set_polytope_simplexes);
	ClassDB::bind_method(D_METHOD("get_seams_accessor_index"), &G4MFMeshSurface4D::get_seams_accessor_index);
	ClassDB::bind_method(D_METHOD("set_seams_accessor_index", "seams_accessor_index"), &G4MFMeshSurface4D::set_seams_accessor_index);
	ClassDB::bind_method(D_METHOD("get_simplexes_accessor_index"), &G4MFMeshSurface4D::get_simplexes_accessor_index);
	ClassDB::bind_method(D_METHOD("set_simplexes_accessor_index", "simplexes_accessor_index"), &G4MFMeshSurface4D::set_simplexes_accessor_index);
	ClassDB::bind_method(D_METHOD("get_texture_map_binding"), &G4MFMeshSurface4D::get_texture_map_binding);
	ClassDB::bind_method(D_METHOD("set_texture_map_binding", "texture_map_binding"), &G4MFMeshSurface4D::set_texture_map_binding);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMeshSurface4D::is_equal_exact);
	ClassDB::bind_method(D_METHOD("convert_separated_geometry_into_packed", "g4mf_state", "separated_geometry", "deduplicate"), &G4MFMeshSurface4D::convert_separated_geometry_into_packed_bind, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("load_geometry_separated", "g4mf_state"), &G4MFMeshSurface4D::load_geometry_separated_bind);
	ClassDB::bind_method(D_METHOD("load_edge_indices", "g4mf_state"), &G4MFMeshSurface4D::load_edge_indices);
	ClassDB::bind_method(D_METHOD("load_seam_indices", "g4mf_state"), &G4MFMeshSurface4D::load_seam_indices);
	ClassDB::bind_method(D_METHOD("load_simplex_indices", "g4mf_state"), &G4MFMeshSurface4D::load_simplex_indices);
	ClassDB::bind_method(D_METHOD("import_generate_tetra_mesh_surface", "g4mf_state", "vertices"), &G4MFMeshSurface4D::import_generate_tetra_mesh_surface);
	ClassDB::bind_method(D_METHOD("import_generate_wire_mesh_surface", "g4mf_state", "vertices"), &G4MFMeshSurface4D::import_generate_wire_mesh_surface);
	ClassDB::bind_static_method("G4MFMeshSurface4D", D_METHOD("export_convert_mesh_surface_for_state", "g4mf_state", "mesh", "deduplicate"), &G4MFMeshSurface4D::export_convert_mesh_surface_for_state, DEFVAL(true));

	ClassDB::bind_static_method("G4MFMeshSurface4D", D_METHOD("from_dictionary", "dict"), &G4MFMeshSurface4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMeshSurface4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "edges_accessor_index"), "set_edges_accessor_index", "get_edges_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "geometry_accessor_indices"), "set_geometry_accessor_indices", "get_geometry_accessor_indices");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material_index"), "set_material_index", "get_material_index");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normals_binding", PROPERTY_HINT_RESOURCE_TYPE, "G4MFMeshSurfaceBinding4D"), "set_normals_binding", "get_normals_binding");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "polytope_simplexes"), "set_polytope_simplexes", "get_polytope_simplexes");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seams_accessor_index"), "set_seams_accessor_index", "get_seams_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "simplexes_accessor_index"), "set_simplexes_accessor_index", "get_simplexes_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture_map_binding", PROPERTY_HINT_RESOURCE_TYPE, "G4MFMeshSurfaceBinding4D"), "set_texture_map_binding", "get_texture_map_binding");

	BIND_ENUM_CONSTANT(MESH_SURFACE_FORMAT_POLYTOPE);
	BIND_ENUM_CONSTANT(MESH_SURFACE_FORMAT_TETRAHEDRAL);
	BIND_ENUM_CONSTANT(MESH_SURFACE_FORMAT_WIREFRAME);
}
