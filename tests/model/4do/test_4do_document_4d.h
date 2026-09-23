#pragma once

#include "../../../model/4do/4do_document_4d.h"
#include "../../../model/mesh/multi_surface_mesh_4d.h"
#include "../../../model/mesh/poly/poly_material_4d.h"

#include "tests/test_macros.h"

namespace TestFourDODocument4D {
static Ref<FourDODocument4D> read_4do_text(const String &p_text) {
	return FourDODocument4D::import_read_from_byte_array(p_text.to_utf8_buffer(), "", "test.4do");
}

static Ref<SingleSurfaceMesh4D> find_surface(const Ref<MultiSurfaceMesh4D> &p_mesh, const String &p_name) {
	for (const Ref<SingleSurfaceMesh4D> &surface : p_mesh->get_surface_meshes()) {
		if (surface->get_name() == p_name) {
			return surface;
		}
	}
	return Ref<SingleSurfaceMesh4D>();
}

// Two unit cubes at x=2 and x=3, in Z-order, taken from a HoxelDraw export.
static const char *TWO_CUBE_VERTICES =
		"v 2 1 0 0\nv 2 0 0 0\nv 2 1 0 1\nv 2 0 0 1\nv 2 1 1 0\nv 2 0 1 0\nv 2 1 1 1\nv 2 0 1 1\n"
		"v 3 0 0 0\nv 3 1 0 0\nv 3 0 0 1\nv 3 1 0 1\nv 3 0 1 0\nv 3 1 1 0\nv 3 0 1 1\nv 3 1 1 1\n";

TEST_CASE("[FourDODocument4D] Version 1 style cell colors become a per-cell PolyMaterial4D") {
	const String text = String("4DO 2\n"
							   "co 255 0 0\n"
							   "co 0 0 255\n"
							   "cellformat co v\n"
							   "usemtl red_blue\n") +
			TWO_CUBE_VERTICES +
			"c 0 7 6 5 4 3 2 1 0\n"
			"c 1 15 14 13 12 11 10 9 8\n";
	const Ref<FourDODocument4D> doc = read_4do_text(text);
	REQUIRE(doc.is_valid());
	const Ref<MultiSurfaceMesh4D> mesh = doc->import_generate_multi_surface_mesh_4d();
	REQUIRE(mesh.is_valid());
	CHECK(mesh->is_mesh_data_valid());
	REQUIRE(mesh->get_surface_meshes().size() == 1);
	// Surfaces are named after their 4DO material.
	const Ref<ArrayPolyMesh4D> poly_mesh = find_surface(mesh, "red_blue");
	REQUIRE(poly_mesh.is_valid());
	CHECK(poly_mesh->get_poly_cell_indices().size() == 2);
	CHECK(poly_mesh->get_poly_cell_indices()[1].size() == 2);
	const Ref<PolyMaterial4D> material = poly_mesh->get_material();
	REQUIRE(material.is_valid());
	const PackedColorArray expected = { Color(1, 0, 0, 1), Color(0, 0, 1, 1) };
	CHECK(material->get_poly_albedo_color_array() == expected);
	CHECK((material->get_albedo_source_flags() & Material4D::COLOR_SOURCE_FLAG_PER_CELL) != 0);
	// No material library, so the base color is white and there is no single color to multiply by.
	CHECK(!(material->get_albedo_source_flags() & Material4D::COLOR_SOURCE_FLAG_SINGLE_COLOR));
}

TEST_CASE("[FourDODocument4D] Vertex colors and missing normals on tetrahedra") {
	// The vertex format is position/normal/color. Vertex 3 of the first cell has neither a normal nor a color.
	const String text = "4DO 2\n"
						"co 255 0 0\n"
						"co 0 255 0\n"
						"vtxformat v/vn/vc\n"
						"vn 0 0 0 1\n"
						"v 0 0 0 0\n"
						"v 1 0 0 0\n"
						"v 0 1 0 0\n"
						"v 0 0 1 0\n"
						"v 0 0 0 1\n"
						"usemtl tets\n"
						"t 0/0/0 1/0/0 2/0/1 3//\n"
						"t 0/0/0 1/0/0 2/0/1 4/0/1\n";
	const Ref<FourDODocument4D> doc = read_4do_text(text);
	REQUIRE(doc.is_valid());
	const Ref<MultiSurfaceMesh4D> mesh = doc->import_generate_multi_surface_mesh_4d();
	REQUIRE(mesh.is_valid());
	CHECK(mesh->is_mesh_data_valid());
	const Ref<ArrayTetraMesh4D> tetra_mesh = find_surface(mesh, "tets");
	REQUIRE(tetra_mesh.is_valid());
	CHECK(tetra_mesh->is_mesh_data_valid());
	CHECK(tetra_mesh->get_simplex_cell_vertex_indices().size() == 8);
	// The one normal from the file, plus a flat shading normal per cell for the vertex that had none.
	const PackedVector4Array normal_values = tetra_mesh->get_normal_values();
	CHECK(normal_values.size() == 3);
	CHECK(normal_values[0] == Vector4(0, 0, 0, 1));
	const PackedInt32Array normal_indices = tetra_mesh->get_simplex_cell_normal_indices();
	REQUIRE(normal_indices.size() == 8);
	CHECK(normal_indices[0] == 0);
	CHECK(normal_indices[3] == 1); // Cell 0's flat shading normal.
	CHECK(normal_indices[7] == 0);
	for (const int32_t normal_index : normal_indices) {
		CHECK(normal_index >= 0);
		CHECK(normal_index < normal_values.size());
	}
	// Per-vertex colors, indexed by vertex position, with white for the uncolored vertex 3.
	const Ref<TetraMaterial4D> material = tetra_mesh->get_material();
	REQUIRE(material.is_valid());
	CHECK(Ref<PolyMaterial4D>(material).is_null());
	const PackedColorArray expected = { Color(1, 0, 0, 1), Color(1, 0, 0, 1), Color(0, 1, 0, 1), Color(1, 1, 1, 1), Color(0, 1, 0, 1) };
	CHECK(material->get_albedo_color_array() == expected);
	CHECK((material->get_albedo_source_flags() & Material4D::COLOR_SOURCE_FLAG_PER_VERT) != 0);
}

TEST_CASE("[FourDODocument4D] Cells face outward despite the inverted 4DO vertex order convention") {
	// A HoxelDraw cuboid in Z-order and a tetrahedron, both listed in the convention where the perpendicular
	// of the edges from vertex 0 points into the mesh, which must be inverted on import.
	const String text = String("4DO 2\n"
							   "usemtl cubes\n") +
			TWO_CUBE_VERTICES +
			"c 7 6 5 4 3 2 1 0\n"
			"c 15 14 13 12 11 10 9 8\n"
			"usemtl tets\n"
			"v 0 0 0 0\n"
			"v 1 0 0 0\n"
			"v 0 1 0 0\n"
			"v 0 0 1 0\n"
			"t 16 17 18 19\n";
	const Ref<FourDODocument4D> doc = read_4do_text(text);
	REQUIRE(doc.is_valid());
	const Ref<MultiSurfaceMesh4D> mesh = doc->import_generate_multi_surface_mesh_4d();
	REQUIRE(mesh.is_valid());
	CHECK(mesh->is_mesh_data_valid());

	// Cuboids: the boundary normal is the negated perpendicular of the Z-order edges from vertex 0 to vertices 1, 2, and 4.
	const Ref<ArrayPolyMesh4D> poly_mesh = find_surface(mesh, "cubes");
	REQUIRE(poly_mesh.is_valid());
	const PackedVector4Array normals = poly_mesh->get_poly_cell_boundary_normals();
	REQUIRE(normals.size() == 2);
	const int32_t cube_orders[2][8] = { { 7, 6, 5, 4, 3, 2, 1, 0 }, { 15, 14, 13, 12, 11, 10, 9, 8 } };
	// Deduplication can reorder vertices, so look positions up from the document's own vertex list instead.
	PackedVector4Array file_vertices;
	for (const String &line : text.split("\n", false)) {
		const PackedStringArray tokens = line.split(" ", false);
		if (tokens[0] == "v") {
			file_vertices.append(Vector4(tokens[1].to_float(), tokens[2].to_float(), tokens[3].to_float(), tokens[4].to_float()));
		}
	}
	for (int cube = 0; cube < 2; cube++) {
		const Vector4 &v0 = file_vertices[cube_orders[cube][0]];
		const Vector4 inward = Vector4D::perpendicular(file_vertices[cube_orders[cube][1]] - v0, file_vertices[cube_orders[cube][2]] - v0, file_vertices[cube_orders[cube][4]] - v0);
		CHECK(normals[cube].is_equal_approx(-inward.normalized()));
	}
	// The decomposed tetrahedra face the same way as their cell, and the cells of the two cubes (at x=2 and x=3) face opposite ways.
	const PackedVector4Array tet_positions = poly_mesh->get_simplex_cell_positions();
	REQUIRE(tet_positions.size() > 0);
	for (int64_t t = 0; t < tet_positions.size() / 4; t++) {
		const Vector4 &a = tet_positions[t * 4];
		const Vector4 perp = Vector4D::perpendicular(tet_positions[t * 4 + 1] - a, tet_positions[t * 4 + 2] - a, tet_positions[t * 4 + 3] - a);
		const int cube = Math::is_equal_approx(a.x, (real_t)2.0) ? 0 : 1;
		CHECK(perp.dot(normals[cube]) > 0.0);
	}

	// Tetrahedra: the imported vertex order gives the negated perpendicular of the file's order.
	const Ref<ArrayTetraMesh4D> tetra_mesh = find_surface(mesh, "tets");
	REQUIRE(tetra_mesh.is_valid());
	const Vector4 file_inward = Vector4D::perpendicular(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0));
	const PackedVector4Array positions = tetra_mesh->get_simplex_cell_positions();
	REQUIRE(positions.size() == 4);
	const Vector4 imported_perp = Vector4D::perpendicular(positions[1] - positions[0], positions[2] - positions[0], positions[3] - positions[0]);
	CHECK(imported_perp.normalized().is_equal_approx(-file_inward.normalized()));
	const PackedVector4Array tetra_normals = tetra_mesh->get_simplex_cell_boundary_normals();
	REQUIRE(tetra_normals.size() == 1);
	CHECK(tetra_normals[0].is_equal_approx(-file_inward.normalized()));
}

TEST_CASE("[FourDODocument4D] Cells referencing missing vertices fail the import with an error") {
	// The file's cell indices are invalid input, so they must produce an error and no mesh, never a crash.
	const String tetra_only = "4DO 2\n"
							  "v 0 0 0 0\n"
							  "v 1 0 0 0\n"
							  "v 0 1 0 0\n"
							  "usemtl tets\n"
							  "t 0 1 2 99\n";
	const String with_cuboid = String("4DO 2\n"
									  "usemtl cubes\n") +
			TWO_CUBE_VERTICES +
			"c 7 6 5 4 3 2 1 0\n"
			"c 15 14 13 12 11 10 9 99\n";
	const String mixed = String("4DO 2\n"
								"usemtl mixed\n") +
			TWO_CUBE_VERTICES +
			"c 7 6 5 4 3 2 1 0\n"
			"t 0 1 2 99\n";
	for (const String &text : { tetra_only, with_cuboid, mixed }) {
		CAPTURE(text);
		const Ref<FourDODocument4D> doc = read_4do_text(text);
		REQUIRE(doc.is_valid());
		ERR_PRINT_OFF;
		const Ref<MultiSurfaceMesh4D> mesh = doc->import_generate_multi_surface_mesh_4d();
		ERR_PRINT_ON;
		CHECK(mesh.is_null());
	}
}

TEST_CASE("[FourDODocument4D] Unrecognized commands are skipped instead of rejecting the file") {
	const String text = String("4DO 2\n"
							   "orient +X +Y +Z +W\n"
							   "loneword\n"
							   "usemtl cubes\n") +
			TWO_CUBE_VERTICES +
			"c 7 6 5 4 3 2 1 0\n"
			"c 15 14 13 12 11 10 9 8\n";
	ERR_PRINT_OFF; // The unknown and malformed lines intentionally warn.
	const Ref<FourDODocument4D> doc = read_4do_text(text);
	ERR_PRINT_ON;
	REQUIRE(doc.is_valid());
	const Ref<MultiSurfaceMesh4D> mesh = doc->import_generate_multi_surface_mesh_4d();
	REQUIRE(mesh.is_valid());
	CHECK(mesh->is_mesh_data_valid());
	const Ref<ArrayPolyMesh4D> poly_mesh = find_surface(mesh, "cubes");
	REQUIRE(poly_mesh.is_valid());
	CHECK(poly_mesh->get_poly_cell_indices()[1].size() == 2);
	// Without colors or a material library, the surface has no material at all.
	CHECK(poly_mesh->get_material().is_null());
}

TEST_CASE("[FourDODocument4D] Returning to a polyline material preserves its earlier segments") {
	const Ref<FourDODocument4D> doc = read_4do_text("4DO 2\nv 0 0 0 0\nv 1 0 0 0\nv 2 0 0 0\nv 3 0 0 0\nusemtl A\npl 0 1\nusemtl B\npl 1 2\nusemtl A\npl 2 3\n");
	REQUIRE(doc.is_valid());
	const Ref<MultiSurfaceMesh4D> mesh = doc->import_generate_multi_surface_mesh_4d();
	REQUIRE(mesh.is_valid());
	REQUIRE(mesh->get_surface_meshes().size() == 2);
	const Ref<SingleSurfaceMesh4D> surface_a = find_surface(mesh, "A");
	const Ref<SingleSurfaceMesh4D> surface_b = find_surface(mesh, "B");
	REQUIRE(surface_a.is_valid());
	REQUIRE(surface_b.is_valid());
	CHECK(surface_a->get_edge_indices() == PackedInt32Array({ 0, 1, 2, 3 }));
	CHECK(surface_b->get_edge_indices() == PackedInt32Array({ 1, 2 }));
}
} // namespace TestFourDODocument4D
