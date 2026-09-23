#include "g4mf_mesh_4d.h"

#include "../../mesh/multi_surface_mesh_4d.h"
#include "../g4mf_state_4d.h"

bool G4MFMesh4D::is_equal_exact(const Ref<G4MFMesh4D> &p_other) const {
	if (p_other.is_null()) {
		return false;
	}
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
	PackedVector4Array vertex_positions;
	vertex_positions.resize(variants_size);
	for (int i = 0; i < variants_size; i++) {
		vertex_positions.set(i, (Vector4)variants[i]);
	}
	return vertex_positions;
}

Ref<Mesh4D> G4MFMesh4D::import_generate_new_mesh(const Ref<G4MFState4D> &p_g4mf_state) const {
	const int surface_count = _surfaces.size();
	ERR_FAIL_COND_V_MSG(surface_count == 0, Ref<Mesh4D>(), "G4MFMesh4D: The mesh '" + get_item_name() + "' has no surfaces. The G4MF specification requires at least one surface per mesh.");
	const PackedVector4Array vertex_positions = load_vertices(p_g4mf_state);
	if (surface_count == 1) {
		const Ref<G4MFMeshSurface4D> g4mf_mesh_surface = _surfaces[0];
		const Ref<SingleSurfaceMesh4D> single_surface_mesh = g4mf_mesh_surface->import_generate_mesh_surface(p_g4mf_state, vertex_positions);
		ERR_FAIL_COND_V(single_surface_mesh.is_null(), Ref<Mesh4D>());
		// Prefer the G4MFMesh4D's name over the surface's name if both are defined and we are returning the surface directly.
		const String item_name = get_item_name();
		if (!item_name.is_empty()) {
			single_surface_mesh->set_name(item_name);
		}
		return single_surface_mesh;
	}
	Vector<Ref<SingleSurfaceMesh4D>> surface_meshes;
	bool any_surface_valid = false;
	for (int surface_index = 0; surface_index < surface_count; surface_index++) {
		const Ref<G4MFMeshSurface4D> g4mf_mesh_surface = _surfaces[surface_index];
		const Ref<SingleSurfaceMesh4D> single_surface_mesh = g4mf_mesh_surface->import_generate_mesh_surface(p_g4mf_state, vertex_positions);
		if (single_surface_mesh.is_null()) {
			ERR_PRINT("G4MFMesh4D: Failed to generate mesh for surface '" + g4mf_mesh_surface->get_item_name() + "' index " + String::num_int64(surface_index) + " of mesh '" + get_item_name() + "'.");
			// Don't continue, we must append null to preserve indices of subsequent surfaces.
		} else {
			any_surface_valid = true;
			// Surfaces keep their own names when they have one, otherwise they are named after the mesh.
			if (single_surface_mesh->get_name().is_empty()) {
				single_surface_mesh->set_name(get_item_name() + String("_Surface") + String::num_int64(surface_index));
			}
		}
		surface_meshes.append(single_surface_mesh);
	}
	ERR_FAIL_COND_V_MSG(!any_surface_valid, Ref<Mesh4D>(), "G4MFMesh4D: Failed to generate any valid surfaces for mesh '" + get_item_name() + "'.");
	Ref<MultiSurfaceMesh4D> multi_surface_mesh;
	multi_surface_mesh.instantiate();
	multi_surface_mesh->set_surface_meshes(surface_meshes);
	multi_surface_mesh->set_name(get_item_name());
	return multi_surface_mesh;
}

Ref<Mesh4D> G4MFMesh4D::import_get_or_generate_mesh(const Ref<G4MFState4D> &p_g4mf_state) {
	if (_godot_mesh_4d.is_valid()) {
		return _godot_mesh_4d;
	}
	_godot_mesh_4d = import_generate_new_mesh(p_g4mf_state);
	return _godot_mesh_4d;
}

int G4MFMesh4D::export_convert_mesh_into_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Mesh4D> &p_mesh, const bool p_deduplicate) {
	PackedVector4Array shared_vertices;
	TypedArray<G4MFMeshSurface4D> g4mf_surfaces;
	const Ref<SingleSurfaceMesh4D> single_surface_mesh = p_mesh;
	// A G4MF mesh stores one shared vertices accessor that all of its surfaces reference. Converting a surface
	// appends its vertices to this array and remaps its edges, simplexes, and vertex bindings to reference it.
	// Encode a single-surface mesh directly, or iterate over multiple surfaces if it's a multi-surface mesh.
	if (single_surface_mesh.is_valid()) {
		Ref<G4MFMeshSurface4D> g4mf_surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(p_g4mf_state, single_surface_mesh, shared_vertices, p_deduplicate);
		ERR_FAIL_COND_V_MSG(g4mf_surface.is_null(), -1, "G4MFMesh4D: Failed to convert the mesh '" + p_mesh->get_name() + "' to a G4MF mesh surface.");
		// The mesh and its only surface are the same object, so the name goes on the mesh below, not on both.
		g4mf_surface->set_item_name("");
		g4mf_surfaces.append(g4mf_surface);
	} else {
		const Ref<MultiSurfaceMesh4D> multi_surface_mesh = p_mesh;
		ERR_FAIL_COND_V_MSG(multi_surface_mesh.is_null(), -1, "G4MFMesh4D: Unknown mesh type, cannot convert to a G4MF mesh.");
		const Vector<Ref<SingleSurfaceMesh4D>> &surface_meshes = multi_surface_mesh->get_surface_meshes();
		for (int64_t surface_index = 0; surface_index < surface_meshes.size(); surface_index++) {
			const Ref<SingleSurfaceMesh4D> this_surface_mesh = surface_meshes[surface_index];
			if (this_surface_mesh.is_null()) {
				continue; // Null entries are allowed while editing a MultiSurfaceMesh4D, and there is nothing to export for them.
			}
			Ref<G4MFMeshSurface4D> g4mf_surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(p_g4mf_state, this_surface_mesh, shared_vertices, p_deduplicate);
			ERR_FAIL_COND_V_MSG(g4mf_surface.is_null(), -1, "G4MFMesh4D: Failed to convert surface " + itos(surface_index) + " of the mesh '" + p_mesh->get_name() + "' to a G4MF mesh surface.");
			g4mf_surfaces.append(g4mf_surface);
		}
	}
	// Encode the shared vertices accumulated across all surfaces above, which is what the
	// surfaces' own indices (edges, simplexes, vertex bindings) were remapped to reference.
	ERR_FAIL_COND_V_MSG(shared_vertices.is_empty(), -1, "G4MFMesh4D: Mesh4D has no vertices, cannot convert to a G4MF mesh.");
	const int vertices_accessor = G4MFAccessor4D::encode_new_accessor_from_vector4s(p_g4mf_state, shared_vertices, p_deduplicate);
	ERR_FAIL_COND_V_MSG(vertices_accessor < 0, -1, "G4MFMesh4D: Failed to encode vertices into G4MFState4D.");
	// Prepare a G4MFMesh4D with the surface.
	Ref<G4MFMesh4D> g4mf_mesh;
	g4mf_mesh.instantiate();
	g4mf_mesh->set_surfaces(g4mf_surfaces);
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
	g4mf_mesh->set_item_name(p_mesh->get_name());
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

	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &G4MFMesh4D::is_equal_exact);

	ClassDB::bind_method(D_METHOD("load_vertices", "g4mf_state"), &G4MFMesh4D::load_vertices);
	ClassDB::bind_method(D_METHOD("import_generate_new_mesh", "g4mf_state"), &G4MFMesh4D::import_generate_new_mesh);
	ClassDB::bind_method(D_METHOD("import_get_or_generate_mesh", "g4mf_state"), &G4MFMesh4D::import_get_or_generate_mesh);
	ClassDB::bind_static_method("G4MFMesh4D", D_METHOD("export_convert_mesh_into_state", "g4mf_state", "mesh", "deduplicate"), &G4MFMesh4D::export_convert_mesh_into_state, DEFVAL(true));

	ClassDB::bind_static_method("G4MFMesh4D", D_METHOD("from_dictionary", "dict"), &G4MFMesh4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFMesh4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "surfaces", PROPERTY_HINT_ARRAY_TYPE, "G4MFMeshSurface4D"), "set_surfaces", "get_surfaces");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "vertices_accessor_index"), "set_vertices_accessor_index", "get_vertices_accessor_index");
}
