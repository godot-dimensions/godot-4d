#pragma once

#include "../../../model/mesh/multi_surface_mesh_4d.h"
#include "../../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../../../model/mesh/poly/poly_material_4d.h"
#include "../../../model/mesh/tetra/box_tetra_mesh_4d.h"
#include "../../../model/mesh/wire/array_wire_mesh_4d.h"
#include "../../../model/mesh/wire/box_wire_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestMultiSurfaceMesh4D {
static const char *VALIDATION_RESET = "mesh_data_validation_reset";
static const char *PROXY_DIRTY = "proxy_mesh_3d_marked_dirty";

// Both signals have no arguments, so each emission is recorded as an empty argument list.
static Array _emissions(const int p_count) {
	Array emissions;
	for (int i = 0; i < p_count; i++) {
		emissions.push_back(Array());
	}
	return emissions;
}

TEST_CASE("[MultiSurfaceMesh4D] Transforming keeps every surface and makes them writable") {
	// A mix of non-array and array surfaces, since the non-array ones must be converted before they can be transformed.
	Ref<BoxTetraMesh4D> box_tetra;
	box_tetra.instantiate();
	box_tetra->set_name("Tetra");
	Ref<BoxPolyMesh4D> box_poly;
	box_poly.instantiate();
	box_poly->set_size(Vector4(2, 2, 2, 2));
	Ref<ArrayPolyMesh4D> array_poly = box_poly->to_array_poly_mesh();
	array_poly->set_name("Poly");
	const PackedVector4Array tetra_vertices = box_tetra->get_vertex_positions();
	const PackedVector4Array poly_vertices = array_poly->get_vertex_positions();

	Ref<MultiSurfaceMesh4D> mesh;
	mesh.instantiate();
	mesh->set_surface_meshes({ box_tetra, array_poly });
	REQUIRE(mesh->is_mesh_data_valid());
	const Rect4 bounds_before = mesh->get_rect_bounds();

	// The same Z-up to Y-up conversion the editor import plugins apply.
	const Transform4D z_up_to_y_up = Transform4D(Basis4D(Vector4(1, 0, 0, 0), Vector4(0, 0, -1, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 0, 1)), Vector4(1, 2, 3, 4));
	mesh->transform_mesh(z_up_to_y_up);
	CHECK(mesh->is_mesh_data_valid());
	// The bounds of the whole mesh must follow the transformed surfaces. The larger poly box (size 2) determines the extent.
	CHECK_MESSAGE(mesh->get_rect_bounds().get_end().is_equal_approx(bounds_before.get_end() + Vector4(1, 2, 3, 4)), "Transforming the surfaces should mark the multi-surface mesh's bounds dirty.");
	const Vector<Ref<SingleSurfaceMesh4D>> &surfaces = mesh->get_surface_meshes();
	REQUIRE(surfaces.size() == 2);
	// The box tetra mesh was converted to an array mesh so it could be transformed, keeping its name.
	const Ref<ArrayTetraMesh4D> transformed_tetra = surfaces[0];
	REQUIRE(transformed_tetra.is_valid());
	CHECK(transformed_tetra->get_name() == "Tetra");
	const PackedVector4Array transformed_tetra_vertices = transformed_tetra->get_vertex_positions();
	REQUIRE(transformed_tetra_vertices.size() == tetra_vertices.size());
	for (int64_t i = 0; i < tetra_vertices.size(); i++) {
		CHECK(transformed_tetra_vertices[i].is_equal_approx(z_up_to_y_up.xform(tetra_vertices[i])));
	}
	// The array poly mesh is transformed in place.
	const Ref<ArrayPolyMesh4D> transformed_poly = surfaces[1];
	REQUIRE(transformed_poly.is_valid());
	CHECK(transformed_poly == array_poly);
	CHECK(transformed_poly->get_name() == "Poly");
	const PackedVector4Array transformed_poly_vertices = transformed_poly->get_vertex_positions();
	REQUIRE(transformed_poly_vertices.size() == poly_vertices.size());
	for (int64_t i = 0; i < poly_vertices.size(); i++) {
		CHECK(transformed_poly_vertices[i].is_equal_approx(z_up_to_y_up.xform(poly_vertices[i])));
	}
}

TEST_CASE("[MultiSurfaceMesh4D] Surface changes propagate to the multi-surface mesh") {
	// A plain array mesh with no texture map, so its simplex indices can be replaced freely below.
	Ref<ArrayTetraMesh4D> array_tetra;
	array_tetra.instantiate();
	array_tetra->set_vertex_positions({ Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0) });
	array_tetra->set_simplex_cell_vertex_indices({ 0, 1, 2, 3 });
	Ref<BoxPolyMesh4D> box_poly;
	box_poly.instantiate();
	Ref<MultiSurfaceMesh4D> mesh;
	mesh.instantiate();
	mesh->set_surface_meshes({ array_tetra, box_poly });
	REQUIRE(mesh->is_mesh_data_valid());
	const Rect4 bounds_before = mesh->get_rect_bounds();

	SUBCASE("Transforming a surface directly only dirties the bounds and proxy mesh") {
		SIGNAL_WATCH(mesh.ptr(), VALIDATION_RESET);
		SIGNAL_WATCH(mesh.ptr(), PROXY_DIRTY);
		array_tetra->transform_mesh(Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		SIGNAL_UNWATCH(mesh.ptr(), VALIDATION_RESET);
		SIGNAL_UNWATCH(mesh.ptr(), PROXY_DIRTY);
		CHECK(mesh->is_mesh_data_valid());
		CHECK_MESSAGE(mesh->get_rect_bounds().get_end().x == doctest::Approx(bounds_before.get_end().x + 10.0), "The multi-surface mesh's bounds must follow a surface transformed behind its back.");
	}

	SUBCASE("Resizing a primitive surface only dirties the bounds and proxy mesh") {
		SIGNAL_WATCH(mesh.ptr(), VALIDATION_RESET);
		SIGNAL_WATCH(mesh.ptr(), PROXY_DIRTY);
		box_poly->set_size(Vector4(4, 4, 4, 4));
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(1));
		SIGNAL_UNWATCH(mesh.ptr(), VALIDATION_RESET);
		SIGNAL_UNWATCH(mesh.ptr(), PROXY_DIRTY);
		CHECK(mesh->get_rect_bounds().get_end().is_equal_approx(Vector4(2, 2, 2, 2)));
	}

	SUBCASE("Changing a surface's data resets the validation of the whole mesh") {
		SIGNAL_WATCH(mesh.ptr(), VALIDATION_RESET);
		SIGNAL_WATCH(mesh.ptr(), PROXY_DIRTY);
		array_tetra->set_simplex_cell_vertex_indices({ 0, 1, 2, 3 });
		SIGNAL_CHECK(VALIDATION_RESET, _emissions(1));
		// Once from the validation reset, and once more from the surface's own proxy dirty signal that follows it.
		SIGNAL_CHECK(PROXY_DIRTY, _emissions(2));
		SIGNAL_UNWATCH(mesh.ptr(), VALIDATION_RESET);
		SIGNAL_UNWATCH(mesh.ptr(), PROXY_DIRTY);
		CHECK(mesh->is_mesh_data_valid());
		array_tetra->set_simplex_cell_vertex_indices({ 0, 1, 2, 99 });
		ERR_PRINT_OFF; // The surface is intentionally invalid now.
		CHECK_FALSE(mesh->is_mesh_data_valid());
		ERR_PRINT_ON;
	}

	SUBCASE("Replaced surfaces are disconnected") {
		mesh->set_surface_meshes({ box_poly });
		SIGNAL_WATCH(mesh.ptr(), VALIDATION_RESET);
		SIGNAL_WATCH(mesh.ptr(), PROXY_DIRTY);
		array_tetra->transform_mesh(Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		array_tetra->set_simplex_cell_vertex_indices({ 0, 1, 2, 3 });
		SIGNAL_CHECK_FALSE(VALIDATION_RESET);
		SIGNAL_CHECK_FALSE(PROXY_DIRTY);
		SIGNAL_UNWATCH(mesh.ptr(), VALIDATION_RESET);
		SIGNAL_UNWATCH(mesh.ptr(), PROXY_DIRTY);
	}
}

TEST_CASE("[MultiSurfaceMesh4D] Merging compatible surfaces combines a shared per-cell material without modifying it") {
	// Four boxes with the same name. Three share one per-cell material, and the fourth has an equal but separate copy.
	Ref<PolyMaterial4D> shared_material;
	shared_material.instantiate();
	shared_material->set_albedo_source(TetraMaterial4D::TETRA_COLOR_SOURCE_PER_CELL_ONLY);
	PackedColorArray cell_colors;
	for (int cell = 0; cell < 8; cell++) {
		cell_colors.append(Color(cell / 8.0f, 0.5f, 0.25f));
	}
	shared_material->set_poly_albedo_color_array(cell_colors);
	const Ref<Material4D> separate_material = shared_material->duplicate();
	Vector<Ref<SingleSurfaceMesh4D>> surfaces;
	for (int i = 0; i < 4; i++) {
		Ref<BoxPolyMesh4D> box;
		box.instantiate();
		Ref<ArrayPolyMesh4D> array_box = box->to_array_poly_mesh();
		array_box->set_name("Same");
		array_box->set_material(i < 3 ? Ref<Material4D>(shared_material) : separate_material);
		surfaces.append(array_box);
	}
	Ref<MultiSurfaceMesh4D> mesh;
	mesh.instantiate();
	mesh->set_surface_meshes(surfaces);
	mesh->merge_compatible_surfaces();
	CHECK(mesh->is_mesh_data_valid());
	const Vector<Ref<SingleSurfaceMesh4D>> &merged = mesh->get_surface_meshes();
	REQUIRE_MESSAGE(merged.size() == 2, "The three surfaces sharing a material must merge into one, and the separate material must stay apart.");
	CHECK(merged[0] == surfaces[0]);
	CHECK(merged[1] == surfaces[3]);
	// The merged surface has a duplicated material with one color per cell of all three boxes, and the shared material is untouched.
	const Ref<PolyMaterial4D> merged_material = merged[0]->get_material();
	REQUIRE(merged_material.is_valid());
	CHECK(merged_material != shared_material);
	const PackedColorArray merged_colors = merged_material->get_poly_albedo_color_array();
	REQUIRE(merged_colors.size() == 8 * 3);
	for (int i = 0; i < merged_colors.size(); i++) {
		CHECK(merged_colors[i] == cell_colors[i % 8]);
	}
	CHECK(shared_material->get_poly_albedo_color_array() == cell_colors);
	CHECK(merged[1]->get_material() == separate_material);
}

TEST_CASE("[MultiSurfaceMesh4D] Self merge appends transformed copies of the original surfaces once") {
	Ref<BoxPolyMesh4D> box_poly;
	box_poly.instantiate();
	const Ref<ArrayPolyMesh4D> array_poly = box_poly->to_array_poly_mesh();
	Ref<BoxTetraMesh4D> box_tetra;
	box_tetra.instantiate();
	const Vector<Ref<SingleSurfaceMesh4D>> originals = { array_poly, box_tetra };
	const Vector<PackedVector4Array> original_vertices = { array_poly->get_vertex_positions(), box_tetra->get_vertex_positions() };
	Ref<MultiSurfaceMesh4D> mesh;
	mesh.instantiate();
	mesh->set_surface_meshes({ array_poly, Ref<SingleSurfaceMesh4D>(), box_tetra });
	const Transform4D transform = Transform4D(Basis4D(), Vector4(3, 4, 5, 6));
	mesh->merge_with(mesh, transform);
	CHECK(mesh->is_mesh_data_valid());
	const Vector<Ref<SingleSurfaceMesh4D>> &surfaces = mesh->get_surface_meshes();
	REQUIRE(surfaces.size() == 5);
	CHECK(surfaces[0] == array_poly);
	CHECK(surfaces[1].is_null());
	CHECK(surfaces[2] == box_tetra);
	for (int i = 0; i < originals.size(); i++) {
		const Ref<SingleSurfaceMesh4D> copy = surfaces[3 + i];
		REQUIRE(copy.is_valid());
		CHECK(copy != originals[i]);
		CHECK(originals[i]->get_vertex_positions() == original_vertices[i]);
		const PackedVector4Array copied_vertices = copy->get_vertex_positions();
		REQUIRE(copied_vertices.size() == original_vertices[i].size());
		for (int64_t vertex_index = 0; vertex_index < copied_vertices.size(); vertex_index++) {
			CHECK(copied_vertices[vertex_index].is_equal_approx(transform.xform(original_vertices[i][vertex_index])));
		}
	}
}

TEST_CASE("[MultiSurfaceMesh4D] Compatible merges preserve invalid sources and destinations") {
	Ref<SingleSurfaceMesh4D> valid;
	Ref<SingleSurfaceMesh4D> invalid;
	SUBCASE("Poly surfaces") {
		Ref<BoxPolyMesh4D> box;
		box.instantiate();
		valid = box->to_array_poly_mesh();
		const Ref<ArrayPolyMesh4D> invalid_poly = box->to_array_poly_mesh();
		invalid_poly->set_poly_cell_vertex_positions(PackedVector4Array());
		invalid = invalid_poly;
	}
	SUBCASE("Tetra surfaces") {
		Ref<BoxTetraMesh4D> box;
		box.instantiate();
		valid = box->to_array_tetra_mesh();
		const Ref<ArrayTetraMesh4D> invalid_tetra = box->to_array_tetra_mesh();
		invalid_tetra->set_vertex_positions(PackedVector4Array());
		invalid = invalid_tetra;
	}
	SUBCASE("Wire surfaces") {
		Ref<BoxWireMesh4D> box;
		box.instantiate();
		valid = box->to_array_wire_mesh();
		const Ref<ArrayWireMesh4D> invalid_wire = box->to_array_wire_mesh();
		invalid_wire->set_vertex_positions(PackedVector4Array());
		invalid = invalid_wire;
	}
	REQUIRE(valid.is_valid());
	REQUIRE(invalid.is_valid());
	REQUIRE(valid->is_mesh_data_valid());
	const PackedVector4Array valid_vertices = valid->get_vertex_positions();
	for (const bool invalid_first : { false, true }) {
		CAPTURE(invalid_first);
		const Vector<Ref<SingleSurfaceMesh4D>> originals = invalid_first ? Vector<Ref<SingleSurfaceMesh4D>>{ invalid, valid } : Vector<Ref<SingleSurfaceMesh4D>>{ valid, invalid };
		Ref<MultiSurfaceMesh4D> mesh;
		mesh.instantiate();
		mesh->set_surface_meshes(originals);
		ERR_PRINT_OFF;
		mesh->merge_compatible_surfaces();
		ERR_PRINT_ON;
		CHECK(mesh->get_surface_meshes() == originals);
		CHECK(valid->is_mesh_data_valid());
		CHECK(valid->get_vertex_positions() == valid_vertices);
	}
}

TEST_CASE("[MultiSurfaceMesh4D] Transforming an empty mesh does nothing") {
	Ref<MultiSurfaceMesh4D> mesh;
	mesh.instantiate();
	mesh->transform_mesh(Transform4D(Basis4D(), Vector4(1, 0, 0, 0)));
	CHECK(mesh->get_surface_meshes().is_empty());
	CHECK(mesh->is_mesh_data_valid());
}
} // namespace TestMultiSurfaceMesh4D
