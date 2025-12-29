#include "g4mf_mesh_surface_4d.h"

#include "../../mesh/tetra/box_tetra_mesh_4d.h"
#include "../g4mf_state_4d.h"

bool G4MFMeshSurface4D::is_equal_exact(const Ref<G4MFMeshSurface4D> &p_other) const {
	return (_simplexes_accessor_index == p_other->get_simplexes_accessor_index() &&
			_edges_accessor_index == p_other->get_edges_accessor_index() &&
			_polytope_simplexes == p_other->get_polytope_simplexes());
}

PackedInt32Array G4MFMeshSurface4D::load_simplex_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_simplexes_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_simplexes_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

PackedInt32Array G4MFMeshSurface4D::load_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_edges_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_edges_accessor_index];
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
	const bool is_valid = tetra_mesh->is_mesh_data_valid();
	ERR_FAIL_COND_V_MSG(!is_valid, Ref<ArrayWireMesh4D>(), "G4MFMeshSurface4D: The mesh data is not valid. Returning an empty mesh instead.");
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
	ERR_FAIL_COND_V_MSG(!is_valid, Ref<ArrayWireMesh4D>(), "G4MFMeshSurface4D: The mesh data is not valid. Returning an empty mesh instead.");
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

Ref<G4MFMeshSurface4D> G4MFMeshSurface4D::convert_mesh_surface_for_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh, const Ref<Material4D> &p_material, const bool p_deduplicate) {
	Ref<G4MFMeshSurface4D> surface;
	surface.instantiate();
	// Convert the material.
	if (p_material.is_valid() && !p_material->is_default_material()) {
		const int material_index = G4MFMaterial4D::convert_material_into_state(p_g4mf_state, p_material, p_deduplicate);
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
	if (p_dict.has("material")) {
		surface->set_material_index(p_dict["material"]);
	}
	if (p_dict.has("polytopeSimplexes")) {
		surface->set_polytope_simplexes(p_dict["polytopeSimplexes"]);
	}
	if (p_dict.has("simplexes")) {
		surface->set_simplexes_accessor_index(p_dict["simplexes"]);
	}
	return surface;
}

Dictionary G4MFMeshSurface4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_simplexes_accessor_index >= 0) {
		dict["simplexes"] = _simplexes_accessor_index;
	}
	if (_edges_accessor_index >= 0) {
		dict["edges"] = _edges_accessor_index;
	}
	if (_material_index >= 0) {
		dict["material"] = _material_index;
	}
	if (_polytope_simplexes) {
		dict["polytopeSimplexes"] = _polytope_simplexes;
	}
	return dict;
}

void G4MFMeshSurface4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_simplexes_accessor_index"), &G4MFMeshSurface4D::get_simplexes_accessor_index);
	ClassDB::bind_method(D_METHOD("set_simplexes_accessor_index", "simplexes_accessor_index"), &G4MFMeshSurface4D::set_simplexes_accessor_index);
	ClassDB::bind_method(D_METHOD("get_edges_accessor_index"), &G4MFMeshSurface4D::get_edges_accessor_index);
	ClassDB::bind_method(D_METHOD("set_edges_accessor_index", "edges_accessor_index"), &G4MFMeshSurface4D::set_edges_accessor_index);
	ClassDB::bind_method(D_METHOD("get_material_index"), &G4MFMeshSurface4D::get_material_index);
	ClassDB::bind_method(D_METHOD("set_material_index", "material_index"), &G4MFMeshSurface4D::set_material_index);
	ClassDB::bind_method(D_METHOD("get_polytope_simplexes"), &G4MFMeshSurface4D::get_polytope_simplexes);
	ClassDB::bind_method(D_METHOD("set_polytope_simplexes", "polytope_simplexes"), &G4MFMeshSurface4D::set_polytope_simplexes);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMeshSurface4D::is_equal_exact);
	ClassDB::bind_method(D_METHOD("load_simplex_indices", "g4mf_state"), &G4MFMeshSurface4D::load_simplex_indices);
	ClassDB::bind_method(D_METHOD("load_edge_indices", "g4mf_state"), &G4MFMeshSurface4D::load_edge_indices);
	ClassDB::bind_method(D_METHOD("generate_tetra_mesh_surface", "g4mf_state", "vertices"), &G4MFMeshSurface4D::generate_tetra_mesh_surface);
	ClassDB::bind_method(D_METHOD("generate_wire_mesh_surface", "g4mf_state", "vertices"), &G4MFMeshSurface4D::generate_wire_mesh_surface);
	ClassDB::bind_static_method("G4MFMeshSurface4D", D_METHOD("convert_mesh_surface_for_state", "g4mf_state", "mesh", "material", "deduplicate"), &G4MFMeshSurface4D::convert_mesh_surface_for_state, DEFVAL(true));

	ClassDB::bind_static_method("G4MFMeshSurface4D", D_METHOD("from_dictionary", "dict"), &G4MFMeshSurface4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMeshSurface4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "simplexes_accessor_index"), "set_simplexes_accessor_index", "get_simplexes_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "edges_accessor_index"), "set_edges_accessor_index", "get_edges_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material_index"), "set_material_index", "get_material_index");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "polytope_simplexes"), "set_polytope_simplexes", "get_polytope_simplexes");
}
