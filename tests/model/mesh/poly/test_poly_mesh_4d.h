#pragma once

#include "../../../../model/mesh/poly/array_poly_mesh_4d.h"
#include "../../../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../../../../model/mesh/poly/orthoplex_poly_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestPolyMesh4D {
// The faces of a tetrahedron with vertices 0, 1, 2, 3, encoded as lists of edge indices.
// Edges: 0:(0,1) 1:(0,2) 2:(0,3) 3:(1,2) 4:(1,3) 5:(2,3)
// Faces: 0: verts 0-1-2, 1: verts 0-1-3, 2: verts 0-2-3, 3: verts 1-2-3.
inline Vector<PackedInt32Array> tetrahedron_cell_faces() {
	Vector<PackedInt32Array> faces;
	faces.push_back(PackedInt32Array{ 0, 3, 1 });
	faces.push_back(PackedInt32Array{ 0, 4, 2 });
	faces.push_back(PackedInt32Array{ 1, 5, 2 });
	faces.push_back(PackedInt32Array{ 3, 5, 4 });
	return faces;
}

// Builds a poly mesh with a single tetrahedral 3D boundary cell in the w=0 hyperplane.
// The cell's faces can be given in any order, and the face edge lists can be overridden,
// to test how orientation is derived from the order of a cell's elements.
inline Ref<ArrayPolyMesh4D> make_tetrahedron_cell_mesh(const PackedInt32Array &p_cell_faces = PackedInt32Array{ 0, 1, 2, 3 }, const Vector<PackedInt32Array> &p_face_overrides = Vector<PackedInt32Array>()) {
	Ref<ArrayPolyMesh4D> mesh;
	mesh.instantiate();
	mesh->append_vertex(Vector4(0, 0, 0, 0));
	mesh->append_vertex(Vector4(1, 0, 0, 0));
	mesh->append_vertex(Vector4(0, 1, 0, 0));
	mesh->append_vertex(Vector4(0, 0, 1, 0));
	mesh->append_edge_indices(0, 1);
	mesh->append_edge_indices(0, 2);
	mesh->append_edge_indices(0, 3);
	mesh->append_edge_indices(1, 2);
	mesh->append_edge_indices(1, 3);
	mesh->append_edge_indices(2, 3);
	const Vector<PackedInt32Array> faces = p_face_overrides.is_empty() ? tetrahedron_cell_faces() : p_face_overrides;
	for (int64_t face_index = 0; face_index < faces.size(); face_index++) {
		mesh->append_poly_cell(2, faces[face_index], false);
	}
	mesh->append_poly_cell(3, p_cell_faces, false);
	return mesh;
}

TEST_CASE("[PolyMesh4D] Validate poly mesh data") {
	SUBCASE("A single tetrahedral cell is valid") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "A hand-built tetrahedral cell should be valid poly mesh data.");
		CHECK_MESSAGE(mesh->is_mesh_data_valid(), "A hand-built tetrahedral cell should also pass full mesh validation.");
	}

	SUBCASE("An empty mesh is trivially valid") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "An empty mesh has nothing invalid in it.");
	}

	SUBCASE("Odd edge index count is invalid") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->append_vertex(Vector4(0, 0, 0, 0));
		mesh->append_vertex(Vector4(1, 0, 0, 0));
		mesh->set_edge_vertex_indices(PackedInt32Array{ 0, 1, 0 });
		ERR_PRINT_OFF;
		CHECK_FALSE_MESSAGE(mesh->is_poly_mesh_data_valid(), "Edge indices must come in pairs of vertices.");
		ERR_PRINT_ON;
	}

	SUBCASE("Edge referencing a non-existent vertex is invalid") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->append_vertex(Vector4(0, 0, 0, 0));
		mesh->append_vertex(Vector4(1, 0, 0, 0));
		mesh->set_edge_vertex_indices(PackedInt32Array{ 0, 5 });
		ERR_PRINT_OFF;
		CHECK_FALSE_MESSAGE(mesh->is_poly_mesh_data_valid(), "Edges must reference vertices that exist.");
		ERR_PRINT_ON;
	}

	SUBCASE("Face with fewer than 3 edges is invalid") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Vector<Vector<PackedInt32Array>> poly_cell_indices;
		Vector<PackedInt32Array> faces;
		faces.push_back(PackedInt32Array{ 0, 3 });
		poly_cell_indices.push_back(faces);
		mesh->set_poly_cell_indices(poly_cell_indices);
		ERR_PRINT_OFF;
		CHECK_FALSE_MESSAGE(mesh->is_poly_mesh_data_valid(), "A 2D face requires at least 3 edges.");
		ERR_PRINT_ON;
	}

	SUBCASE("Face whose first two edges do not share a vertex is invalid") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->append_vertex(Vector4(0, 0, 0, 0));
		mesh->append_vertex(Vector4(1, 0, 0, 0));
		mesh->append_vertex(Vector4(1, 1, 0, 0));
		mesh->append_vertex(Vector4(0, 1, 0, 0));
		mesh->append_edge_indices(0, 1); // Edge 0.
		mesh->append_edge_indices(1, 2); // Edge 1.
		mesh->append_edge_indices(2, 3); // Edge 2.
		mesh->append_edge_indices(0, 3); // Edge 3.
		// Edges 0 and 2 are opposite sides of the square, so orientation is not determinable.
		mesh->append_poly_cell(2, PackedInt32Array{ 0, 2, 1, 3 }, false);
		ERR_PRINT_OFF;
		CHECK_FALSE_MESSAGE(mesh->is_poly_mesh_data_valid(), "The first two edges of a face must share a vertex.");
		ERR_PRINT_ON;
	}

	SUBCASE("Cell whose first two faces do not share an edge is invalid") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		// Two disconnected triangles.
		mesh->append_vertex(Vector4(0, 0, 0, 0));
		mesh->append_vertex(Vector4(1, 0, 0, 0));
		mesh->append_vertex(Vector4(0, 1, 0, 0));
		mesh->append_vertex(Vector4(5, 0, 0, 0));
		mesh->append_vertex(Vector4(6, 0, 0, 0));
		mesh->append_vertex(Vector4(5, 1, 0, 0));
		mesh->append_edge_indices(0, 1); // Edge 0.
		mesh->append_edge_indices(0, 2); // Edge 1.
		mesh->append_edge_indices(1, 2); // Edge 2.
		mesh->append_edge_indices(3, 4); // Edge 3.
		mesh->append_edge_indices(3, 5); // Edge 4.
		mesh->append_edge_indices(4, 5); // Edge 5.
		mesh->append_poly_cell(2, PackedInt32Array{ 0, 2, 1 }, false);
		mesh->append_poly_cell(2, PackedInt32Array{ 3, 5, 4 }, false);
		mesh->append_poly_cell(3, PackedInt32Array{ 0, 1, 0, 1 }, false);
		ERR_PRINT_OFF;
		CHECK_FALSE_MESSAGE(mesh->is_poly_mesh_data_valid(), "The first two faces of a 3D cell must share an edge.");
		ERR_PRINT_ON;
	}

	SUBCASE("4D cell whose first two 3D cells do not share a face is invalid") {
		Ref<BoxPolyMesh4D> box;
		box.instantiate();
		Ref<ArrayPolyMesh4D> mesh = box->to_array_poly_mesh();
		// Cells 0 (-W) and 7 (+W) are opposite cells of the tesseract and share no face.
		Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		Vector<PackedInt32Array> hyper_cells = poly_cell_indices[2];
		hyper_cells.set(0, PackedInt32Array{ 0, 7, 1, 2, 3, 4, 5, 6 });
		poly_cell_indices.set(2, hyper_cells);
		mesh->set_poly_cell_indices(poly_cell_indices);
		ERR_PRINT_OFF;
		CHECK_FALSE_MESSAGE(mesh->is_poly_mesh_data_valid(), "The first two 3D cells of a 4D cell must share a face.");
		ERR_PRINT_ON;
	}

	SUBCASE("Boundary normals count must match boundary cell count") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		mesh->set_poly_cell_boundary_normals(PackedVector4Array{ Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1) });
		ERR_PRINT_OFF;
		CHECK_FALSE_MESSAGE(mesh->is_poly_mesh_data_valid(), "Two boundary normals for one cell is invalid.");
		ERR_PRINT_ON;
		mesh->set_poly_cell_boundary_normals(PackedVector4Array{ Vector4(0, 0, 0, 1) });
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "One boundary normal for one cell is valid.");
	}

	SUBCASE("Boundary pivot overrides must reference valid vertices") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		mesh->set_poly_cell_boundary_pivot_overrides(PackedInt32Array{ 99 });
		ERR_PRINT_OFF;
		CHECK_FALSE_MESSAGE(mesh->is_poly_mesh_data_valid(), "Pivot override referencing a non-existent vertex is invalid.");
		ERR_PRINT_ON;
		mesh->set_poly_cell_boundary_pivot_overrides(PackedInt32Array{ -1 });
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Pivot override of -1 means no override, which is valid.");
		mesh->set_poly_cell_boundary_pivot_overrides(PackedInt32Array{ 0 });
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Pivot override referencing an existing vertex is valid.");
	}

	SUBCASE("Vertex normals count must match cell vertex count") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Vector<PackedVector4Array> vertex_normals;
		vertex_normals.push_back(PackedVector4Array{ Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1) });
		mesh->set_poly_cell_vertex_normals(vertex_normals);
		ERR_PRINT_OFF;
		CHECK_FALSE_MESSAGE(mesh->is_poly_mesh_data_valid(), "Three vertex normals for a cell with four vertices is invalid.");
		ERR_PRINT_ON;
		vertex_normals.set(0, PackedVector4Array{ Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1) });
		mesh->set_poly_cell_vertex_normals(vertex_normals);
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Four vertex normals for a cell with four vertices is valid.");
		vertex_normals.set(0, PackedVector4Array());
		mesh->set_poly_cell_vertex_normals(vertex_normals);
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Cells are allowed to be missing vertex normals.");
	}

	SUBCASE("Texture map count must match cell vertex count") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Vector<PackedVector3Array> texture_map;
		texture_map.push_back(PackedVector3Array{ Vector3(0, 0, 0), Vector3(1, 0, 0) });
		mesh->set_poly_cell_texture_map(texture_map);
		ERR_PRINT_OFF;
		CHECK_FALSE_MESSAGE(mesh->is_poly_mesh_data_valid(), "Two texture map entries for a cell with four vertices is invalid.");
		ERR_PRINT_ON;
		texture_map.set(0, PackedVector3Array{ Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) });
		mesh->set_poly_cell_texture_map(texture_map);
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Four texture map entries for a cell with four vertices is valid.");
		texture_map.set(0, PackedVector3Array());
		mesh->set_poly_cell_texture_map(texture_map);
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Cells are allowed to be missing texture map data.");
	}
}

TEST_CASE("[PolyMesh4D] Face vertex indices") {
	SUBCASE("Tetrahedron faces produce winding-ordered triangles") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		const Vector<PackedInt32Array> face_vertex_indices = mesh->get_all_face_vertex_indices();
		REQUIRE(face_vertex_indices.size() == 4);
		// The first two edges of each face share a vertex, which is placed in the middle.
		CHECK((face_vertex_indices[0] == PackedInt32Array{ 0, 1, 2 }));
		CHECK((face_vertex_indices[1] == PackedInt32Array{ 0, 1, 3 }));
		CHECK((face_vertex_indices[2] == PackedInt32Array{ 0, 2, 3 }));
		CHECK((face_vertex_indices[3] == PackedInt32Array{ 1, 2, 3 }));
	}

	SUBCASE("Swapping the first two edges of a face reverses its 3-vertex canonical span") {
		Vector<PackedInt32Array> faces = tetrahedron_cell_faces();
		faces.set(0, PackedInt32Array{ 3, 0, 1 }); // Swap the first two edges of face 0.
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh(PackedInt32Array{ 0, 1, 2, 3 }, faces);
		const Vector<PackedInt32Array> face_vertex_indices = mesh->get_all_poly_cell_vertex_indices(2, true);
		REQUIRE(face_vertex_indices.size() == 4);
		CHECK_MESSAGE((face_vertex_indices[0] == PackedInt32Array{ 2, 1, 0 }), "Swapping the first two edges of a face must reverse the face's canonical span.");
	}

	SUBCASE("Box faces are quadrilaterals") {
		Ref<BoxPolyMesh4D> box;
		box.instantiate();
		const Vector<PackedInt32Array> face_vertex_indices = box->get_all_face_vertex_indices();
		REQUIRE(face_vertex_indices.size() == 24);
		for (int64_t face_index = 0; face_index < face_vertex_indices.size(); face_index++) {
			CHECK_MESSAGE(face_vertex_indices[face_index].size() == 4, "Each face of a box should have 4 vertices.");
		}
	}
}

TEST_CASE("[PolyMesh4D] Boundary cell vertex indices") {
	SUBCASE("Tetrahedron cell with and without canonical span") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		const Vector<PackedInt32Array> with_span = mesh->get_all_boundary_cell_vertex_indices(true);
		REQUIRE(with_span.size() == 1);
		// Faces 0 (verts 0-1-2) and 1 (verts 0-1-3) share edge 0 (verts 0-1). The canonical span
		// starts with the non-shared vertex of the next edge of face 0, then the shared edge's
		// vertices, then the non-shared vertex of the next edge of face 1.
		CHECK((with_span[0] == PackedInt32Array{ 2, 0, 1, 3 }));
		const Vector<PackedInt32Array> without_span = mesh->get_all_boundary_cell_vertex_indices(false);
		REQUIRE(without_span.size() == 1);
		CHECK((without_span[0] == PackedInt32Array{ 0, 1, 2, 3 }));
	}

	SUBCASE("Swapping the first two faces of a cell changes the canonical span") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh(PackedInt32Array{ 1, 0, 2, 3 });
		const Vector<PackedInt32Array> with_span = mesh->get_all_boundary_cell_vertex_indices(true);
		REQUIRE(with_span.size() == 1);
		CHECK((with_span[0] == PackedInt32Array{ 3, 0, 1, 2 }));
	}

	SUBCASE("Box cells have 8 vertices each with a distinct 4-vertex canonical span") {
		Ref<BoxPolyMesh4D> box;
		box.instantiate();
		const Vector<PackedInt32Array> with_span = box->get_all_boundary_cell_vertex_indices(true);
		const Vector<PackedInt32Array> without_span = box->get_all_boundary_cell_vertex_indices(false);
		REQUIRE(with_span.size() == 8);
		REQUIRE(without_span.size() == 8);
		for (int64_t cell_index = 0; cell_index < with_span.size(); cell_index++) {
			const PackedInt32Array &cell_vertices = with_span[cell_index];
			CHECK_MESSAGE(cell_vertices.size() == 8, "Each cell of a box should have 8 vertices.");
			CHECK_MESSAGE(without_span[cell_index].size() == 8, "Each cell of a box should have 8 vertices.");
			// The first 4 vertices are the canonical span and must be distinct.
			for (int64_t i = 0; i < 4; i++) {
				for (int64_t j = i + 1; j < 4; j++) {
					CHECK_MESSAGE(cell_vertices[i] != cell_vertices[j], "The canonical span must consist of distinct vertices.");
				}
				CHECK_MESSAGE(without_span[cell_index].has(cell_vertices[i]), "Both orderings must contain the same vertices.");
			}
		}
	}

	SUBCASE("Orthoplex cells have 4 vertices each") {
		Ref<OrthoplexPolyMesh4D> orthoplex;
		orthoplex.instantiate();
		const Vector<PackedInt32Array> with_span = orthoplex->get_all_boundary_cell_vertex_indices(true);
		REQUIRE(with_span.size() == 16);
		for (int64_t cell_index = 0; cell_index < with_span.size(); cell_index++) {
			CHECK_MESSAGE(with_span[cell_index].size() == 4, "Each cell of an orthoplex is a tetrahedron with 4 vertices.");
		}
	}
}

TEST_CASE("[PolyMesh4D] Cell orientation determines simplex boundary normals for all face permutations") {
	// The critical property of the canonical span: the orientation of a 3D cell is controlled
	// entirely by the order of its first two faces, and swapping them flips the normal vector.
	// The test tetrahedron is flat in the w=0 hyperplane, so every orientation-derived normal
	// must be exactly +W or -W. This exhaustively tests all 24 permutations of the cell's faces
	// through the downstream simplex decomposition used for rendering.
	Vector4 pair_normals[4][4];
	bool pair_seen[4][4] = {};
	for (int32_t first = 0; first < 4; first++) {
		for (int32_t second = 0; second < 4; second++) {
			if (second == first) {
				continue;
			}
			for (int32_t third = 0; third < 4; third++) {
				if (third == first || third == second) {
					continue;
				}
				const int32_t fourth = 6 - first - second - third;
				Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh(PackedInt32Array{ first, second, third, fourth });
				CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Any permutation of a tetrahedron's faces is a valid cell.");
				const PackedVector4Array simplex_normals = mesh->get_simplex_cell_boundary_normals();
				REQUIRE_MESSAGE(simplex_normals.size() == 1, "A single tetrahedral cell must decompose into exactly one simplex.");
				const Vector4 normal = simplex_normals[0];
				CHECK_MESSAGE(Math::is_equal_approx(Math::abs(normal.w), (real_t)1.0), "The normal of a cell flat in the w=0 hyperplane must be +W or -W.");
				if (pair_seen[first][second]) {
					CHECK_MESSAGE(normal.is_equal_approx(pair_normals[first][second]), "The order of faces after the first two must not affect the normal.");
				} else {
					pair_normals[first][second] = normal;
					pair_seen[first][second] = true;
				}
			}
		}
	}
	// Verify the anchor orientation, computed by hand from the canonical span {2, 0, 1, 3}.
	CHECK_MESSAGE(pair_normals[0][1].is_equal_approx(Vector4(0, 0, 0, 1)), "The face order {0, 1, 2, 3} must produce a +W normal.");
	// Verify that swapping the first two faces flips the normal, for every possible pair.
	for (int32_t first = 0; first < 4; first++) {
		for (int32_t second = first + 1; second < 4; second++) {
			CHECK_MESSAGE(pair_normals[first][second].is_equal_approx(-pair_normals[second][first]), "Swapping the first two faces of a cell must flip its normal vector.");
		}
	}
}

TEST_CASE("[PolyMesh4D] Cell orientation is not affected by the edge order within triangle faces") {
	// Per the canonical span design, rearranging the elements inside lower-dimensional cells
	// must not affect the orientation of the higher-dimensional cell. For triangle faces this
	// is exact: any of the 6 orderings of either face's edges must give the same +W normal.
	const int32_t edge_permutations[6][3] = {
		{ 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 }, { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 }
	};
	const Vector<PackedInt32Array> base_faces = tetrahedron_cell_faces();
	for (int64_t face0_perm = 0; face0_perm < 6; face0_perm++) {
		for (int64_t face1_perm = 0; face1_perm < 6; face1_perm++) {
			Vector<PackedInt32Array> faces = base_faces;
			const PackedInt32Array &face0 = base_faces[0];
			const PackedInt32Array &face1 = base_faces[1];
			faces.set(0, PackedInt32Array{ face0[edge_permutations[face0_perm][0]], face0[edge_permutations[face0_perm][1]], face0[edge_permutations[face0_perm][2]] });
			faces.set(1, PackedInt32Array{ face1[edge_permutations[face1_perm][0]], face1[edge_permutations[face1_perm][1]], face1[edge_permutations[face1_perm][2]] });
			Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh(PackedInt32Array{ 0, 1, 2, 3 }, faces);
			CHECK(mesh->is_poly_mesh_data_valid());
			const PackedVector4Array simplex_normals = mesh->get_simplex_cell_boundary_normals();
			REQUIRE(simplex_normals.size() == 1);
			CHECK_MESSAGE(simplex_normals[0].is_equal_approx(Vector4(0, 0, 0, 1)), "Rearranging the edges within triangle faces must not affect the cell's normal.");
		}
	}
}

TEST_CASE("[PolyMesh4D] Canonical span of 4D cells") {
	// 4D cells use the general N-dimensional canonical span construction: one vertex of the
	// first member off the shared "ridge" (the face shared by the first two 3D cells), the
	// ridge's lowest vertex indices in ascending order, and one vertex of the second member
	// off the ridge. Compute the expected span for the box hyper-cell from public API data.
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	const Vector<Vector<PackedInt32Array>> box_indices = box->get_poly_cell_indices();
	const PackedInt32Array hyper_cell = box_indices[2][0];
	const PackedInt32Array &first_member = box_indices[1][hyper_cell[0]];
	const PackedInt32Array &second_member = box_indices[1][hyper_cell[1]];
	int32_t ridge_face = -1;
	for (const int32_t first_member_face : first_member) {
		if (second_member.has(first_member_face)) {
			ridge_face = first_member_face;
			break;
		}
	}
	REQUIRE_MESSAGE(ridge_face != -1, "The first two cells of the box hyper-cell must share a face.");
	const Vector<PackedInt32Array> face_vertices = box->get_all_poly_cell_vertex_indices(2, false);
	const Vector<PackedInt32Array> cell_vertices = box->get_all_poly_cell_vertex_indices(3, false);
	const PackedInt32Array ridge_face_vertices = face_vertices[ridge_face];
	PackedInt32Array expected_ridge_span = ridge_face_vertices;
	expected_ridge_span.sort();
	expected_ridge_span.resize(3);
	int32_t expected_first_extension = INT32_MAX;
	for (const int32_t cell_vertex : cell_vertices[hyper_cell[0]]) {
		if (!ridge_face_vertices.has(cell_vertex) && cell_vertex < expected_first_extension) {
			expected_first_extension = cell_vertex;
		}
	}
	int32_t expected_second_extension = INT32_MAX;
	for (const int32_t cell_vertex : cell_vertices[hyper_cell[1]]) {
		if (!ridge_face_vertices.has(cell_vertex) && cell_vertex < expected_second_extension) {
			expected_second_extension = cell_vertex;
		}
	}
	const PackedInt32Array expected_span = PackedInt32Array{ expected_first_extension, expected_ridge_span[0], expected_ridge_span[1], expected_ridge_span[2], expected_second_extension };

	SUBCASE("The span is one vertex off the ridge, the ridge ascending, and one vertex off the other member") {
		const Vector<PackedInt32Array> result = box->get_all_poly_cell_vertex_indices(4, true);
		REQUIRE(result.size() == 1);
		const PackedInt32Array &hyper_vertices = result[0];
		REQUIRE(hyper_vertices.size() == 16);
		for (int64_t i = 0; i < 5; i++) {
			CHECK_MESSAGE(hyper_vertices[i] == expected_span[i], "The 4D cell's canonical span must follow the documented construction.");
		}
		CHECK_MESSAGE(hyper_vertices[1] < hyper_vertices[2], "The ridge vertices must be in ascending order.");
		CHECK_MESSAGE(hyper_vertices[2] < hyper_vertices[3], "The ridge vertices must be in ascending order.");
		for (int64_t i = 0; i < 16; i++) {
			for (int64_t j = i + 1; j < 16; j++) {
				CHECK_MESSAGE(hyper_vertices[i] != hyper_vertices[j], "The hyper-cell's vertex list must not contain duplicates.");
			}
		}
	}

	SUBCASE("Swapping the first two 3D cells of a 4D cell transposes the span's ends") {
		Ref<ArrayPolyMesh4D> mesh = box->to_array_poly_mesh();
		Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		Vector<PackedInt32Array> hyper_cells = poly_cell_indices[2];
		PackedInt32Array swapped_hyper_cell = hyper_cells[0];
		const int32_t temp = swapped_hyper_cell[0];
		swapped_hyper_cell.set(0, swapped_hyper_cell[1]);
		swapped_hyper_cell.set(1, temp);
		hyper_cells.set(0, swapped_hyper_cell);
		poly_cell_indices.set(2, hyper_cells);
		mesh->set_poly_cell_indices(poly_cell_indices);
		const Vector<PackedInt32Array> result = mesh->get_all_poly_cell_vertex_indices(4, true);
		REQUIRE(result.size() == 1);
		const PackedInt32Array &hyper_vertices = result[0];
		REQUIRE(hyper_vertices.size() == 16);
		// The ends swap while the ridge stays in place: one transposition, which is
		// guaranteed to flip the orientation, making it controllable at the 4D cell's level.
		CHECK(hyper_vertices[0] == expected_span[4]);
		CHECK(hyper_vertices[1] == expected_span[1]);
		CHECK(hyper_vertices[2] == expected_span[2]);
		CHECK(hyper_vertices[3] == expected_span[3]);
		CHECK(hyper_vertices[4] == expected_span[0]);
	}

	SUBCASE("The span does not depend on the order of members after the first two") {
		Ref<ArrayPolyMesh4D> mesh = box->to_array_poly_mesh();
		Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		Vector<PackedInt32Array> hyper_cells = poly_cell_indices[2];
		hyper_cells.set(0, PackedInt32Array{ 0, 1, 7, 6, 5, 4, 3, 2 });
		poly_cell_indices.set(2, hyper_cells);
		mesh->set_poly_cell_indices(poly_cell_indices);
		const Vector<PackedInt32Array> result = mesh->get_all_poly_cell_vertex_indices(4, true);
		REQUIRE(result.size() == 1);
		for (int64_t i = 0; i < 5; i++) {
			CHECK_MESSAGE(result[0][i] == expected_span[i], "Reordering the trailing members must not change the canonical span.");
		}
	}

	SUBCASE("The span does not depend on the internal order of lower-dimensional cells") {
		// Rotate the ridge face's edge list, which preserves the winding but changes the storage.
		{
			Ref<ArrayPolyMesh4D> mesh = box->to_array_poly_mesh();
			Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
			Vector<PackedInt32Array> faces = poly_cell_indices[0];
			const PackedInt32Array ridge_face_edges = faces[ridge_face];
			PackedInt32Array rotated_edges;
			for (int64_t i = 0; i < ridge_face_edges.size(); i++) {
				rotated_edges.append(ridge_face_edges[(i + 1) % ridge_face_edges.size()]);
			}
			faces.set(ridge_face, rotated_edges);
			poly_cell_indices.set(0, faces);
			mesh->set_poly_cell_indices(poly_cell_indices);
			const Vector<PackedInt32Array> result = mesh->get_all_poly_cell_vertex_indices(4, true);
			REQUIRE(result.size() == 1);
			for (int64_t i = 0; i < 5; i++) {
				CHECK_MESSAGE(result[0][i] == expected_span[i], "Rotating the ridge face's edges must not change the 4D cell's canonical span.");
			}
		}
		// Swap the ridge face's first two edges, which flips the face's own orientation.
		{
			Ref<ArrayPolyMesh4D> mesh = box->to_array_poly_mesh();
			Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
			Vector<PackedInt32Array> faces = poly_cell_indices[0];
			PackedInt32Array swapped_edges = faces[ridge_face];
			const int32_t temp = swapped_edges[0];
			swapped_edges.set(0, swapped_edges[1]);
			swapped_edges.set(1, temp);
			faces.set(ridge_face, swapped_edges);
			poly_cell_indices.set(0, faces);
			mesh->set_poly_cell_indices(poly_cell_indices);
			const Vector<PackedInt32Array> result = mesh->get_all_poly_cell_vertex_indices(4, true);
			REQUIRE(result.size() == 1);
			for (int64_t i = 0; i < 5; i++) {
				CHECK_MESSAGE(result[0][i] == expected_span[i], "Flipping the ridge face's orientation must not change the 4D cell's canonical span.");
			}
		}
		// Swap the first member cell's first two faces, which flips that 3D cell's orientation.
		{
			Ref<ArrayPolyMesh4D> mesh = box->to_array_poly_mesh();
			Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
			Vector<PackedInt32Array> cells = poly_cell_indices[1];
			PackedInt32Array swapped_faces = cells[hyper_cell[0]];
			const int32_t temp = swapped_faces[0];
			swapped_faces.set(0, swapped_faces[1]);
			swapped_faces.set(1, temp);
			cells.set(hyper_cell[0], swapped_faces);
			poly_cell_indices.set(1, cells);
			mesh->set_poly_cell_indices(poly_cell_indices);
			const Vector<PackedInt32Array> result = mesh->get_all_poly_cell_vertex_indices(4, true);
			REQUIRE(result.size() == 1);
			for (int64_t i = 0; i < 5; i++) {
				CHECK_MESSAGE(result[0][i] == expected_span[i], "Flipping a member 3D cell's orientation must not change the 4D cell's canonical span.");
			}
		}
	}
}

TEST_CASE("[PolyMesh4D] Simplex decomposition of a single tetrahedron cell") {
	Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
	const PackedInt32Array simplex_indices = mesh->get_simplex_cell_indices();
	REQUIRE_MESSAGE(simplex_indices.size() == 4, "A tetrahedral cell must decompose into exactly one simplex.");
	// The simplex must use all four vertices, each exactly once.
	for (int32_t vertex_index = 0; vertex_index < 4; vertex_index++) {
		CHECK_MESSAGE(simplex_indices.has(vertex_index), "The single simplex must use every vertex of the tetrahedral cell.");
	}
	CHECK_MESSAGE(mesh->get_vertices() == mesh->get_poly_cell_vertices(), "Decomposing a single tetrahedral cell should not add any vertices.");
	CHECK_MESSAGE(mesh->get_source_poly_cell_for_simplex_cell(0) == 0, "The simplex must map back to the boundary cell it came from.");
	CHECK_MESSAGE(mesh->get_source_poly_cell_for_simplex_cell(-1) == -1, "Out of range simplex indices must map to -1.");
	CHECK_MESSAGE(mesh->get_source_poly_cell_for_simplex_cell(1) == -1, "Out of range simplex indices must map to -1.");
	const PackedVector4Array simplex_normals = mesh->get_simplex_cell_boundary_normals();
	REQUIRE(simplex_normals.size() == 1);
	CHECK_MESSAGE(simplex_normals[0].is_equal_approx(Vector4(0, 0, 0, 1)), "The simplex normal must match the cell orientation.");
	CHECK_MESSAGE(mesh->get_simplex_cell_vertex_normals().is_empty(), "No vertex normal data means no simplex vertex normals.");
	CHECK_MESSAGE(mesh->get_simplex_cell_texture_map().is_empty(), "No texture map data means no simplex texture map.");
}

TEST_CASE("[PolyMesh4D] Simplex decomposition of a box") {
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	const PackedInt32Array simplex_indices = box->get_simplex_cell_indices();
	REQUIRE_MESSAGE(simplex_indices.size() % 4 == 0, "Simplex cell indices must come in groups of 4.");
	const int64_t simplex_count = simplex_indices.size() / 4;
	CHECK_MESSAGE(simplex_count == 48, "Each of the 8 cube cells should decompose into 6 tetrahedra.");
	const PackedVector4Array curated_normals = box->get_poly_cell_boundary_normals();
	const PackedVector4Array simplex_normals = box->get_simplex_cell_boundary_normals();
	REQUIRE(simplex_normals.size() == simplex_count);
	PackedInt32Array simplexes_per_cell;
	simplexes_per_cell.resize_initialized(8);
	for (int64_t simplex_index = 0; simplex_index < simplex_count; simplex_index++) {
		const int32_t source_cell = box->get_source_poly_cell_for_simplex_cell(simplex_index);
		REQUIRE_MESSAGE(source_cell >= 0, "Every simplex must map back to a source boundary cell.");
		REQUIRE(source_cell < 8);
		simplexes_per_cell.set(source_cell, simplexes_per_cell[source_cell] + 1);
		CHECK_MESSAGE(simplex_normals[simplex_index].is_equal_approx(curated_normals[source_cell]), "Each simplex normal must match the curated normal of its source cell.");
		// The 4 vertices of each simplex must be distinct.
		for (int64_t i = 0; i < 4; i++) {
			for (int64_t j = i + 1; j < 4; j++) {
				CHECK(simplex_indices[simplex_index * 4 + i] != simplex_indices[simplex_index * 4 + j]);
			}
		}
	}
	for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
		CHECK_MESSAGE(simplexes_per_cell[cell_index] == 6, "Each cube cell should contribute 6 tetrahedra.");
	}
	// Vertex normals: the box data uses flat shading, so all simplex vertex normals match the source cell.
	const PackedVector4Array simplex_vertex_normals = box->get_simplex_cell_vertex_normals();
	REQUIRE(simplex_vertex_normals.size() == simplex_count * 4);
	for (int64_t simplex_index = 0; simplex_index < simplex_count; simplex_index++) {
		const int32_t source_cell = box->get_source_poly_cell_for_simplex_cell(simplex_index);
		for (int64_t vertex_in_simplex = 0; vertex_in_simplex < 4; vertex_in_simplex++) {
			CHECK(simplex_vertex_normals[simplex_index * 4 + vertex_in_simplex].is_equal_approx(curated_normals[source_cell]));
		}
	}
	// Texture map: all curated box texture coordinates are within the 0 to 1 range.
	const PackedVector3Array simplex_texture_map = box->get_simplex_cell_texture_map();
	REQUIRE(simplex_texture_map.size() == simplex_count * 4);
	for (int64_t i = 0; i < simplex_texture_map.size(); i++) {
		const Vector3 texcoord = simplex_texture_map[i];
		CHECK(texcoord.x >= (real_t)0.0);
		CHECK(texcoord.y >= (real_t)0.0);
		CHECK(texcoord.z >= (real_t)0.0);
		CHECK(texcoord.x <= (real_t)1.0);
		CHECK(texcoord.y <= (real_t)1.0);
		CHECK(texcoord.z <= (real_t)1.0);
	}
}

TEST_CASE("[PolyMesh4D] Simplex decomposition of an orthoplex") {
	Ref<OrthoplexPolyMesh4D> orthoplex;
	orthoplex.instantiate();
	const PackedInt32Array simplex_indices = orthoplex->get_simplex_cell_indices();
	REQUIRE_MESSAGE(simplex_indices.size() == 16 * 4, "Each of the 16 tetrahedral cells decomposes into exactly one simplex.");
	CHECK_MESSAGE(orthoplex->get_vertices().size() == 8, "Decomposing an orthoplex should not add any vertices.");
	const PackedVector4Array curated_normals = orthoplex->get_poly_cell_boundary_normals();
	const PackedVector4Array simplex_normals = orthoplex->get_simplex_cell_boundary_normals();
	REQUIRE(simplex_normals.size() == 16);
	PackedInt32Array simplexes_per_cell;
	simplexes_per_cell.resize_initialized(16);
	for (int64_t simplex_index = 0; simplex_index < 16; simplex_index++) {
		const int32_t source_cell = orthoplex->get_source_poly_cell_for_simplex_cell(simplex_index);
		REQUIRE(source_cell >= 0);
		REQUIRE(source_cell < 16);
		simplexes_per_cell.set(source_cell, simplexes_per_cell[source_cell] + 1);
		CHECK_MESSAGE(simplex_normals[simplex_index].is_equal_approx(curated_normals[source_cell].normalized()), "Each simplex normal must match the curated normal of its source cell.");
	}
	for (int64_t cell_index = 0; cell_index < 16; cell_index++) {
		CHECK_MESSAGE(simplexes_per_cell[cell_index] == 1, "Each orthoplex cell should contribute exactly one tetrahedron.");
	}
}

TEST_CASE("[PolyMesh4D] Boundary pivot overrides are used by the simplex decomposition") {
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	Ref<ArrayPolyMesh4D> mesh = box->to_array_poly_mesh();
	// Vertex 7 is a corner of cell 0 (the -W cube). Forcing it as the pivot means every
	// tetrahedron of cell 0 must fan out from vertex 7.
	PackedInt32Array pivot_overrides;
	pivot_overrides.resize_initialized(8);
	for (int64_t i = 0; i < 8; i++) {
		pivot_overrides.set(i, -1);
	}
	pivot_overrides.set(0, 7);
	mesh->set_poly_cell_boundary_pivot_overrides(pivot_overrides);
	const PackedInt32Array simplex_indices = mesh->get_simplex_cell_indices();
	REQUIRE(simplex_indices.size() % 4 == 0);
	const int64_t simplex_count = simplex_indices.size() / 4;
	int64_t cell_0_simplex_count = 0;
	for (int64_t simplex_index = 0; simplex_index < simplex_count; simplex_index++) {
		if (mesh->get_source_poly_cell_for_simplex_cell(simplex_index) != 0) {
			continue;
		}
		cell_0_simplex_count++;
		bool has_pivot = false;
		for (int64_t i = 0; i < 4; i++) {
			if (simplex_indices[simplex_index * 4 + i] == 7) {
				has_pivot = true;
			}
		}
		CHECK_MESSAGE(has_pivot, "Every simplex of a cell with a pivot override must contain the override vertex.");
	}
	CHECK_MESSAGE(cell_0_simplex_count == 6, "A cube cell pivoting on one of its corners produces 6 tetrahedra.");
}

TEST_CASE("[PolyMesh4D] Poly cell vertex indices by dimension") {
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	SUBCASE("Dimension 0 returns each vertex by itself") {
		const Vector<PackedInt32Array> result = box->get_all_poly_cell_vertex_indices(0, false);
		REQUIRE(result.size() == 16);
		for (int32_t vertex_index = 0; vertex_index < 16; vertex_index++) {
			CHECK((result[vertex_index] == PackedInt32Array{ vertex_index }));
		}
	}
	SUBCASE("Dimension 1 returns each edge as a pair") {
		const Vector<PackedInt32Array> result = box->get_all_poly_cell_vertex_indices(1, false);
		const PackedInt32Array edge_indices = box->get_edge_indices();
		REQUIRE(result.size() == 32);
		for (int64_t edge_index = 0; edge_index < 32; edge_index++) {
			CHECK((result[edge_index] == PackedInt32Array{ edge_indices[edge_index * 2], edge_indices[edge_index * 2 + 1] }));
		}
	}
	SUBCASE("Dimension 2 returns the vertices of each face") {
		const Vector<PackedInt32Array> result = box->get_all_poly_cell_vertex_indices(2, false);
		REQUIRE(result.size() == 24);
		for (int64_t face_index = 0; face_index < 24; face_index++) {
			CHECK(result[face_index].size() == 4);
		}
	}
	SUBCASE("Dimension 3 returns the vertices of each boundary cell") {
		const Vector<PackedInt32Array> result = box->get_all_poly_cell_vertex_indices(3, false);
		REQUIRE(result.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			CHECK(result[cell_index].size() == 8);
		}
	}
	SUBCASE("Dimension 4 returns the vertices of the hyper-cell") {
		const Vector<PackedInt32Array> result = box->get_all_poly_cell_vertex_indices(4, false);
		REQUIRE(result.size() == 1);
		CHECK(result[0].size() == 16);
		const Vector<PackedInt32Array> with_span = box->get_all_poly_cell_vertex_indices(4, true);
		REQUIRE(with_span.size() == 1);
		CHECK(with_span[0].size() == 16);
	}
	SUBCASE("Dimension out of range fails gracefully") {
		ERR_PRINT_OFF;
		const Vector<PackedInt32Array> result = box->get_all_poly_cell_vertex_indices(5, false);
		ERR_PRINT_ON;
		CHECK(result.is_empty());
	}
}

TEST_CASE("[PolyMesh4D] Poly cell poly indices decompositions") {
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	SUBCASE("Decomposing a dimension into itself is the identity") {
		const Vector<PackedInt32Array> result = box->get_all_poly_cell_poly_indices(3, 3);
		REQUIRE(result.size() == 8);
		for (int32_t cell_index = 0; cell_index < 8; cell_index++) {
			CHECK((result[cell_index] == PackedInt32Array{ cell_index }));
		}
	}
	SUBCASE("Decomposing into the next dimension down returns the stored elements") {
		const Vector<PackedInt32Array> cells_to_faces = box->get_all_poly_cell_poly_indices(3, 2);
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = box->get_poly_cell_indices();
		REQUIRE(cells_to_faces.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			CHECK(cells_to_faces[cell_index] == poly_cell_indices[1][cell_index]);
		}
		const Vector<PackedInt32Array> edges_to_vertices = box->get_all_poly_cell_poly_indices(1, 0);
		const PackedInt32Array edge_indices = box->get_edge_indices();
		REQUIRE(edges_to_vertices.size() == 32);
		for (int64_t edge_index = 0; edge_index < 32; edge_index++) {
			CHECK((edges_to_vertices[edge_index] == PackedInt32Array{ edge_indices[edge_index * 2], edge_indices[edge_index * 2 + 1] }));
		}
	}
	SUBCASE("Decomposing cells into edges uses the general recursive case") {
		const Vector<PackedInt32Array> cells_to_edges = box->get_all_poly_cell_poly_indices(3, 1);
		REQUIRE(cells_to_edges.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			CHECK_MESSAGE(cells_to_edges[cell_index].size() == 12, "Each cube cell of a box has 12 edges.");
		}
		const Vector<PackedInt32Array> hyper_to_faces = box->get_all_poly_cell_poly_indices(4, 2);
		REQUIRE(hyper_to_faces.size() == 1);
		CHECK_MESSAGE(hyper_to_faces[0].size() == 24, "The hyper-cell of a box contains all 24 faces.");
	}
	SUBCASE("Decomposing into vertices matches the vertex indices function") {
		const Vector<PackedInt32Array> cells_to_vertices = box->get_all_poly_cell_poly_indices(3, 0);
		const Vector<PackedInt32Array> expected = box->get_all_poly_cell_vertex_indices(3, false);
		REQUIRE(cells_to_vertices.size() == expected.size());
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			CHECK(cells_to_vertices[cell_index] == expected[cell_index]);
		}
	}
	SUBCASE("Invalid dimension combinations fail gracefully") {
		ERR_PRINT_OFF;
		CHECK(box->get_all_poly_cell_poly_indices(2, 3).is_empty());
		CHECK(box->get_all_poly_cell_poly_indices(3, -1).is_empty());
		CHECK(box->get_all_poly_cell_poly_indices(5, 0).is_empty());
		ERR_PRINT_ON;
	}
}

TEST_CASE("[PolyMesh4D] To array poly mesh") {
	SUBCASE("Box converts with all data intact") {
		Ref<BoxPolyMesh4D> box;
		box.instantiate();
		Ref<ArrayPolyMesh4D> array_mesh = box->to_array_poly_mesh();
		REQUIRE(array_mesh.is_valid());
		CHECK(array_mesh->get_poly_cell_vertices() == box->get_poly_cell_vertices());
		CHECK(array_mesh->get_edge_indices() == box->get_edge_indices());
		CHECK(array_mesh->get_poly_cell_boundary_normals() == box->get_poly_cell_boundary_normals());
		const Vector<Vector<PackedInt32Array>> array_indices = array_mesh->get_poly_cell_indices();
		const Vector<Vector<PackedInt32Array>> box_indices = box->get_poly_cell_indices();
		REQUIRE(array_indices.size() == box_indices.size());
		for (int64_t dim_index = 0; dim_index < array_indices.size(); dim_index++) {
			CHECK(array_indices[dim_index] == box_indices[dim_index]);
		}
		const Vector<PackedVector4Array> array_vertex_normals = array_mesh->get_poly_cell_vertex_normals();
		const Vector<PackedVector4Array> box_vertex_normals = box->get_poly_cell_vertex_normals();
		REQUIRE(array_vertex_normals.size() == box_vertex_normals.size());
		for (int64_t cell_index = 0; cell_index < array_vertex_normals.size(); cell_index++) {
			CHECK(array_vertex_normals[cell_index] == box_vertex_normals[cell_index]);
		}
		const Vector<PackedVector3Array> array_texture_map = array_mesh->get_poly_cell_texture_map();
		const Vector<PackedVector3Array> box_texture_map = box->get_poly_cell_texture_map();
		REQUIRE(array_texture_map.size() == box_texture_map.size());
		for (int64_t cell_index = 0; cell_index < array_texture_map.size(); cell_index++) {
			CHECK(array_texture_map[cell_index] == box_texture_map[cell_index]);
		}
		CHECK_MESSAGE(array_mesh->is_poly_mesh_data_valid(), "The converted array mesh must be valid.");
		CHECK_MESSAGE(array_mesh->get_simplex_cell_indices() == box->get_simplex_cell_indices(), "The converted array mesh must decompose identically.");
	}

	SUBCASE("Orthoplex converts and stays valid") {
		Ref<OrthoplexPolyMesh4D> orthoplex;
		orthoplex.instantiate();
		Ref<ArrayPolyMesh4D> array_mesh = orthoplex->to_array_poly_mesh();
		REQUIRE(array_mesh.is_valid());
		CHECK(array_mesh->get_poly_cell_vertices() == orthoplex->get_poly_cell_vertices());
		CHECK(array_mesh->is_poly_mesh_data_valid());
	}
}

TEST_CASE("[PolyMesh4D] Cache clearing keeps results consistent") {
	Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
	const PackedInt32Array first_simplex_indices = mesh->get_simplex_cell_indices();
	const PackedVector4Array first_normals = mesh->get_simplex_cell_boundary_normals();
	const PackedVector4Array first_vertices = mesh->get_vertices();
	mesh->poly_mesh_clear_cache();
	CHECK_MESSAGE(mesh->get_simplex_cell_indices() == first_simplex_indices, "Recomputing after a cache clear must give the same simplexes.");
	CHECK_MESSAGE(mesh->get_vertices() == first_vertices, "Recomputing after a cache clear must give the same vertices.");
	mesh->poly_mesh_clear_cache(true);
	CHECK_MESSAGE(mesh->get_simplex_cell_boundary_normals() == first_normals, "Recomputing after a normals-only cache clear must give the same normals.");
	mesh->reset_poly_mesh_data_validation();
	CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Revalidation after a reset must succeed for unchanged valid data.");
}

TEST_CASE("[PolyMesh4D] Meshes without boundary cells") {
	SUBCASE("A mesh with only vertices and edges") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->append_edge_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0));
		mesh->append_edge_points(Vector4(1, 0, 0, 0), Vector4(1, 1, 0, 0));
		CHECK(mesh->is_poly_mesh_data_valid());
		CHECK_MESSAGE(mesh->get_vertices() == mesh->get_poly_cell_vertices(), "Without boundary cells, the simplex vertices are just the poly vertices.");
		CHECK_MESSAGE(mesh->get_simplex_cell_indices().is_empty(), "Without boundary cells, there are no simplexes.");
	}

	SUBCASE("A mesh with faces but no cells") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->append_vertex(Vector4(0, 0, 0, 0));
		mesh->append_vertex(Vector4(1, 0, 0, 0));
		mesh->append_vertex(Vector4(0, 1, 0, 0));
		mesh->append_edge_indices(0, 1);
		mesh->append_edge_indices(0, 2);
		mesh->append_edge_indices(1, 2);
		mesh->append_poly_cell(2, PackedInt32Array{ 0, 2, 1 }, false);
		CHECK(mesh->is_poly_mesh_data_valid());
		CHECK_MESSAGE(mesh->get_simplex_cell_indices().is_empty(), "A face-only mesh has no boundary cells to decompose.");
		const Vector<PackedInt32Array> face_vertex_indices = mesh->get_all_face_vertex_indices();
		REQUIRE(face_vertex_indices.size() == 1);
		CHECK(face_vertex_indices[0].size() == 3);
	}
}
} // namespace TestPolyMesh4D
