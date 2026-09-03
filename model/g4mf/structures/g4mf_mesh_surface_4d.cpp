#include "g4mf_mesh_surface_4d.h"

#include "../../mesh/tetra/box_tetra_mesh_4d.h"
#include "../g4mf_state_4d.h"

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
		const int geom_accessor_index = G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, packed_geometry_data, p_deduplicate);
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
		const int geom_accessor_index = G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, packed_geometry_data, p_deduplicate);
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

Ref<ArrayPolyMesh4D> G4MFMeshSurface4D::generate_poly_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const {
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
	if (_edges_accessor_index >= 0) {
		const PackedInt32Array edge_indices = load_edge_indices(p_g4mf_state);
		poly_mesh->set_edge_vertex_indices(edge_indices);
	}
	if (_geometry_accessor_indices.size() > 0) {
		ERR_FAIL_COND_V_MSG(_edges_accessor_index < 0, Ref<ArrayPolyMesh4D>(), "G4MFMeshSurface4D: Cannot import poly mesh surface geometry because edges are missing.");
		const Vector<Vector<PackedInt32Array>> geom_sep = load_geometry_separated(p_g4mf_state);
		ERR_FAIL_COND_V_MSG(geom_sep.size() != _geometry_accessor_indices.size(), Ref<ArrayPolyMesh4D>(), "G4MFMeshSurface4D: Failed to decode poly mesh geometry.");
		poly_mesh->set_poly_cell_indices(geom_sep);
		if (geom_sep.size() > 1) {
			// G4MF stores indexed values, which is also how the runtime mesh classes store
			// their data, so the values and indices can be loaded without any conversion.
			if (_normals_binding.is_valid()) {
				const TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_decompositions = _normals_binding->get_geometry_bindings();
				for (int bind_geom_index = 0; bind_geom_index < geometry_decompositions.size(); bind_geom_index++) {
					const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_decomposition = geometry_decompositions[bind_geom_index];
					ERR_FAIL_COND_V(geometry_decomposition.is_null(), Ref<ArrayPolyMesh4D>());
					// Look for 3D poly cells (geom dimension 3) decomposed into vertices (decomp dim 0).
					if (geometry_decomposition->get_geometry_dimension() == 3 && geometry_decomposition->get_decompose_dimension() == 0) {
						const PackedInt32Array packed_normal_indices = geometry_decomposition->load_indices(p_g4mf_state);
						Vector<PackedInt32Array> poly_cell_normal_indices;
						int64_t norm_index_index = 0;
						const int64_t value_count = poly_mesh->get_poly_cell_normal_values().size();
						while (norm_index_index < packed_normal_indices.size()) {
							PackedInt32Array this_cell_normal_indices;
							const int vertex_count = packed_normal_indices[norm_index_index];
							ERR_FAIL_COND_V_MSG(vertex_count < 0 || vertex_count > packed_normal_indices.size() - norm_index_index - 1, Ref<ArrayPolyMesh4D>(), "G4MFMeshSurface4D: Invalid packed normal binding count.");
							this_cell_normal_indices.resize(vertex_count);
							norm_index_index += 1;
							for (int i = 0; i < vertex_count; i++) {
								const int32_t value_index = packed_normal_indices[norm_index_index];
								ERR_FAIL_INDEX_V(value_index, value_count, Ref<ArrayPolyMesh4D>());
								this_cell_normal_indices.set(i, value_index);
								norm_index_index += 1;
							}
							poly_cell_normal_indices.append(this_cell_normal_indices);
						}
						poly_mesh->set_poly_cell_normal_indices(poly_cell_normal_indices);
						break;
					}
				}
			}
			if (_texture_map_binding.is_valid()) {
				const TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_decompositions = _texture_map_binding->get_geometry_bindings();
				for (int bind_geom_index = 0; bind_geom_index < geometry_decompositions.size(); bind_geom_index++) {
					const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_decomposition = geometry_decompositions[bind_geom_index];
					ERR_FAIL_COND_V(geometry_decomposition.is_null(), Ref<ArrayPolyMesh4D>());
					// Look for 3D poly cells (geom dimension 3) decomposed into vertices (decomp dim 0).
					if (geometry_decomposition->get_geometry_dimension() == 3 && geometry_decomposition->get_decompose_dimension() == 0) {
						const PackedInt32Array packed_texture_map_indices = geometry_decomposition->load_indices(p_g4mf_state);
						Vector<PackedInt32Array> poly_cell_texture_map_indices;
						int64_t tex_index_index = 0;
						const int64_t value_count = poly_mesh->get_poly_cell_texture_map_values().size();
						while (tex_index_index < packed_texture_map_indices.size()) {
							PackedInt32Array this_cell_texture_map_indices;
							const int vertex_count = packed_texture_map_indices[tex_index_index];
							ERR_FAIL_COND_V_MSG(vertex_count < 0 || vertex_count > packed_texture_map_indices.size() - tex_index_index - 1, Ref<ArrayPolyMesh4D>(), "G4MFMeshSurface4D: Invalid packed texture map binding count.");
							this_cell_texture_map_indices.resize(vertex_count);
							tex_index_index += 1;
							for (int i = 0; i < vertex_count; i++) {
								const int32_t value_index = packed_texture_map_indices[tex_index_index];
								ERR_FAIL_INDEX_V(value_index, value_count, Ref<ArrayPolyMesh4D>());
								this_cell_texture_map_indices.set(i, value_index);
								tex_index_index += 1;
							}
							poly_cell_texture_map_indices.append(this_cell_texture_map_indices);
						}
						poly_mesh->set_poly_cell_texture_map_indices(poly_cell_texture_map_indices);
						break;
					}
				}
			}
		}
		if (_seams_accessor_index >= 0) {
			poly_mesh->set_seam_face_indices_bind(load_seam_indices(p_g4mf_state));
		}
	}
	const bool is_valid = poly_mesh->is_mesh_data_valid();
	ERR_FAIL_COND_V_MSG(!is_valid, Ref<ArrayPolyMesh4D>(), "G4MFMeshSurface4D: The mesh data is not valid. Returning an empty mesh instead.");
	if (poly_mesh->get_poly_cell_indices().size() > 1 && !poly_mesh->get_poly_cell_indices()[1].is_empty()) {
		poly_mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
	}
	if (_material_index >= 0) {
		const TypedArray<G4MFMaterial4D> materials = p_g4mf_state->get_g4mf_materials();
		ERR_FAIL_INDEX_V(_material_index, materials.size(), poly_mesh);
		const Ref<G4MFMaterial4D> g4mf_material = materials[_material_index];
		ERR_FAIL_COND_V(g4mf_material.is_null(), poly_mesh);
		Ref<TetraMaterial4D> tetra_material = g4mf_material->generate_tetra_material(p_g4mf_state);
		poly_mesh->set_material(tetra_material);
	}
	return poly_mesh;
}

Ref<ArrayTetraMesh4D> G4MFMeshSurface4D::generate_tetra_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const {
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
	// their data, so the values and indices can be loaded without any conversion.
	if (_normals_binding.is_valid()) {
		tetra_mesh->set_normal_values(_normals_binding->load_values_as_vector4s(p_g4mf_state));
		if (_normals_binding->get_simplexes_accessor_index() >= 0) {
			const PackedInt32Array normal_indices = _normals_binding->load_simplex_indices(p_g4mf_state);
			if (!normal_indices.is_empty()) {
				tetra_mesh->set_simplex_cell_normal_indices(normal_indices);
			}
		}
	}
	if (_texture_map_binding.is_valid()) {
		tetra_mesh->set_texture_map_values(_texture_map_binding->load_values_as_vector3s(p_g4mf_state));
		if (_texture_map_binding->get_simplexes_accessor_index() >= 0) {
			const PackedInt32Array texture_map_indices = _texture_map_binding->load_simplex_indices(p_g4mf_state);
			if (!texture_map_indices.is_empty()) {
				tetra_mesh->set_simplex_cell_texture_map_indices(texture_map_indices);
			}
		}
	}
	const bool is_valid = tetra_mesh->is_mesh_data_valid();
	ERR_FAIL_COND_V_MSG(!is_valid, Ref<ArrayTetraMesh4D>(), "G4MFMeshSurface4D.generate_tetra_mesh_surface: The mesh data is not valid. Returning an empty mesh instead.");
	if (_material_index >= 0) {
		const TypedArray<G4MFMaterial4D> materials = p_g4mf_state->get_g4mf_materials();
		ERR_FAIL_INDEX_V(_material_index, materials.size(), tetra_mesh);
		const Ref<G4MFMaterial4D> g4mf_material = materials[_material_index];
		ERR_FAIL_COND_V(g4mf_material.is_null(), tetra_mesh);
		Ref<TetraMaterial4D> tetra_material = g4mf_material->generate_tetra_material(p_g4mf_state);
		tetra_mesh->set_material(tetra_material);
	}
	return tetra_mesh;
}

Ref<ArrayWireMesh4D> G4MFMeshSurface4D::generate_wire_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const {
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
		Ref<WireMaterial4D> wire_material = g4mf_material->generate_wire_material(p_g4mf_state);
		wire_mesh->set_material(wire_material);
	}
	return wire_mesh;
}

void G4MFMeshSurface4D::_convert_poly_mesh_surface_for_state(const Ref<G4MFState4D> &p_g4mf_state, const Ref<PolyMesh4D> &p_poly_mesh, const bool p_deduplicate) {
	const Vector<Vector<PackedInt32Array>> separated_geometry = p_poly_mesh->get_poly_cell_indices();
	if (!separated_geometry.is_empty()) {
		convert_separated_geometry_into_packed(p_g4mf_state, separated_geometry, p_deduplicate);
	}
	// The runtime mesh classes store indexed values, which is also how G4MF stores
	// this data, so the indices can be packed and encoded without any conversion.
	const Vector<PackedInt32Array> poly_cell_normal_indices = p_poly_mesh->get_poly_cell_normal_indices();
	if (!poly_cell_normal_indices.is_empty()) {
		PackedInt32Array geom1_vertex_normal_indices;
		bool has_normal_data = false;
		for (int cell_index = 0; cell_index < poly_cell_normal_indices.size(); cell_index++) {
			const PackedInt32Array &this_cell_normal_indices = poly_cell_normal_indices[cell_index];
			has_normal_data = has_normal_data || !this_cell_normal_indices.is_empty();
			// Zero counts preserve the positions of cells with no binding data.
			geom1_vertex_normal_indices.append(this_cell_normal_indices.size());
			geom1_vertex_normal_indices.append_array(this_cell_normal_indices);
		}
		if (has_normal_data) {
			if (_normals_binding.is_null()) {
				_normals_binding.instantiate();
			}
			const int indices_accessor_index = G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, geom1_vertex_normal_indices, p_deduplicate);
			ERR_FAIL_COND(indices_accessor_index < 0);
			Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_decomposition;
			geometry_decomposition.instantiate();
			geometry_decomposition->set_geometry_dimension(3);
			geometry_decomposition->set_indices_accessor_index(indices_accessor_index);
			TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_decompositions = _normals_binding->get_geometry_bindings();
			geometry_decompositions.append(geometry_decomposition);
			_normals_binding->set_geometry_bindings(geometry_decompositions);
		}
	}
	const Vector<PackedInt32Array> poly_cell_texture_map_indices = p_poly_mesh->get_poly_cell_texture_map_indices();
	if (!poly_cell_texture_map_indices.is_empty()) {
		PackedInt32Array geom1_vertex_texture_map_indices;
		bool has_texture_map_data = false;
		for (int cell_index = 0; cell_index < poly_cell_texture_map_indices.size(); cell_index++) {
			const PackedInt32Array &this_cell_texture_map_indices = poly_cell_texture_map_indices[cell_index];
			has_texture_map_data = has_texture_map_data || !this_cell_texture_map_indices.is_empty();
			// Zero counts preserve the positions of cells with no binding data.
			geom1_vertex_texture_map_indices.append(this_cell_texture_map_indices.size());
			geom1_vertex_texture_map_indices.append_array(this_cell_texture_map_indices);
		}
		if (has_texture_map_data) {
			if (_texture_map_binding.is_null()) {
				_texture_map_binding.instantiate();
			}
			const int indices_accessor_index = G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, geom1_vertex_texture_map_indices, p_deduplicate);
			ERR_FAIL_COND(indices_accessor_index < 0);
			Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_decomposition;
			geometry_decomposition.instantiate();
			geometry_decomposition->set_geometry_dimension(3);
			geometry_decomposition->set_indices_accessor_index(indices_accessor_index);
			TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_decompositions = _texture_map_binding->get_geometry_bindings();
			geometry_decompositions.append(geometry_decomposition);
			_texture_map_binding->set_geometry_bindings(geometry_decompositions);
		}
	}
}

void G4MFMeshSurface4D::_convert_tetra_mesh_surface_for_state(const Ref<G4MFState4D> &p_g4mf_state, const Ref<TetraMesh4D> &p_tetra_mesh, const bool p_deduplicate) {
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
		_normals_binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, simplex_normal_indices, p_deduplicate));
	}
	const PackedInt32Array simplex_texture_map_indices = p_tetra_mesh->get_simplex_cell_texture_map_indices();
	if (!simplex_texture_map_indices.is_empty()) {
		if (_texture_map_binding.is_null()) {
			_texture_map_binding.instantiate();
		}
		_texture_map_binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(p_g4mf_state, simplex_texture_map_indices, p_deduplicate));
	}
}

Ref<G4MFMeshSurface4D> G4MFMeshSurface4D::convert_mesh_surface_for_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh, const bool p_deduplicate) {
	Ref<G4MFMeshSurface4D> surface;
	surface.instantiate();
	// Convert the material.
	const Ref<Material4D> material = p_mesh->get_material();
	if (material.is_valid() && !material->is_default_material()) {
		const int material_index = G4MFMaterial4D::convert_material_into_state(p_g4mf_state, material, p_deduplicate);
		surface->set_material_index(material_index);
		if (material_index < 0) {
			ERR_PRINT("G4MFMeshSurface4D: Failed to encode material into G4MFState4D.");
		}
	}
	bool export_edges = true;
	const Ref<PolyMesh4D> poly_mesh = p_mesh;
	if (poly_mesh.is_valid()) {
		// For poly meshes, convert both poly cell geometry and tetrahedral simplex cells into accessors.
		// When simplex bindings exist, their values are a superset of the poly cell values,
		// so both representations can share one values accessor per binding.
		surface->_convert_poly_mesh_surface_for_state(p_g4mf_state, poly_mesh, p_deduplicate);
		surface->_convert_tetra_mesh_surface_for_state(p_g4mf_state, poly_mesh, p_deduplicate);
		// Don't return here: Always convert edges for poly meshes.
	} else {
		// For tetra meshes, convert tetrahedral simplex cells into an accessor.
		const Ref<TetraMesh4D> tetra_mesh = p_mesh;
		if (tetra_mesh.is_valid()) {
			surface->_convert_tetra_mesh_surface_for_state(p_g4mf_state, tetra_mesh, p_deduplicate);
			// For BoxTetraMesh4D in polytope mode, use its explicitly defined edges.
			// Other meshes can skip saving this and rely on implicitly calculated ones.
			const Ref<BoxTetraMesh4D> box_tetra_mesh = p_mesh;
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
		PackedVector4Array normal_values = p_mesh->get_normal_values();
		if (normal_values.is_empty() && poly_mesh.is_valid()) {
			// A poly mesh can have geometry bindings but no exposed simplexes.
			normal_values = poly_mesh->get_poly_cell_normal_values();
		}
		const int normal_values_accessor = G4MFAccessor4D::encode_new_accessor_from_vector4s(p_g4mf_state, normal_values, p_deduplicate);
		ERR_FAIL_COND_V(normal_values_accessor < 0, surface);
		surface->_normals_binding->set_values_accessor_index(normal_values_accessor);
	}
	if (surface->_texture_map_binding.is_valid()) {
		PackedVector3Array texture_map_values = p_mesh->get_texture_map_values();
		if (texture_map_values.is_empty() && poly_mesh.is_valid()) {
			texture_map_values = poly_mesh->get_poly_cell_texture_map_values();
		}
		const int texture_map_values_accessor = G4MFAccessor4D::encode_new_accessor_from_vector3s(p_g4mf_state, texture_map_values, p_deduplicate);
		ERR_FAIL_COND_V(texture_map_values_accessor < 0, surface);
		surface->_texture_map_binding->set_values_accessor_index(texture_map_values_accessor);
	}
	if (!export_edges) {
		return surface;
	}
	// Convert edges into an accessor.
	const PackedInt32Array edge_indices = p_mesh->get_edge_indices();
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
	ClassDB::bind_method(D_METHOD("generate_tetra_mesh_surface", "g4mf_state", "vertices"), &G4MFMeshSurface4D::generate_tetra_mesh_surface);
	ClassDB::bind_method(D_METHOD("generate_wire_mesh_surface", "g4mf_state", "vertices"), &G4MFMeshSurface4D::generate_wire_mesh_surface);
	ClassDB::bind_static_method("G4MFMeshSurface4D", D_METHOD("convert_mesh_surface_for_state", "g4mf_state", "mesh", "deduplicate"), &G4MFMeshSurface4D::convert_mesh_surface_for_state, DEFVAL(true));

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
}
