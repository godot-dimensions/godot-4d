#pragma once

#include "../../../model/g4mf/g4mf_document_4d.h"
#include "../../../model/g4mf/g4mf_state_4d.h"
#include "../../../model/mesh/mesh_instance_4d.h"
#include "../../../model/mesh/multi_surface_mesh_4d.h"
#include "../../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../../../model/mesh/tetra/box_tetra_mesh_4d.h"
#include "../../../model/mesh/wire/box_wire_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestG4MFMesh4D {
static Ref<ArrayTetraMesh4D> make_named_box(const String &p_name, const Color &p_color) {
	Ref<BoxTetraMesh4D> box;
	box.instantiate();
	Ref<ArrayTetraMesh4D> mesh = box->to_array_tetra_mesh();
	mesh->set_name(p_name);
	Ref<TetraMaterial4D> material;
	material.instantiate();
	material->set_albedo_color(p_color);
	mesh->set_material(material);
	return mesh;
}

TEST_CASE("[G4MFMesh4D] Multi-surface meshes keep their surface names through export and import") {
	Ref<MultiSurfaceMesh4D> source;
	source.instantiate();
	source->set_name("Multi");
	source->set_surface_meshes({ make_named_box("Alpha", Color(1, 0, 0)), make_named_box("Beta", Color(0, 0, 1)), make_named_box("", Color(0, 1, 0)) });

	Ref<G4MFState4D> state;
	state.instantiate();
	const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, source);
	REQUIRE(mesh_index >= 0);
	const Ref<G4MFMesh4D> g4mf_mesh = state->get_g4mf_meshes()[mesh_index];
	CHECK(g4mf_mesh->get_item_name() == "Multi");
	const TypedArray<G4MFMeshSurface4D> g4mf_surfaces = g4mf_mesh->get_surfaces();
	REQUIRE(g4mf_surfaces.size() == 3);
	CHECK(Ref<G4MFMeshSurface4D>(g4mf_surfaces[0])->get_item_name() == "Alpha");
	CHECK(Ref<G4MFMeshSurface4D>(g4mf_surfaces[1])->get_item_name() == "Beta");
	CHECK(Ref<G4MFMeshSurface4D>(g4mf_surfaces[2])->get_item_name() == "");

	const Ref<MultiSurfaceMesh4D> imported = g4mf_mesh->import_generate_new_mesh(state);
	REQUIRE(imported.is_valid());
	CHECK(imported->get_name() == "Multi");
	const Vector<Ref<SingleSurfaceMesh4D>> &imported_surfaces = imported->get_surface_meshes();
	REQUIRE(imported_surfaces.size() == 3);
	CHECK(imported_surfaces[0]->get_name() == "Alpha");
	CHECK(imported_surfaces[1]->get_name() == "Beta");
	// A surface without a name of its own is named after the mesh, with a suffix to keep names unique.
	CHECK(imported_surfaces[2]->get_name() == "Multi_Surface2");
	// Each surface's own material comes back on that surface.
	const Color expected_colors[3] = { Color(1, 0, 0), Color(0, 0, 1), Color(0, 1, 0) };
	for (int i = 0; i < 3; i++) {
		const Ref<Material4D> material = imported_surfaces[i]->get_material();
		REQUIRE(material.is_valid());
		CHECK(material->get_albedo_color() == expected_colors[i]);
	}
}

TEST_CASE("[G4MFMesh4D] A surface that fails to import is kept as a null entry so the other indices still line up") {
	Ref<MultiSurfaceMesh4D> source;
	source.instantiate();
	source->set_surface_meshes({ make_named_box("First", Color(1, 1, 1)), make_named_box("Second", Color(1, 1, 1)) });
	Ref<G4MFState4D> state;
	state.instantiate();
	const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, source);
	REQUIRE(mesh_index >= 0);
	const Ref<G4MFMesh4D> g4mf_mesh = state->get_g4mf_meshes()[mesh_index];
	// Break the first surface by pointing a simplex at a vertex that does not exist.
	const Ref<G4MFMeshSurface4D> first_surface = g4mf_mesh->get_surfaces()[0];
	first_surface->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, { 0, 1, 2, 99 }, 4, false));
	// An override for the second surface only, keyed by G4MF surface index.
	Ref<TetraMaterial4D> red;
	red.instantiate();
	red->set_albedo_color(Color(1, 0, 0));
	Ref<G4MFMeshInstance4D> g4mf_instance;
	g4mf_instance.instantiate();
	g4mf_instance->set_mesh_index(mesh_index);
	g4mf_instance->set_material_indices({ -1, G4MFMaterial4D::export_convert_material_into_state(state, red) });
	ERR_PRINT_OFF; // The first surface intentionally fails to generate.
	MeshInstance4D *imported = g4mf_instance->import_generate_mesh_instance(state);
	ERR_PRINT_ON;
	REQUIRE(imported != nullptr);
	const Ref<MultiSurfaceMesh4D> imported_mesh = imported->get_mesh();
	REQUIRE(imported_mesh.is_valid());
	REQUIRE(imported_mesh->get_surface_meshes().size() == 2);
	CHECK(imported_mesh->get_surface_meshes()[0].is_null());
	REQUIRE(imported_mesh->get_surface_meshes()[1].is_valid());
	CHECK(imported_mesh->get_surface_meshes()[1]->get_name() == "Second");
	REQUIRE(imported->get_active_material(1).is_valid());
	CHECK(imported->get_active_material(1)->get_albedo_color() == Color(1, 0, 0));
	memdelete(imported);
}

TEST_CASE("[G4MFMesh4D] Single-surface meshes only name the mesh, not the surface") {
	const Ref<ArrayTetraMesh4D> source = make_named_box("Solo", Color(1, 0, 0));
	Ref<G4MFState4D> state;
	state.instantiate();
	const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, source);
	REQUIRE(mesh_index >= 0);
	const Ref<G4MFMesh4D> g4mf_mesh = state->get_g4mf_meshes()[mesh_index];
	CHECK(g4mf_mesh->get_item_name() == "Solo");
	REQUIRE(g4mf_mesh->get_surfaces().size() == 1);
	CHECK(Ref<G4MFMeshSurface4D>(g4mf_mesh->get_surfaces()[0])->get_item_name() == "");

	const Ref<ArrayTetraMesh4D> imported = g4mf_mesh->import_generate_new_mesh(state);
	REQUIRE(imported.is_valid());
	CHECK(imported->get_name() == "Solo");
}

static Ref<G4MFMeshSurfaceBindingGeometry4D> find_geometry_binding(const Ref<G4MFMeshSurfaceBinding4D> &p_binding, const int p_geometry_dimension, const int p_decompose_dimension) {
	if (p_binding.is_null()) {
		return Ref<G4MFMeshSurfaceBindingGeometry4D>();
	}
	const TypedArray<G4MFMeshSurfaceBindingGeometry4D> geometry_bindings = p_binding->get_geometry_bindings();
	for (int i = 0; i < geometry_bindings.size(); i++) {
		const Ref<G4MFMeshSurfaceBindingGeometry4D> geometry_binding = geometry_bindings[i];
		if (geometry_binding->get_geometry_dimension() == p_geometry_dimension && geometry_binding->get_decompose_dimension() == p_decompose_dimension) {
			return geometry_binding;
		}
	}
	return Ref<G4MFMeshSurfaceBindingGeometry4D>();
}

// A box poly mesh translated by p_offset, with a per-vertex (0, 0) normal binding pointing away from the box center.
static Ref<ArrayPolyMesh4D> make_box_with_vertex_normals(const Vector4 &p_offset, const bool p_with_vertex_normals) {
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	Ref<ArrayPolyMesh4D> mesh = box->to_array_poly_mesh();
	PackedVector4Array vertices = mesh->get_poly_cell_vertex_positions();
	PackedVector4Array vertex_normals;
	vertex_normals.resize(vertices.size());
	for (int64_t i = 0; i < vertices.size(); i++) {
		vertex_normals.set(i, vertices[i].normalized());
		vertices.set(i, vertices[i] + p_offset);
	}
	mesh->set_poly_cell_vertex_positions(vertices);
	if (p_with_vertex_normals) {
		mesh->set_poly_cell_dense_normals(Vector2i(0, 0), Vector<PackedVector4Array>{ vertex_normals });
	} else {
		// A key with no data, which must not be written as a binding full of placeholders.
		mesh->set_poly_cell_dense_normals(Vector2i(0, 0), Vector<PackedVector4Array>{ PackedVector4Array() });
	}
	return mesh;
}

static int64_t count_zero_vectors(const PackedVector4Array &p_values) {
	int64_t count = 0;
	for (const Vector4 &value : p_values) {
		if (value == Vector4()) {
			count++;
		}
	}
	return count;
}

TEST_CASE("[G4MFMesh4D] Per-vertex bindings of multi-surface meshes round trip with placeholders") {
	// Surface A spans shared vertices 0-15, surface B spans 16-31, and surface C has an empty (0, 0) key.
	const Ref<ArrayPolyMesh4D> surface_a = make_box_with_vertex_normals(Vector4(), true);
	const Ref<ArrayPolyMesh4D> surface_b = make_box_with_vertex_normals(Vector4(5, 0, 0, 0), true);
	const Ref<ArrayPolyMesh4D> surface_c = make_box_with_vertex_normals(Vector4(0, 5, 0, 0), false);
	REQUIRE(surface_a->is_mesh_data_valid());
	REQUIRE(surface_c->is_mesh_data_valid());
	const int64_t box_vertex_count = surface_a->get_vertex_positions().size();
	const Vector<PackedVector4Array> normals_a = surface_a->get_poly_cell_dense_normals(Vector2i(0, 0));
	const Vector<PackedVector4Array> normals_b = surface_b->get_poly_cell_dense_normals(Vector2i(0, 0));
	REQUIRE(normals_a.size() == 1);
	REQUIRE(normals_a[0].size() == box_vertex_count);
	Ref<MultiSurfaceMesh4D> source;
	source.instantiate();
	source->set_name("Placeholders");
	source->set_surface_meshes({ surface_a, surface_b, surface_c });

	Ref<G4MFState4D> state;
	state.instantiate();
	const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, source);
	REQUIRE(mesh_index >= 0);
	const Ref<G4MFMesh4D> g4mf_mesh = state->get_g4mf_meshes()[mesh_index];
	const TypedArray<G4MFMeshSurface4D> g4mf_surfaces = g4mf_mesh->get_surfaces();
	REQUIRE(g4mf_surfaces.size() == 3);
	REQUIRE(g4mf_mesh->load_vertices(state).size() == box_vertex_count * 3);
	// Surface A was exported first, so its vertices are a prefix of the shared array and it needs no placeholders.
	const Ref<G4MFMeshSurface4D> g4mf_a = g4mf_surfaces[0];
	const Ref<G4MFMeshSurfaceBindingGeometry4D> vertex_binding_a = find_geometry_binding(g4mf_a->get_normals_binding(), 0, 0);
	REQUIRE(vertex_binding_a.is_valid());
	CHECK(vertex_binding_a->load_indices(state).size() == box_vertex_count);
	CHECK(count_zero_vectors(g4mf_a->get_normals_binding()->load_values_as_vector4s(state)) == 0);
	// Surface B's vertices come after A's, so the A range is filled with one shared placeholder value.
	const Ref<G4MFMeshSurface4D> g4mf_b = g4mf_surfaces[1];
	const Ref<G4MFMeshSurfaceBindingGeometry4D> vertex_binding_b = find_geometry_binding(g4mf_b->get_normals_binding(), 0, 0);
	REQUIRE(vertex_binding_b.is_valid());
	const PackedInt32Array indices_b = vertex_binding_b->load_indices(state);
	const PackedVector4Array values_b = g4mf_b->get_normals_binding()->load_values_as_vector4s(state);
	REQUIRE(indices_b.size() == box_vertex_count * 2);
	CHECK(count_zero_vectors(values_b) == 1);
	for (int64_t i = 0; i < box_vertex_count; i++) {
		CHECK(indices_b[i] == indices_b[0]);
		CHECK(values_b[indices_b[i]] == Vector4());
		CHECK(values_b[indices_b[box_vertex_count + i]] == normals_b[0][i]);
	}
	// Surface C's empty key is dropped rather than written as placeholders.
	const Ref<G4MFMeshSurface4D> g4mf_c = g4mf_surfaces[2];
	CHECK(find_geometry_binding(g4mf_c->get_normals_binding(), 0, 0).is_null());

	// Importing accepts A's shorter binding and B's placeholders.
	const Ref<MultiSurfaceMesh4D> imported = g4mf_mesh->import_generate_new_mesh(state);
	REQUIRE(imported.is_valid());
	CHECK(imported->is_mesh_data_valid());
	const Vector<Ref<SingleSurfaceMesh4D>> &imported_surfaces = imported->get_surface_meshes();
	REQUIRE(imported_surfaces.size() == 3);
	const Ref<ArrayPolyMesh4D> imported_a = imported_surfaces[0];
	const Ref<ArrayPolyMesh4D> imported_b = imported_surfaces[1];
	const Ref<ArrayPolyMesh4D> imported_c = imported_surfaces[2];
	REQUIRE(imported_a.is_valid());
	REQUIRE(imported_b.is_valid());
	REQUIRE(imported_c.is_valid());
	CHECK(imported_a->get_poly_cell_dense_normals(Vector2i(0, 0)) == normals_a);
	const Vector<PackedVector4Array> imported_normals_b = imported_b->get_poly_cell_dense_normals(Vector2i(0, 0));
	REQUIRE(imported_normals_b.size() == 1);
	REQUIRE(imported_normals_b[0].size() == box_vertex_count * 2);
	for (int64_t i = 0; i < box_vertex_count; i++) {
		CHECK(imported_normals_b[0][i] == Vector4());
		CHECK(imported_normals_b[0][box_vertex_count + i] == normals_b[0][i]);
	}
	CHECK(imported_c->get_poly_cell_dense_normals(Vector2i(0, 0)).is_empty());
}

TEST_CASE("[G4MFMesh4D] Coincident vertices retain distinct dense bindings through export and import") {
	for (const int binding_mode : { 1, 2, 3 }) {
		CAPTURE(binding_mode);
		Ref<BoxPolyMesh4D> box;
		box.instantiate();
		const Ref<ArrayPolyMesh4D> source = box->to_array_poly_mesh();
		const int64_t box_vertex_count = source->get_poly_cell_vertex_positions().size();
		// Two overlapping boxes in one surface have identical positions but different vertex attributes.
		source->merge_with(box);
		PackedVector4Array normals;
		PackedVector3Array texture_map;
		for (int64_t i = 0; i < box_vertex_count * 2; i++) {
			normals.append(i < box_vertex_count ? Vector4(1, 0, 0, 0) : Vector4(0, 1, 0, 0));
			texture_map.append(i < box_vertex_count ? Vector3(0, 0, 0) : Vector3(1, 1, 1));
		}
		if (binding_mode & 1) {
			source->set_poly_cell_dense_normals(Vector2i(0, 0), { normals });
		}
		if (binding_mode & 2) {
			source->set_poly_cell_dense_texture_map(Vector2i(0, 0), { texture_map });
		}
		REQUIRE(source->is_mesh_data_valid());
		Ref<G4MFState4D> state;
		state.instantiate();
		const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, source);
		REQUIRE(mesh_index >= 0);
		const Ref<G4MFMesh4D> exported = state->get_g4mf_meshes()[mesh_index];
		const Ref<ArrayPolyMesh4D> imported = exported->import_generate_new_mesh(state);
		REQUIRE(imported.is_valid());
		REQUIRE(imported->is_mesh_data_valid());
		const PackedInt32Array source_edges = source->get_edge_indices();
		const PackedInt32Array imported_edges = imported->get_edge_indices();
		REQUIRE(imported_edges.size() == source_edges.size());
		const Vector<PackedVector4Array> imported_normals = imported->get_poly_cell_dense_normals(Vector2i(0, 0));
		const Vector<PackedVector3Array> imported_texture_map = imported->get_poly_cell_dense_texture_map(Vector2i(0, 0));
		if (binding_mode & 1) {
			REQUIRE(imported_normals.size() == 1);
		}
		if (binding_mode & 2) {
			REQUIRE(imported_texture_map.size() == 1);
		}
		// Check the bindings through the geometry references, allowing shared vertex indices to change.
		for (int64_t i = 0; i < source_edges.size(); i++) {
			if (binding_mode & 1) {
				REQUIRE(imported_edges[i] < imported_normals[0].size());
				CHECK(imported_normals[0][imported_edges[i]] == normals[source_edges[i]]);
			}
			if (binding_mode & 2) {
				REQUIRE(imported_edges[i] < imported_texture_map[0].size());
				CHECK(imported_texture_map[0][imported_edges[i]] == texture_map[source_edges[i]]);
			}
		}
	}
}

TEST_CASE("[G4MFMesh4D] Fully shared vertices need no placeholders") {
	// Two identical boxes deduplicate to the same shared vertices, so both bindings are complete.
	const Ref<ArrayPolyMesh4D> surface_a = make_box_with_vertex_normals(Vector4(), true);
	const Ref<ArrayPolyMesh4D> surface_b = make_box_with_vertex_normals(Vector4(), true);
	Ref<MultiSurfaceMesh4D> source;
	source.instantiate();
	source->set_surface_meshes({ surface_a, surface_b });
	Ref<G4MFState4D> state;
	state.instantiate();
	const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, source);
	REQUIRE(mesh_index >= 0);
	const Ref<G4MFMesh4D> g4mf_mesh = state->get_g4mf_meshes()[mesh_index];
	const int64_t box_vertex_count = surface_a->get_vertex_positions().size();
	REQUIRE(g4mf_mesh->load_vertices(state).size() == box_vertex_count);
	for (int surface_index = 0; surface_index < 2; surface_index++) {
		const Ref<G4MFMeshSurface4D> g4mf_surface = g4mf_mesh->get_surfaces()[surface_index];
		const Ref<G4MFMeshSurfaceBindingGeometry4D> vertex_binding = find_geometry_binding(g4mf_surface->get_normals_binding(), 0, 0);
		REQUIRE(vertex_binding.is_valid());
		CHECK(vertex_binding->load_indices(state).size() == box_vertex_count);
		CHECK(count_zero_vectors(g4mf_surface->get_normals_binding()->load_values_as_vector4s(state)) == 0);
	}
}

TEST_CASE("[G4MFMesh4D] Per-cell-vertex bindings survive a round trip with vertices shared across surfaces") {
	// Two adjacent boxes share the 8 vertices of one cubic cell, so surface B's vertices are remapped
	// non-monotonically onto surface A's shared vertex indices when exported.
	Ref<BoxPolyMesh4D> box_a;
	box_a.instantiate();
	Ref<ArrayPolyMesh4D> surface_a = box_a->to_array_poly_mesh();
	Ref<BoxPolyMesh4D> box_b;
	box_b.instantiate();
	Ref<ArrayPolyMesh4D> surface_b = box_b->to_array_poly_mesh();
	surface_b->transform_mesh(Transform4D(Basis4D(), Vector4(1, 0, 0, 0)));
	// Give every cell corner a distinct normal and texture coordinate so any permutation is detectable.
	for (const Ref<ArrayPolyMesh4D> &surface : { surface_a, surface_b }) {
		Vector<PackedVector4Array> normals = surface->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
		Vector<PackedVector3Array> texture = surface->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(normals.size() == 8);
		REQUIRE(texture.size() == 8);
		for (int64_t cell = 0; cell < normals.size(); cell++) {
			PackedVector4Array cell_normals = normals[cell];
			PackedVector3Array cell_texture = texture[cell];
			for (int64_t corner = 0; corner < cell_normals.size(); corner++) {
				cell_normals.set(corner, Vector4(0.125f * cell, 0.0625f * corner, 0.5f, 0.25f).normalized());
				cell_texture.set(corner, Vector3(0.125f * cell, 0.0625f * corner, 0.5f));
			}
			normals.set(cell, cell_normals);
			texture.set(cell, cell_texture);
		}
		surface->set_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY, normals);
		surface->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, texture);
		REQUIRE(surface->is_mesh_data_valid());
	}
	Ref<MultiSurfaceMesh4D> source;
	source.instantiate();
	source->set_surface_meshes({ surface_a, surface_b });

	Ref<G4MFState4D> state;
	state.instantiate();
	const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, source);
	REQUIRE(mesh_index >= 0);
	const Ref<G4MFMesh4D> g4mf_mesh = state->get_g4mf_meshes()[mesh_index];
	const int64_t box_vertex_count = surface_a->get_vertex_positions().size();
	// The 8 vertices of the shared cell are deduplicated.
	CHECK(g4mf_mesh->load_vertices(state).size() == box_vertex_count * 2 - 8);

	const Ref<MultiSurfaceMesh4D> imported = g4mf_mesh->import_generate_new_mesh(state);
	REQUIRE(imported.is_valid());
	REQUIRE(imported->is_mesh_data_valid());
	REQUIRE(imported->get_surface_meshes().size() == 2);
	for (int surface_index = 0; surface_index < 2; surface_index++) {
		CAPTURE(surface_index);
		const Ref<ArrayPolyMesh4D> original = surface_index == 0 ? surface_a : surface_b;
		const Ref<ArrayPolyMesh4D> result = imported->get_surface_meshes()[surface_index];
		REQUIRE(result.is_valid());
		// Compare per cell by matching cell centroids, since the cell order may differ.
		const Vector<PackedInt32Array> original_cells = original->get_all_boundary_cell_vertex_indices(false);
		const Vector<PackedInt32Array> result_cells = result->get_all_boundary_cell_vertex_indices(false);
		REQUIRE(result_cells.size() == original_cells.size());
		const PackedVector4Array original_vertices = original->get_vertex_positions();
		const PackedVector4Array result_vertices = result->get_vertex_positions();
		const Vector<PackedVector4Array> original_normals = original->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
		const Vector<PackedVector4Array> result_normals = result->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
		const Vector<PackedVector3Array> original_texture = original->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		const Vector<PackedVector3Array> result_texture = result->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(result_normals.size() == result_cells.size());
		REQUIRE(result_texture.size() == result_cells.size());
		for (int64_t result_cell = 0; result_cell < result_cells.size(); result_cell++) {
			// Every corner value must be attached to the same vertex position as in the original.
			for (int64_t result_corner = 0; result_corner < result_cells[result_cell].size(); result_corner++) {
				const Vector4 position = result_vertices[result_cells[result_cell][result_corner]];
				bool found = false;
				for (int64_t original_cell = 0; original_cell < original_cells.size() && !found; original_cell++) {
					if (original_normals[original_cell].size() != result_normals[result_cell].size()) {
						continue;
					}
					for (int64_t original_corner = 0; original_corner < original_cells[original_cell].size(); original_corner++) {
						if (original_vertices[original_cells[original_cell][original_corner]].is_equal_approx(position) && original_normals[original_cell][original_corner].is_equal_approx(result_normals[result_cell][result_corner]) && original_texture[original_cell][original_corner].is_equal_approx(result_texture[result_cell][result_corner])) {
							found = true;
							break;
						}
					}
				}
				CHECK_MESSAGE(found, "Cell corner at ", position, " has a normal or texture coordinate that does not match any original corner at that position.");
			}
		}
	}
}

TEST_CASE("[G4MFMesh4D] Exporting skips null surfaces and refuses invalid ones") {
	Ref<G4MFState4D> state;
	state.instantiate();
	SUBCASE("A null entry in a multi-surface mesh is skipped") {
		Ref<MultiSurfaceMesh4D> source;
		source.instantiate();
		source->set_surface_meshes({ make_named_box("Only", Color(1, 0, 0)), Ref<SingleSurfaceMesh4D>() });
		const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, source);
		REQUIRE(mesh_index >= 0);
		const Ref<G4MFMesh4D> g4mf_mesh = state->get_g4mf_meshes()[mesh_index];
		CHECK(g4mf_mesh->get_surfaces().size() == 1);
	}
	SUBCASE("Material overrides skip the null entry too, staying aligned with the exported surfaces") {
		Ref<MultiSurfaceMesh4D> source;
		source.instantiate();
		source->set_surface_meshes({ Ref<SingleSurfaceMesh4D>(), make_named_box("Only", Color(1, 1, 1)) });
		MeshInstance4D *instance = memnew(MeshInstance4D);
		instance->set_mesh(source);
		Ref<TetraMaterial4D> red;
		red.instantiate();
		instance->set_material_overrides({ Ref<Material4D>(), red });
		const Ref<G4MFMeshInstance4D> g4mf_instance = G4MFMeshInstance4D::export_convert_mesh_instance(state, instance);
		REQUIRE(g4mf_instance.is_valid());
		const PackedInt32Array material_indices = g4mf_instance->get_material_indices();
		REQUIRE(material_indices.size() == 1);
		CHECK(material_indices[0] >= 0);
		memdelete(instance);
	}
	SUBCASE("An invalid surface fails the export with an error instead of crashing") {
		Ref<ArrayTetraMesh4D> broken;
		broken.instantiate();
		broken->set_vertex_positions({ Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) });
		broken->set_simplex_cell_vertex_indices({ 0, 1, 2, 99 });
		ERR_PRINT_OFF; // Validating and exporting the deliberately broken mesh prints errors.
		REQUIRE(!broken->is_mesh_data_valid());
		PackedVector4Array shared_vertices;
		const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, broken, shared_vertices);
		CHECK(surface.is_null());
		CHECK(G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, Ref<SingleSurfaceMesh4D>(), shared_vertices).is_null());
		CHECK(G4MFMesh4D::export_convert_mesh_into_state(state, broken) == -1);
		Ref<MultiSurfaceMesh4D> multi;
		multi.instantiate();
		multi->set_surface_meshes({ make_named_box("Fine", Color(1, 0, 0)), broken });
		CHECK(G4MFMesh4D::export_convert_mesh_into_state(state, multi) == -1);
		ERR_PRINT_ON;
	}
}

TEST_CASE("[G4MFMeshInstance4D] Material overrides round trip per surface") {
	Ref<MultiSurfaceMesh4D> mesh;
	mesh.instantiate();
	mesh->set_surface_meshes({ make_named_box("A", Color(1, 1, 1)), make_named_box("B", Color(1, 1, 1)), make_named_box("C", Color(1, 1, 1)) });
	MeshInstance4D *instance = memnew(MeshInstance4D);
	instance->set_mesh(mesh);
	Ref<TetraMaterial4D> red;
	red.instantiate();
	red->set_albedo_color(Color(1, 0, 0));
	Ref<TetraMaterial4D> blue;
	blue.instantiate();
	blue->set_albedo_color(Color(0, 0, 1));

	SUBCASE("One override per surface, with a gap") {
		instance->set_material_overrides({ Ref<Material4D>(red), Ref<Material4D>(), Ref<Material4D>(blue) });
		Ref<G4MFState4D> state;
		state.instantiate();
		const Ref<G4MFMeshInstance4D> g4mf_instance = G4MFMeshInstance4D::export_convert_mesh_instance(state, instance);
		REQUIRE(g4mf_instance.is_valid());
		const PackedInt32Array material_indices = g4mf_instance->get_material_indices();
		REQUIRE(material_indices.size() == 3);
		CHECK(material_indices[0] >= 0);
		CHECK(material_indices[1] == -1);
		CHECK(material_indices[2] >= 0);
		CHECK(material_indices[0] != material_indices[2]);
		MeshInstance4D *imported = g4mf_instance->import_generate_mesh_instance(state);
		REQUIRE(imported != nullptr);
		const Vector<Ref<Material4D>> imported_overrides = imported->get_material_overrides();
		REQUIRE(imported_overrides.size() == 3);
		REQUIRE(imported_overrides[0].is_valid());
		CHECK(imported_overrides[0]->get_albedo_color() == Color(1, 0, 0));
		CHECK(imported_overrides[1].is_null());
		REQUIRE(imported_overrides[2].is_valid());
		CHECK(imported_overrides[2]->get_albedo_color() == Color(0, 0, 1));
		memdelete(imported);
	}

	SUBCASE("A single override applies to every surface") {
		instance->set_material_override(red);
		Ref<G4MFState4D> state;
		state.instantiate();
		const Ref<G4MFMeshInstance4D> g4mf_instance = G4MFMeshInstance4D::export_convert_mesh_instance(state, instance);
		REQUIRE(g4mf_instance.is_valid());
		REQUIRE(g4mf_instance->get_material_indices().size() == 1);
		MeshInstance4D *imported = g4mf_instance->import_generate_mesh_instance(state);
		REQUIRE(imported != nullptr);
		const Vector<Ref<Material4D>> imported_overrides = imported->get_material_overrides();
		REQUIRE(imported_overrides.size() == 1);
		REQUIRE(imported_overrides[0].is_valid());
		CHECK(imported_overrides[0]->get_albedo_color() == Color(1, 0, 0));
		for (int surface = 0; surface < 3; surface++) {
			CHECK(imported->get_active_material(surface) == imported_overrides[0]);
		}
		memdelete(imported);
	}
	memdelete(instance);
}

TEST_CASE("[G4MFMeshInstance4D] A single override uses each generated surface's material class") {
	for (const bool wire_first : { false, true }) {
		CAPTURE(wire_first);
		for (const bool force_wireframe : { false, true }) {
			CAPTURE(force_wireframe);
			Ref<BoxWireMesh4D> wire;
			wire.instantiate();
			Ref<BoxPolyMesh4D> poly;
			poly.instantiate();
			const Ref<ArrayTetraMesh4D> tetra = make_named_box("Tetra", Color(1, 1, 1));
			Ref<MultiSurfaceMesh4D> source;
			source.instantiate();
			source->set_surface_meshes(wire_first ? Vector<Ref<SingleSurfaceMesh4D>>{ wire, tetra, poly } : Vector<Ref<SingleSurfaceMesh4D>>{ tetra, wire, poly });
			Ref<G4MFState4D> state;
			state.instantiate();
			const int mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, source);
			REQUIRE(mesh_index >= 0);
			if (force_wireframe) {
				state->set_preferred_mesh_surface_format(G4MFMeshSurface4D::MESH_SURFACE_FORMAT_WIREFRAME);
			}
			Ref<TetraMaterial4D> red;
			red.instantiate();
			red->set_albedo_color(Color(1, 0, 0));
			Ref<G4MFMeshInstance4D> instance;
			instance.instantiate();
			instance->set_mesh_index(mesh_index);
			instance->set_material_indices({ G4MFMaterial4D::export_convert_material_into_state(state, red) });
			MeshInstance4D *imported = instance->import_generate_mesh_instance(state);
			REQUIRE(imported != nullptr);
			const Ref<MultiSurfaceMesh4D> imported_mesh = imported->get_mesh();
			REQUIRE(imported_mesh.is_valid());
			REQUIRE(imported_mesh->get_surface_meshes().size() == 3);
			CHECK(imported->get_material_overrides().size() == (force_wireframe ? 1 : 3));
			for (int i = 0; i < 3; i++) {
				const Ref<Material4D> material = imported->get_active_material(i);
				REQUIRE(material.is_valid());
				CHECK(material->get_albedo_color() == Color(1, 0, 0));
				const String expected_class = force_wireframe || i == (wire_first ? 0 : 1) ? "WireMaterial4D" : (i == 2 ? "PolyMaterial4D" : "TetraMaterial4D");
				CHECK(material->get_class() == expected_class);
			}
			memdelete(imported);
		}
	}
}

TEST_CASE("[G4MFDocument4D] Combined mesh import skips meshes that fail to generate") {
	Ref<G4MFState4D> state;
	state.instantiate();
	const int good_mesh_index = G4MFMesh4D::export_convert_mesh_into_state(state, make_named_box("Good", Color(1, 0, 0)));
	REQUIRE(good_mesh_index >= 0);
	// A mesh whose only surface references a vertex that does not exist cannot be generated.
	const Ref<G4MFMesh4D> good_mesh = state->get_g4mf_meshes()[good_mesh_index];
	Ref<G4MFMeshSurface4D> broken_surface;
	broken_surface.instantiate();
	broken_surface->set_simplexes_accessor_index(G4MFAccessor4D::encode_new_accessor_from_int32s(state, { 0, 1, 2, 99 }, 4, false));
	Ref<G4MFMesh4D> broken_mesh;
	broken_mesh.instantiate();
	broken_mesh->set_vertices_accessor_index(good_mesh->get_vertices_accessor_index());
	TypedArray<G4MFMeshSurface4D> broken_surfaces;
	broken_surfaces.append(broken_surface);
	broken_mesh->set_surfaces(broken_surfaces);
	TypedArray<G4MFMesh4D> meshes = state->get_g4mf_meshes();
	meshes.append(broken_mesh);
	state->set_g4mf_meshes(meshes);
	const int broken_mesh_index = meshes.size() - 1;
	for (const int mesh_index : { good_mesh_index, broken_mesh_index, good_mesh_index }) {
		Ref<G4MFMeshInstance4D> g4mf_instance;
		g4mf_instance.instantiate();
		g4mf_instance->set_mesh_index(mesh_index);
		Ref<G4MFNode4D> node;
		node.instantiate();
		node->set_mesh_instance(g4mf_instance);
		state->append_g4mf_node(node);
	}
	Ref<G4MFDocument4D> document;
	document.instantiate();
	ERR_PRINT_OFF; // The broken mesh intentionally fails to generate.
	const Ref<Mesh4D> combined = document->import_generate_godot_mesh(state, -1, true);
	ERR_PRINT_ON;
	REQUIRE(combined.is_valid());
	CHECK(combined->is_mesh_data_valid());
	// The two good instances merged into one surface, and the broken one was skipped rather than aborting the import.
	const Ref<ArrayTetraMesh4D> combined_tetra = combined;
	REQUIRE(combined_tetra.is_valid());
	const int64_t box_tet_count = make_named_box("Box", Color(1, 1, 1))->get_simplex_cell_vertex_indices().size() / 4;
	CHECK(combined_tetra->get_simplex_cell_vertex_indices().size() / 4 == box_tet_count * 2);
}
} // namespace TestG4MFMesh4D
