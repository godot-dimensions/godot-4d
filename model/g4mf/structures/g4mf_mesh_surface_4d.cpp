#include "g4mf_mesh_surface_4d.h"

#include "../../tetra/box_tetra_mesh_4d.h"
#include "../g4mf_state_4d.h"

bool G4MFMeshSurface4D::is_equal_exact(const Ref<G4MFMeshSurface4D> &p_other) const {
	return (_cells_accessor_index == p_other->get_cells_accessor_index() &&
			_edges_accessor_index == p_other->get_edges_accessor_index() &&
			_polytope_cells == p_other->get_polytope_cells());
}

PackedInt32Array G4MFMeshSurface4D::load_cell_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_accessors();
	ERR_FAIL_INDEX_V(_cells_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_cells_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

PackedInt32Array G4MFMeshSurface4D::load_edge_indices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_accessors();
	ERR_FAIL_INDEX_V(_edges_accessor_index, state_accessors.size(), PackedInt32Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_edges_accessor_index];
	return accessor->decode_int32s_from_bytes(p_g4mf_state);
}

Ref<ArrayTetraMesh4D> G4MFMeshSurface4D::generate_tetra_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array &p_vertices) const {
	Ref<ArrayTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_vertices(p_vertices);
	if (_cells_accessor_index >= 0) {
		tetra_mesh->set_cell_indices(load_cell_indices(p_g4mf_state));
		tetra_mesh->calculate_normals();
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
	} else if (_cells_accessor_index >= 0) {
		// Calculate edges from cells.
		const PackedInt32Array cell_indices = load_cell_indices(p_g4mf_state);
		const PackedInt32Array edge_indices = TetraMesh4D::calculate_edge_indices_from_cell_indices(cell_indices);
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
	// Convert the vertices into an accessor.
	//const PackedVector4Array vertices = p_mesh->get_vertices();
	//ERR_FAIL_COND_V_MSG(vertices.is_empty(), surface, "G4MFMeshSurface4D: Mesh4D has no vertices, cannot convert to a surface.");
	//Array vertices_variants;
	//vertices_variants.resize(vertices.size());
	//for (int i = 0; i < vertices.size(); i++) {
	//	vertices_variants[i] = vertices[i];
	//}
	//const String vert_prim_type = G4MFAccessor4D::minimal_primitive_type_for_vector4s(vertices);
	//const int vertices_accessor = G4MFAccessor4D::encode_new_accessor_from_variants(p_g4mf_state, vertices_variants, vert_prim_type, 4, p_deduplicate);
	//const int vertices_accessor = G4MFAccessor4D::encode_new_accessor_from_vector4s(p_g4mf_state, vertices, p_deduplicate);
	//ERR_FAIL_COND_V_MSG(vertices_accessor < 0, surface, "G4MFMeshSurface4D: Failed to encode vertices into G4MFState4D.");
	//surface->set_vertices_accessor_index(vertices_accessor);
	// Convert the material.
	if (p_material.is_valid() && !p_material->is_default_material()) {
		const int material_index = G4MFMaterial4D::convert_material_into_state(p_g4mf_state, p_material, p_deduplicate);
		surface->set_material_index(material_index);
		if (material_index < 0) {
			ERR_PRINT("G4MFMeshSurface4D: Failed to encode material into G4MFState4D.");
		}
	}
	// Convert tetrahedral cells into an accessor.
	const Ref<TetraMesh4D> tetra_mesh = p_mesh;
	if (tetra_mesh.is_valid()) {
		const PackedInt32Array cell_indices = tetra_mesh->get_cell_indices();
		if (!cell_indices.is_empty()) {
			Array cell_indices_variants;
			cell_indices_variants.resize(cell_indices.size());
			for (int i = 0; i < cell_indices.size(); i++) {
				cell_indices_variants[i] = cell_indices[i];
			}
			const String cell_prim_type = G4MFAccessor4D::minimal_primitive_type_for_int32s(cell_indices);
			const int cells_accessor = G4MFAccessor4D::encode_new_accessor_from_variants(p_g4mf_state, cell_indices_variants, cell_prim_type, 4, p_deduplicate);
			ERR_FAIL_COND_V_MSG(cells_accessor < 0, surface, "G4MFMeshSurface4D: Failed to encode cells into G4MFState4D.");
			surface->set_cells_accessor_index(cells_accessor);
		}
		// For ArrayTetraMesh4D, the only edges are those implicitly calculated from cells.
		// However, for other tetra meshes like BoxTetraMesh4D, use its explicitly defined edges.
		const Ref<BoxTetraMesh4D> box_tetra_mesh = p_mesh;
		if (box_tetra_mesh.is_valid() && box_tetra_mesh->get_tetra_decomp() == BoxTetraMesh4D::BOX_TETRA_DECOMP_48_CELL_POLYTOPE) {
			surface->set_polytope_cells(true);
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
	const String edge_prim_type = G4MFAccessor4D::minimal_primitive_type_for_int32s(edge_indices);
	const int edges_accessor = G4MFAccessor4D::encode_new_accessor_from_variants(p_g4mf_state, edge_indices_variants, edge_prim_type, 2, p_deduplicate);
	ERR_FAIL_COND_V_MSG(edges_accessor < 0, surface, "G4MFMeshSurface4D: Failed to encode edges into G4MFState4D.");
	surface->set_edges_accessor_index(edges_accessor);
	return surface;
}

Ref<G4MFMeshSurface4D> G4MFMeshSurface4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFMeshSurface4D> surface;
	surface.instantiate();
	surface->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("cells")) {
		surface->set_cells_accessor_index(p_dict["cells"]);
	}
	if (p_dict.has("edges")) {
		surface->set_edges_accessor_index(p_dict["edges"]);
	}
	if (p_dict.has("material")) {
		surface->set_material_index(p_dict["material"]);
	}
	if (p_dict.has("polytopeCells")) {
		surface->set_polytope_cells(p_dict["polytopeCells"]);
	}
	return surface;
}

Dictionary G4MFMeshSurface4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_cells_accessor_index >= 0) {
		dict["cells"] = _cells_accessor_index;
	}
	if (_edges_accessor_index >= 0) {
		dict["edges"] = _edges_accessor_index;
	}
	if (_material_index >= 0) {
		dict["material"] = _material_index;
	}
	if (_polytope_cells) {
		dict["polytopeCells"] = _polytope_cells;
	}
	return dict;
}

void G4MFMeshSurface4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_cells_accessor_index"), &G4MFMeshSurface4D::get_cells_accessor_index);
	ClassDB::bind_method(D_METHOD("set_cells_accessor_index", "cells_accessor_index"), &G4MFMeshSurface4D::set_cells_accessor_index);
	ClassDB::bind_method(D_METHOD("get_edges_accessor_index"), &G4MFMeshSurface4D::get_edges_accessor_index);
	ClassDB::bind_method(D_METHOD("set_edges_accessor_index", "edges_accessor_index"), &G4MFMeshSurface4D::set_edges_accessor_index);
	ClassDB::bind_method(D_METHOD("get_material_index"), &G4MFMeshSurface4D::get_material_index);
	ClassDB::bind_method(D_METHOD("set_material_index", "material_index"), &G4MFMeshSurface4D::set_material_index);
	ClassDB::bind_method(D_METHOD("get_polytope_cells"), &G4MFMeshSurface4D::get_polytope_cells);
	ClassDB::bind_method(D_METHOD("set_polytope_cells", "polytope_cells"), &G4MFMeshSurface4D::set_polytope_cells);

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMeshSurface4D::is_equal_exact);
	ClassDB::bind_method(D_METHOD("load_cell_indices", "g4mf_state"), &G4MFMeshSurface4D::load_cell_indices);
	ClassDB::bind_method(D_METHOD("load_edge_indices", "g4mf_state"), &G4MFMeshSurface4D::load_edge_indices);
	ClassDB::bind_method(D_METHOD("generate_tetra_mesh_surface", "g4mf_state", "vertices"), &G4MFMeshSurface4D::generate_tetra_mesh_surface);
	ClassDB::bind_method(D_METHOD("generate_wire_mesh_surface", "g4mf_state", "vertices"), &G4MFMeshSurface4D::generate_wire_mesh_surface);
	ClassDB::bind_static_method("G4MFMeshSurface4D", D_METHOD("convert_mesh_surface_for_state", "g4mf_state", "mesh", "material", "deduplicate"), &G4MFMeshSurface4D::convert_mesh_surface_for_state, DEFVAL(true));

	ClassDB::bind_static_method("G4MFMeshSurface4D", D_METHOD("from_dictionary", "dict"), &G4MFMeshSurface4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMeshSurface4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "cells_accessor_index"), "set_cells_accessor_index", "get_cells_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "edges_accessor_index"), "set_edges_accessor_index", "get_edges_accessor_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material_index"), "set_material_index", "get_material_index");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "polytope_cells"), "set_polytope_cells", "get_polytope_cells");
}
