#include "g4mf_mesh_4d.h"

#include "../g4mf_state_4d.h"

Ref<ArrayTetraMesh4D> G4MFMesh4D::_generate_tetra_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array p_vertices, const int p_surface) const {
	Ref<ArrayTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	ERR_FAIL_INDEX_V(p_surface, _surfaces.size(), tetra_mesh);
	const Ref<G4MFMeshSurface4D> surface = _surfaces[p_surface];
	return surface->generate_tetra_mesh_surface(p_g4mf_state, p_vertices);
}

Ref<ArrayWireMesh4D> G4MFMesh4D::_generate_wire_mesh_surface(const Ref<G4MFState4D> &p_g4mf_state, const PackedVector4Array p_vertices, const int p_surface) const {
	Ref<ArrayWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	ERR_FAIL_INDEX_V(p_surface, _surfaces.size(), wire_mesh);
	const Ref<G4MFMeshSurface4D> surface = _surfaces[p_surface];
	return surface->generate_wire_mesh_surface(p_g4mf_state, p_vertices);
}

bool G4MFMesh4D::can_generate_tetra_meshes_for_all_surfaces() const {
	for (int i = 0; i < _surfaces.size(); i++) {
		const Ref<G4MFMeshSurface4D> surface = _surfaces[i];
		if (surface->get_simplexes_accessor_index() < 0) {
			return false;
		}
	}
	return true;
}

bool G4MFMesh4D::is_equal_exact(const Ref<G4MFMesh4D> &p_other) const {
	const TypedArray<G4MFMeshSurface4D> other_surfaces = p_other->get_surfaces();
	const int surfaces_count = _surfaces.size();
	if (surfaces_count != other_surfaces.size()) {
		return false;
	}
	if (_vertices_accessor_index != p_other->get_vertices_accessor_index()) {
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

PackedVector4Array G4MFMesh4D::load_vertices(const Ref<G4MFState4D> &p_g4mf_state) const {
	TypedArray<G4MFAccessor4D> state_accessors = p_g4mf_state->get_g4mf_accessors();
	ERR_FAIL_INDEX_V(_vertices_accessor_index, state_accessors.size(), PackedVector4Array());
	const Ref<G4MFAccessor4D> accessor = state_accessors[_vertices_accessor_index];
	ERR_FAIL_COND_V(accessor.is_null(), PackedVector4Array());
	Array variants = accessor->decode_variants_from_bytes(p_g4mf_state, Variant::VECTOR4);
	const int variants_size = variants.size();
	PackedVector4Array vertices;
	vertices.resize(variants_size);
	for (int i = 0; i < variants_size; i++) {
		vertices.set(i, (Vector4)variants[i]);
	}
	return vertices;
}

Ref<TetraMesh4D> G4MFMesh4D::import_generate_tetra_mesh(const Ref<G4MFState4D> &p_g4mf_state) const {
	const int surface_count = _surfaces.size();
	ERR_FAIL_COND_V_MSG(surface_count == 0, Ref<Mesh4D>(), "G4MFMesh4D.import_generate_tetra_mesh: No surfaces defined for mesh.");
	if (surface_count > 1) {
		WARN_PRINT("G4MFMesh4D.import_generate_tetra_mesh: Godot 4D only supports one surface per mesh. These will be merged into one surface.");
	}
	const PackedVector4Array vertices = load_vertices(p_g4mf_state);
	ERR_FAIL_COND_V_MSG(vertices.is_empty(), Ref<Mesh4D>(), "G4MFMesh4D.import_generate_tetra_mesh: No vertices found in the mesh, cannot generate mesh.");
	Ref<ArrayTetraMesh4D> tetra_mesh = _generate_tetra_mesh_surface(p_g4mf_state, vertices, 0);
	ERR_FAIL_COND_V_MSG(tetra_mesh.is_null(), tetra_mesh, "G4MFMesh4D.import_generate_tetra_mesh: Failed to generate tetra mesh surface.");
	// TODO: The merge_with function is not ideal for this, since it may result in duplicate vertices.
	for (int i = 1; i < surface_count; i++) {
		Ref<ArrayTetraMesh4D> next_tetra_mesh = _generate_tetra_mesh_surface(p_g4mf_state, vertices, i);
		ERR_FAIL_COND_V_MSG(next_tetra_mesh.is_null(), tetra_mesh, "G4MFMesh4D.import_generate_tetra_mesh: Failed to generate tetra mesh surface for surface index " + String::num(i) + ".");
		tetra_mesh->merge_with(next_tetra_mesh);
	}
	tetra_mesh->set_name(get_name());
	return tetra_mesh;
}

Ref<WireMesh4D> G4MFMesh4D::import_generate_wire_mesh(const Ref<G4MFState4D> &p_g4mf_state) const {
	const int surface_count = _surfaces.size();
	ERR_FAIL_COND_V_MSG(surface_count == 0, Ref<Mesh4D>(), "G4MFMesh4D.import_generate_wire_mesh: No surfaces defined for mesh.");
	if (surface_count > 1) {
		WARN_PRINT("G4MFMesh4D.import_generate_wire_mesh: Godot 4D only supports one surface per mesh. These will be merged into one surface.");
	}
	const PackedVector4Array vertices = load_vertices(p_g4mf_state);
	ERR_FAIL_COND_V_MSG(vertices.is_empty(), Ref<Mesh4D>(), "G4MFMesh4D.import_generate_wire_mesh: No vertices found in the mesh, cannot generate mesh.");
	Ref<ArrayWireMesh4D> wire_mesh = _generate_wire_mesh_surface(p_g4mf_state, vertices, 0);
	ERR_FAIL_COND_V_MSG(wire_mesh.is_null(), wire_mesh, "G4MFMesh4D.import_generate_wire_mesh: Failed to generate wire mesh surface.");
	for (int i = 1; i < surface_count; i++) {
		Ref<ArrayWireMesh4D> next_wire_mesh = _generate_wire_mesh_surface(p_g4mf_state, vertices, i);
		ERR_FAIL_COND_V_MSG(next_wire_mesh.is_null(), wire_mesh, "G4MFMesh4D.import_generate_wire_mesh: Failed to generate wire mesh surface for surface index " + String::num(i) + ".");
		wire_mesh->merge_with(next_wire_mesh);
	}
	wire_mesh->set_name(get_name());
	return wire_mesh;
}

Ref<Mesh4D> G4MFMesh4D::import_generate_mesh(const Ref<G4MFState4D> &p_g4mf_state, const bool p_force_single_surface) const {
	const bool use_tetra_mesh = !p_g4mf_state->get_force_wireframe() && can_generate_tetra_meshes_for_all_surfaces();
	if (use_tetra_mesh) {
		return import_generate_tetra_mesh(p_g4mf_state);
	}
	return import_generate_wire_mesh(p_g4mf_state);
}

int G4MFMesh4D::export_convert_mesh_into_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh, const bool p_deduplicate) {
	const PackedVector4Array vertices = p_mesh->get_vertices();
	ERR_FAIL_COND_V_MSG(vertices.is_empty(), -1, "G4MFMesh4D: Mesh4D has no vertices, cannot convert to a G4MF mesh.");
	const int vertices_accessor = G4MFAccessor4D::encode_new_accessor_from_vector4s(p_g4mf_state, vertices, p_deduplicate);
	ERR_FAIL_COND_V_MSG(vertices_accessor < 0, -1, "G4MFMesh4D: Failed to encode vertices into G4MFState4D.");
	Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::convert_mesh_surface_for_state(p_g4mf_state, p_mesh, p_deduplicate);
	// Prepare a G4MFMesh4D with the surface.
	TypedArray<G4MFMeshSurface4D> surfaces;
	surfaces.append(surface);
	Ref<G4MFMesh4D> g4mf_mesh;
	g4mf_mesh.instantiate();
	g4mf_mesh->set_surfaces(surfaces);
	g4mf_mesh->set_vertices_accessor_index(vertices_accessor);
	// Add the G4MFMesh4D to the G4MFState4D, but check for duplicates first.
	TypedArray<G4MFMesh4D> state_meshes = p_g4mf_state->get_g4mf_meshes();
	const int state_mesh_count = state_meshes.size();
	if (p_deduplicate) {
		for (int i = 0; i < state_mesh_count; i++) {
			const Ref<G4MFMesh4D> state_mesh = state_meshes[i];
			if (g4mf_mesh->is_equal_exact(state_mesh)) {
				// An identical mesh already exists in state, we can just use that.
				return i;
			}
		}
	}
	g4mf_mesh->set_name(p_mesh->get_name());
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
	if (p_dict.has("vertices")) {
		mesh->set_vertices_accessor_index(p_dict["vertices"]);
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
	if (_vertices_accessor_index >= 0) {
		dict["vertices"] = _vertices_accessor_index;
	}
	return dict;
}

void G4MFMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_surfaces"), &G4MFMesh4D::get_surfaces);
	ClassDB::bind_method(D_METHOD("set_surfaces", "surfaces"), &G4MFMesh4D::set_surfaces);

	ClassDB::bind_method(D_METHOD("get_vertices_accessor_index"), &G4MFMesh4D::get_vertices_accessor_index);
	ClassDB::bind_method(D_METHOD("set_vertices_accessor_index", "vertices_accessor_index"), &G4MFMesh4D::set_vertices_accessor_index);

	ClassDB::bind_method(D_METHOD("can_generate_tetra_meshes_for_all_surfaces"), &G4MFMesh4D::can_generate_tetra_meshes_for_all_surfaces);
	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMesh4D::is_equal_exact);

	ClassDB::bind_method(D_METHOD("load_vertices", "g4mf_state"), &G4MFMesh4D::load_vertices);
	ClassDB::bind_method(D_METHOD("import_generate_mesh", "g4mf_state", "force_single_surface"), &G4MFMesh4D::import_generate_mesh, DEFVAL(false));
	ClassDB::bind_static_method("G4MFMesh4D", D_METHOD("export_convert_mesh_into_state", "g4mf_state", "mesh", "deduplicate"), &G4MFMesh4D::export_convert_mesh_into_state, DEFVAL(true));

	ClassDB::bind_static_method("G4MFMesh4D", D_METHOD("from_dictionary", "dict"), &G4MFMesh4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMesh4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "surfaces", PROPERTY_HINT_ARRAY_TYPE, "G4MFMeshSurface4D"), "set_surfaces", "get_surfaces");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "vertices_accessor_index"), "set_vertices_accessor_index", "get_vertices_accessor_index");
}
