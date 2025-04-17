#include "g4mf_mesh_4d.h"

#include "../g4mf_state_4d.h"

bool G4MFMesh4D::_can_generate_tetra_meshes_for_all_surfaces() const {
	for (int i = 0; i < _surfaces.size(); i++) {
		const Ref<G4MFMeshSurface4D> surface = _surfaces[i];
		if (surface->get_cells_accessor_index() < 0) {
			return false;
		}
	}
	return true;
}

Ref<ArrayTetraMesh4D> G4MFMesh4D::_generate_tetra_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const int p_surface) const {
	Ref<ArrayTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	ERR_FAIL_INDEX_V(p_surface, _surfaces.size(), tetra_mesh);
	const Ref<G4MFMeshSurface4D> surface = _surfaces[p_surface];
	return surface->generate_tetra_mesh_surface(p_g4mf_state);
}

Ref<ArrayWireMesh4D> G4MFMesh4D::_generate_wire_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const int p_surface) const {
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	ERR_FAIL_INDEX_V(p_surface, _surfaces.size(), wire_mesh);
	const Ref<G4MFMeshSurface4D> surface = _surfaces[p_surface];
	return surface->generate_wire_mesh_surface(p_g4mf_state);
}

bool G4MFMesh4D::is_equal_exact(const Ref<G4MFMesh4D> &p_other) const {
	const TypedArray<G4MFMeshSurface4D> other_surfaces = p_other->get_surfaces();
	const int surfaces_count = _surfaces.size();
	if (surfaces_count != other_surfaces.size()) {
		return false;
	}
	for (int i = 0; i < surfaces_count; i++) {
		const Ref<G4MFMeshSurface4D> this_surface = _surfaces[i];
		const Ref<G4MFMeshSurface4D> other_surface = other_surfaces[i];
		if (this_surface.is_null() || other_surface.is_null()) {
			return false;
		}
		if (!this_surface->is_equal_exact(other_surface)) {
			return false;
		}
	}
	return true;
}

Ref<Mesh4D> G4MFMesh4D::generate_mesh(const Ref<G4MFState4D> &p_g4mf_state, const bool p_force_wireframe) const {
	const int surface_count = _surfaces.size();
	ERR_FAIL_COND_V_MSG(surface_count == 0, Ref<Mesh4D>(), "G4MFMesh4D.generate_mesh: No surfaces defined for mesh.");
	if (surface_count > 1) {
		WARN_PRINT("G4MFMesh4D.generate_mesh: Godot 4D only supports one surface per mesh. These will be merged into one surface.");
	}
	const bool use_tetra_mesh = !p_force_wireframe && _can_generate_tetra_meshes_for_all_surfaces();
	if (use_tetra_mesh) {
		Ref<ArrayTetraMesh4D> tetra_mesh = _generate_tetra_mesh_surface(p_g4mf_state, 0);
		for (int i = 1; i < surface_count; i++) {
			Ref<ArrayTetraMesh4D> next_tetra_mesh = _generate_tetra_mesh_surface(p_g4mf_state, i);
			tetra_mesh->merge_with(next_tetra_mesh);
		}
		return tetra_mesh;
	}
	Ref<ArrayWireMesh4D> wire_mesh = _generate_wire_mesh_surface(p_g4mf_state, 0);
	for (int i = 1; i < surface_count; i++) {
		Ref<ArrayWireMesh4D> next_wire_mesh = _generate_wire_mesh_surface(p_g4mf_state, i);
		wire_mesh->merge_with(next_wire_mesh);
	}
	return wire_mesh;
}

int G4MFMesh4D::convert_mesh_into_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh, const bool p_deduplicate) {
	Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::convert_mesh_surface_for_state(p_g4mf_state, p_mesh, p_deduplicate);
	ERR_FAIL_COND_V_MSG(surface->get_vertices_accessor_index() < 0, -1, "G4MFMesh4D.convert_mesh_into_state: Failed to convert mesh surface.");
	// Prepare a G4MFMesh4D with the surface.
	TypedArray<G4MFMeshSurface4D> surfaces;
	surfaces.append(surface);
	Ref<G4MFMesh4D> g4mf_mesh;
	g4mf_mesh.instantiate();
	g4mf_mesh->set_surfaces(surfaces);
	// Add the G4MFMesh4D to the G4MFState4D.
	TypedArray<G4MFMesh4D> state_meshes = p_g4mf_state->get_g4mf_meshes();
	const int state_mesh_count = state_meshes.size();
	for (int i = 0; i < state_mesh_count; i++) {
		const Ref<G4MFMesh4D> state_mesh = state_meshes[i];
		if (g4mf_mesh->is_equal_exact(state_mesh)) {
			// An identical already exists in state, we can just use that.
			return i;
		}
	}
	state_meshes.append(g4mf_mesh);
	p_g4mf_state->set_g4mf_meshes(state_meshes);
	return state_mesh_count;
}

Ref<G4MFMesh4D> G4MFMesh4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFMesh4D> mesh;
	mesh.instantiate();
	mesh->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("surfaces")) {
		const Array surface_dicts = p_dict["surfaces"];
		const int surface_count = surface_dicts.size();
		mesh->_surfaces.resize(surface_count);
		for (int i = 0; i < surface_count; i++) {
			const Dictionary surface_dict = surface_dicts[i];
			Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::from_dictionary(surface_dict);
			mesh->_surfaces[i] = surface;
		}
	}
	return mesh;
}

Dictionary G4MFMesh4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (!_surfaces.is_empty()) {
		Array surface_dicts;
		const int surface_count = _surfaces.size();
		surface_dicts.resize(surface_count);
		for (int i = 0; i < surface_count; i++) {
			const Ref<G4MFMeshSurface4D> surface = _surfaces[i];
			surface_dicts[i] = surface->to_dictionary();
		}
		dict["surfaces"] = surface_dicts;
	}
	return dict;
}

void G4MFMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_surfaces"), &G4MFMesh4D::get_surfaces);
	ClassDB::bind_method(D_METHOD("set_surfaces", "surfaces"), &G4MFMesh4D::set_surfaces);

	ClassDB::bind_method(D_METHOD("generate_mesh", "g4mf_state", "force_wireframe"), &G4MFMesh4D::generate_mesh, DEFVAL(false));
	ClassDB::bind_static_method("G4MFMesh4D", D_METHOD("convert_mesh_into_state", "g4mf_state", "mesh", "deduplicate"), &G4MFMesh4D::convert_mesh_into_state, DEFVAL(true));

	ClassDB::bind_static_method("G4MFMesh4D", D_METHOD("from_dictionary", "dict"), &G4MFMesh4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMesh4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "surfaces", PROPERTY_HINT_ARRAY_TYPE, "G4MFMeshSurface4D"), "set_surfaces", "get_surfaces");
}
