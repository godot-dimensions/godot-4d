#include "g4mf_mesh_surface_4d.h"

#include "../../mesh/tetra/box_tetra_mesh_4d.h"
#include "../g4mf_state_4d.h"

bool G4MFMeshSurface4D::is_equal_exact(const Ref<G4MFMeshSurface4D> &p_other) const {
	return (_simplexes_accessor_index == p_other->get_simplexes_accessor_index() &&
			_edges_accessor_index == p_other->get_edges_accessor_index() &&
			_polytope_simplexes == p_other->get_polytope_simplexes());
}

void G4MFMeshSurface4D::convert_separated_geometry_into_packed(const Ref<G4MFState4D> &p_g4mf_state, const TypedArray<Array> &p_separated_geometry, const bool p_deduplicate) {
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

TypedArray<Array> G4MFMeshSurface4D::load_geometry_separated(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	TypedArray<Array> all_separated;
	all_separated.resize(_geometry_accessor_indices.size());
	for (int geom_index = 0; geom_index < _geometry_accessor_indices.size(); geom_index++) {
		const int accessor_index = _geometry_accessor_indices[geom_index];
		ERR_FAIL_INDEX_V(accessor_index, state_accessors.size(), Array());
		const Ref<G4MFAccessor4D> accessor = state_accessors[accessor_index];
		const PackedInt32Array packed_geometry_data = accessor->decode_int32s_from_bytes(p_g4mf_state);
		const int64_t packed_geometry_data_size = packed_geometry_data.size();
		// This should be TypedArray<PackedInt32Array> but Godot's type system doesn't allow nested typed arrays.
		Array separated_geometry_data;
		int cell_start = 0;
		while (cell_start < packed_geometry_data_size) {
			const int cell_vertex_count = packed_geometry_data[cell_start];
			ERR_FAIL_COND_V(cell_vertex_count <= 0, Array());
			ERR_FAIL_INDEX_V(cell_start + cell_vertex_count, packed_geometry_data_size, Array());
			cell_start += 1; // Move past the vertex count to the start of the vertex indices.
			PackedInt32Array cell_vertex_indices;
			cell_vertex_indices.resize(cell_vertex_count);
			for (int i = 0; i < cell_vertex_count; i++) {
				cell_vertex_indices.set(i, packed_geometry_data[cell_start + i]);
			}
			separated_geometry_data.append(cell_vertex_indices);
			cell_start += cell_vertex_count;
		}
		all_separated[geom_index] = separated_geometry_data;
	}
	return all_separated;
}

PackedInt32Array G4MFMeshSurface4D::load_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_edges_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_edges_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

PackedInt32Array G4MFMeshSurface4D::load_seam_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_seams_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_seams_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

PackedInt32Array G4MFMeshSurface4D::load_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_simplexes_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_simplexes_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

Ref<ArrayTetraMesh4D> G4MFMeshSurface4D::generate_tetra_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const {
	Ref<ArrayTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_vertices(p_vertices);
	if (_simplexes_accessor_index >= 0) {
		tetra_mesh->set_simplex_cell_indices(load_simplex_indices(p_g4mf_state));
		tetra_mesh->calculate_boundary_normals();
	}
	if (_normals_binding.is_valid()) {
		const PackedInt32Array normal_indices = _normals_binding->load_simplex_indices(p_g4mf_state);
		if (!normal_indices.is_empty()) {
			const PackedVector4Array normal_values = _normals_binding->load_values_as_vector4s(p_g4mf_state);
			PackedVector4Array sampled_normals;
			sampled_normals.resize(normal_indices.size());
			for (int i = 0; i < normal_indices.size(); i++) {
				const int normal_index = normal_indices[i];
				ERR_FAIL_INDEX_V(normal_index, normal_values.size(), tetra_mesh);
				sampled_normals.set(i, normal_values[normal_index]);
			}
			tetra_mesh->set_simplex_cell_vertex_normals(sampled_normals);
		}
	}
	if (_texture_map_binding.is_valid()) {
		const PackedInt32Array texture_map_indices = _texture_map_binding->load_simplex_indices(p_g4mf_state);
		if (!texture_map_indices.is_empty()) {
			const PackedVector3Array texture_map_values = _texture_map_binding->load_values_as_vector3s(p_g4mf_state);
			PackedVector3Array sampled_texture_map;
			sampled_texture_map.resize(texture_map_indices.size());
			for (int i = 0; i < texture_map_indices.size(); i++) {
				const int texture_map_index = texture_map_indices[i];
				ERR_FAIL_INDEX_V(texture_map_index, texture_map_values.size(), tetra_mesh);
				sampled_texture_map.set(i, texture_map_values[texture_map_index]);
			}
			tetra_mesh->set_simplex_cell_texture_map(sampled_texture_map);
		}
	}
	const bool is_valid = tetra_mesh->is_mesh_data_valid();
	ERR_FAIL_COND_V_MSG(!is_valid, Ref<ArrayWireMesh4D>(), "G4MFMeshSurface4D.generate_tetra_mesh_surface: The mesh data is not valid. Returning an empty mesh instead.");
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
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_vertices(p_vertices);
	if (_edges_accessor_index >= 0) {
		wire_mesh->set_edge_indices(load_edge_indices(p_g4mf_state));
	} else if (_simplexes_accessor_index >= 0) {
		// Calculate edges from simplex cells.
		const PackedInt32Array simplex_indices = load_simplex_indices(p_g4mf_state);
		const PackedInt32Array edge_indices = TetraMesh4D::calculate_edge_indices_from_simplex_cell_indices(simplex_indices);
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
	// Convert tetrahedral simplex cells into an accessor.
	const Ref<TetraMesh4D> tetra_mesh = p_mesh;
	if (tetra_mesh.is_valid()) {
		const PackedInt32Array simplex_indices = tetra_mesh->get_simplex_cell_indices();
		if (!simplex_indices.is_empty()) {
			Array simplex_indices_variants;
			simplex_indices_variants.resize(simplex_indices.size());
			for (int i = 0; i < simplex_indices.size(); i++) {
				simplex_indices_variants[i] = simplex_indices[i];
			}
			const String simplex_prim_type = G4MFAccessor4D::minimal_component_type_for_int32s(simplex_indices);
			const int simplexes_accessor = G4MFAccessor4D::encode_new_accessor_from_variants(p_g4mf_state, simplex_indices_variants, simplex_prim_type, 4, p_deduplicate);
			ERR_FAIL_COND_V_MSG(simplexes_accessor < 0, surface, "G4MFMeshSurface4D: Failed to encode simplex cells into G4MFState4D.");
			surface->set_simplexes_accessor_index(simplexes_accessor);
		}
		const PackedVector4Array simplex_vertex_normals = tetra_mesh->get_simplex_cell_vertex_normals();
		if (!simplex_vertex_normals.is_empty()) {
			Ref<G4MFMeshSurfaceBinding4D> normals_binding;
			normals_binding.instantiate();
			PackedInt64Array normal_indices;
			normal_indices.resize(simplex_vertex_normals.size());
			if (p_deduplicate) {
				// TODO: Once we have blend shapes that can affect vertex normals, use
				// a smarter deduplication method that takes this into account.
				PackedVector4Array normal_values;
				for (int i = 0; i < simplex_vertex_normals.size(); i++) {
					const Vector4 &normal_value = simplex_vertex_normals[i];
					const int64_t normal_index = normal_values.find(normal_value);
					if (normal_index == -1) {
						normal_values.append(normal_value);
						normal_indices.set(i, normal_values.size() - 1);
					} else {
						normal_indices.set(i, normal_index);
					}
				}
				normals_binding->set_values_accessor_index(G4MFAccessor4D::encode_new_accessor_from_vector4s(p_g4mf_state, normal_values, p_deduplicate));
			} else {
				normals_binding->set_values_accessor_index(G4MFAccessor4D::encode_new_accessor_from_vector4s(p_g4mf_state, simplex_vertex_normals, p_deduplicate));
				for (int i = 0; i < simplex_vertex_normals.size(); i++) {
					normal_indices.set(i, i);
				}
			}
			normals_binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int64s(p_g4mf_state, normal_indices, p_deduplicate));
			surface->set_normals_binding(normals_binding);
		}
		const PackedVector3Array simplex_texture_map = tetra_mesh->get_simplex_cell_texture_map();
		if (!simplex_texture_map.is_empty()) {
			Ref<G4MFMeshSurfaceBinding4D> texture_map_binding;
			texture_map_binding.instantiate();
			PackedInt64Array texture_map_indices;
			texture_map_indices.resize(simplex_texture_map.size());
			if (p_deduplicate) {
				// TODO: Once we have blend shapes that can affect vertex texture maps, use
				// a smarter deduplication method that takes this into account.
				PackedVector3Array texture_map_values;
				for (int i = 0; i < simplex_texture_map.size(); i++) {
					const Vector3 &texture_map_value = simplex_texture_map[i];
					const int64_t texture_map_index = texture_map_values.find(texture_map_value);
					if (texture_map_index == -1) {
						texture_map_values.append(texture_map_value);
						texture_map_indices.set(i, texture_map_values.size() - 1);
					} else {
						texture_map_indices.set(i, texture_map_index);
					}
				}
				texture_map_binding->set_values_accessor_index(G4MFAccessor4D::encode_new_accessor_from_vector3s(p_g4mf_state, texture_map_values, p_deduplicate));
			} else {
				texture_map_binding->set_values_accessor_index(G4MFAccessor4D::encode_new_accessor_from_vector3s(p_g4mf_state, simplex_texture_map, p_deduplicate));
				for (int i = 0; i < simplex_texture_map.size(); i++) {
					texture_map_indices.set(i, i);
				}
			}
			texture_map_binding->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int64s(p_g4mf_state, texture_map_indices, p_deduplicate));
			surface->set_texture_map_binding(texture_map_binding);
		}
		// For ArrayTetraMesh4D, the only edges are those implicitly calculated from simplex cells.
		// However, for other tetra meshes like BoxTetraMesh4D, use its explicitly defined edges.
		const Ref<BoxTetraMesh4D> box_tetra_mesh = p_mesh;
		if (box_tetra_mesh.is_valid() && box_tetra_mesh->get_tetra_decomp() == BoxTetraMesh4D::BOX_TETRA_DECOMP_48_CELL_POLYTOPE) {
			surface->set_polytope_simplexes(true);
			// Don't return here, so that we keep the edge indices code below.
		} else {
			return surface;
		}
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
		dict["geometry"] = _geometry_accessor_indices;
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
	ClassDB::bind_method(D_METHOD("convert_separated_geometry_into_packed", "g4mf_state", "separated_geometry", "deduplicate"), &G4MFMeshSurface4D::convert_separated_geometry_into_packed, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("load_geometry_separated", "g4mf_state"), &G4MFMeshSurface4D::load_geometry_separated);
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
