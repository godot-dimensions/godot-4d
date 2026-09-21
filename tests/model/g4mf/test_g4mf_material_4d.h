#pragma once

#include "../../../model/g4mf/g4mf_state_4d.h"
#include "../../../model/g4mf/structures/g4mf_material_4d.h"
#include "../../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../../../model/mesh/poly/poly_material_4d.h"

#include "tests/test_macros.h"

namespace TestG4MFMaterial4D {
// A box has 8 boundary cells (cubes).
static Ref<ArrayPolyMesh4D> make_box_poly_mesh() {
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	return box->to_array_poly_mesh();
}

// Colors that are exactly representable as 32-bit floats, so they survive encoding into a G4MF accessor.
static PackedColorArray make_distinct_colors(const int64_t p_count) {
	PackedColorArray colors;
	for (int64_t i = 0; i < p_count; i++) {
		colors.append(Color(0.125f * (i % 8), 1.0f - 0.0625f * (i % 16), 0.5f * (i % 2), 1.0f));
	}
	return colors;
}

static Ref<G4MFMaterial4D> export_single_material(const Ref<G4MFState4D> &p_state, const Ref<Material4D> &p_material) {
	const int material_index = G4MFMaterial4D::export_convert_material_into_state(p_state, p_material);
	if (material_index < 0 || material_index >= p_state->get_g4mf_materials().size()) {
		return Ref<G4MFMaterial4D>();
	}
	return p_state->get_g4mf_materials()[material_index];
}

TEST_CASE("[G4MFMaterial4D] Per-cell poly material exports without populating its per-tetrahedron cache") {
	const PackedColorArray cell_colors = make_distinct_colors(8);
	Ref<PolyMaterial4D> source;
	source.instantiate();
	source->set_name("PerCellPoly");
	source->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
	source->set_poly_albedo_color_array(cell_colors);
	REQUIRE(source->get_albedo_color_array().is_empty()); // Never rendered, so the per-tetrahedron cache is empty.

	Ref<G4MFState4D> state;
	state.instantiate();
	ERR_PRINT_OFF; // Exporting an unrendered PolyMaterial4D intentionally warns that only per-cell colors are written.
	const Ref<G4MFMaterial4D> g4mf_material = export_single_material(state, source);
	ERR_PRINT_ON;
	REQUIRE(g4mf_material.is_valid());
	CHECK(g4mf_material->get_name() == "PerCellPoly");
	const Ref<G4MFMaterialChannel4D> base_color = g4mf_material->get_base_color_channel();
	REQUIRE(base_color.is_valid());
	const Ref<G4MFMeshSurfaceBinding4D> binding = base_color->get_element_map_binding();
	REQUIRE(binding.is_valid());
	// Only the per-polytope-cell binding is written, there is no per-simplex data to write.
	CHECK(binding->get_per_simplex_accessor_index() == -1);
	const PackedInt32Array per_cell_indices = binding->load_geometry_binding_indices(state, 3, 3);
	REQUIRE(per_cell_indices.size() == cell_colors.size());
	const PackedColorArray values = binding->load_values_as_colors(state);
	for (int64_t i = 0; i < per_cell_indices.size(); i++) {
		REQUIRE(per_cell_indices[i] >= 0);
		REQUIRE(per_cell_indices[i] < values.size());
		CHECK(values[per_cell_indices[i]] == cell_colors[i]);
	}

	SUBCASE("Importing as a poly material restores the per-cell colors and is cached") {
		const Ref<PolyMaterial4D> imported = g4mf_material->import_get_or_generate_poly_material(state);
		REQUIRE(imported.is_valid());
		CHECK(imported->get_name() == "PerCellPoly");
		CHECK(imported->get_poly_albedo_color_array() == cell_colors);
		CHECK(imported->get_albedo_color_array().is_empty());
		CHECK((imported->get_albedo_source_flags() & Material4D::COLOR_SOURCE_FLAG_PER_CELL) != 0);
		CHECK((imported->get_albedo_source_flags() & Material4D::COLOR_SOURCE_FLAG_USES_COLOR_ARRAY) != 0);
		CHECK(!(imported->get_albedo_source_flags() & Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR));
		// The cache hands back the same object, while generating a new one does not.
		CHECK(g4mf_material->import_get_or_generate_poly_material(state) == imported);
		const Ref<PolyMaterial4D> fresh = g4mf_material->import_generate_new_poly_material(state);
		REQUIRE(fresh.is_valid());
		CHECK(fresh != imported);
		CHECK(fresh->get_poly_albedo_color_array() == cell_colors);
		// Each material class has its own cache slot.
		const Ref<TetraMaterial4D> tetra = g4mf_material->import_get_or_generate_tetra_material(state);
		REQUIRE(tetra.is_valid());
		CHECK(tetra != imported);
		CHECK(g4mf_material->import_get_or_generate_tetra_material(state) == tetra);
	}
}

TEST_CASE("[G4MFMaterial4D] Poly material import falls back to per-simplex colors") {
	// A plain TetraMaterial4D on a poly mesh exports only per-simplex colors.
	const Ref<ArrayPolyMesh4D> mesh = make_box_poly_mesh();
	const int64_t simplex_count = mesh->get_simplex_cell_vertex_indices().size() / 4;
	REQUIRE(simplex_count > 8);
	const PackedColorArray simplex_colors = make_distinct_colors(simplex_count);
	Ref<TetraMaterial4D> source;
	source.instantiate();
	source->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
	source->set_albedo_color_array(simplex_colors);

	Ref<G4MFState4D> state;
	state.instantiate();
	const Ref<G4MFMaterial4D> g4mf_material = export_single_material(state, source);
	REQUIRE(g4mf_material.is_valid());
	const Ref<G4MFMeshSurfaceBinding4D> binding = g4mf_material->get_base_color_channel()->get_element_map_binding();
	REQUIRE(binding.is_valid());
	CHECK(binding->get_per_simplex_accessor_index() >= 0);
	CHECK(binding->load_geometry_binding_indices(state, 3, 3).is_empty());

	const Ref<PolyMaterial4D> poly = g4mf_material->import_get_or_generate_poly_material(state);
	REQUIRE(poly.is_valid());
	CHECK(poly->get_poly_albedo_color_array().is_empty());
	CHECK(poly->get_albedo_color_array() == simplex_colors);
	CHECK((poly->get_albedo_source_flags() & Material4D::COLOR_SOURCE_FLAG_PER_CELL) != 0);
	// Populating from the poly array is a no-op when it is empty, so the per-simplex colors are kept for rendering.
	poly->populate_albedo_color_array_for_poly_mesh(mesh);
	CHECK(poly->get_albedo_color_array() == simplex_colors);
	const Ref<TetraMaterial4D> tetra = g4mf_material->import_get_or_generate_tetra_material(state);
	REQUIRE(tetra.is_valid());
	CHECK(tetra->get_albedo_color_array() == simplex_colors);
}

TEST_CASE("[G4MFMaterial4D] Poly mesh surface round trip keeps a PolyMaterial4D with per-cell colors") {
	const PackedColorArray cell_colors = make_distinct_colors(8);
	Ref<ArrayPolyMesh4D> source = make_box_poly_mesh();
	REQUIRE(source->get_poly_cell_indices().size() > 1);
	REQUIRE(source->get_poly_cell_indices()[1].size() == cell_colors.size());
	Ref<PolyMaterial4D> source_material;
	source_material.instantiate();
	source_material->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
	source_material->set_poly_albedo_color_array(cell_colors);
	source->set_material(source_material);

	Ref<G4MFState4D> state;
	state.instantiate();
	ERR_PRINT_OFF; // Exporting an unrendered PolyMaterial4D intentionally warns that only per-cell colors are written.
	const Ref<G4MFMeshSurface4D> surface = G4MFMeshSurface4D::export_convert_mesh_surface_for_state(state, source);
	ERR_PRINT_ON;
	REQUIRE(surface.is_valid());
	REQUIRE(surface->get_material_index() >= 0);

	const Ref<ArrayPolyMesh4D> imported = surface->import_generate_poly_mesh_surface(state, source->get_vertex_positions());
	REQUIRE(imported.is_valid());
	const Ref<PolyMaterial4D> imported_material = imported->get_material();
	REQUIRE(imported_material.is_valid());
	CHECK(imported_material->get_poly_albedo_color_array() == cell_colors);
	// Two surfaces referencing the same G4MF material share one imported material object.
	const Ref<ArrayPolyMesh4D> imported_again = surface->import_generate_poly_mesh_surface(state, source->get_vertex_positions());
	REQUIRE(imported_again.is_valid());
	CHECK(imported_again->get_material() == imported->get_material());
	// The same material imports as a TetraMaterial4D for tetrahedral meshes, from a separate cache slot.
	const Ref<ArrayTetraMesh4D> imported_tetra = surface->import_generate_tetra_mesh_surface(state, source->get_vertex_positions());
	REQUIRE(imported_tetra.is_valid());
	const Ref<Material4D> imported_tetra_material = imported_tetra->get_material();
	REQUIRE(imported_tetra_material.is_valid());
	CHECK(imported_tetra_material != imported->get_material());
	CHECK(Ref<PolyMaterial4D>(imported_tetra_material).is_null());
}
} // namespace TestG4MFMaterial4D
