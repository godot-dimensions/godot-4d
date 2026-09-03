#pragma once

#include "../../../../math/vector_4d.h"
#include "../../../../model/mesh/poly/array_poly_mesh_4d.h"
#include "../../../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../mesh_attribute_test_helpers_4d.h"

#include "tests/test_macros.h"

namespace TestArrayPolyMesh4D {
using namespace TestMeshAttributes4D;

static HashMap<Vector2i, Vector<PackedVector4Array>> get_all_dense_normals(const Ref<ArrayPolyMesh4D> &p_mesh) {
	HashMap<Vector2i, Vector<PackedVector4Array>> result;
	for (const KeyValue<Vector2i, Vector<PackedInt32Array>> &binding : p_mesh->get_all_poly_cell_normal_indices()) {
		result.insert(binding.key, p_mesh->get_poly_cell_dense_normals(binding.key));
	}
	return result;
}

static HashMap<Vector2i, Vector<PackedVector3Array>> get_all_dense_texture_maps(const Ref<ArrayPolyMesh4D> &p_mesh) {
	HashMap<Vector2i, Vector<PackedVector3Array>> result;
	for (const KeyValue<Vector2i, Vector<PackedInt32Array>> &binding : p_mesh->get_all_poly_cell_texture_map_indices()) {
		result.insert(binding.key, p_mesh->get_poly_cell_dense_texture_map(binding.key));
	}
	return result;
}

static void set_all_dense_normals(const Ref<ArrayPolyMesh4D> &p_mesh, const HashMap<Vector2i, Vector<PackedVector4Array>> &p_bindings) {
	HashMap<Vector2i, Vector<PackedInt32Array>> bindings;
	PackedVector4Array values;
	for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &binding : p_bindings) {
		Vector<PackedInt32Array> arrays;
		for (const PackedVector4Array &cell : binding.value) {
			PackedInt32Array indices;
			for (const Vector4 &value : cell) {
				indices.append(Vector4D::vector4_array_append_deduplicate(values, value));
			}
			arrays.append(indices);
		}
		bindings.insert(binding.key, arrays);
	}
	p_mesh->set_poly_cell_normal_values(values);
	p_mesh->set_all_poly_cell_normal_indices(bindings);
}

static void set_all_dense_texture_maps(const Ref<ArrayPolyMesh4D> &p_mesh, const HashMap<Vector2i, Vector<PackedVector3Array>> &p_bindings) {
	HashMap<Vector2i, Vector<PackedInt32Array>> bindings;
	PackedVector3Array values;
	for (const KeyValue<Vector2i, Vector<PackedVector3Array>> &binding : p_bindings) {
		Vector<PackedInt32Array> arrays;
		for (const PackedVector3Array &cell : binding.value) {
			PackedInt32Array indices;
			for (const Vector3 &value : cell) {
				indices.append(Vector4D::vector3_array_append_deduplicate(values, value));
			}
			arrays.append(indices);
		}
		bindings.insert(binding.key, arrays);
	}
	p_mesh->set_poly_cell_texture_map_values(values);
	p_mesh->set_all_poly_cell_texture_map_indices(bindings);
}

// The faces of a tetrahedron with vertices 0, 1, 2, 3, encoded as lists of edge indices.
// Edges: 0:(0,1) 1:(0,2) 2:(0,3) 3:(1,2) 4:(1,3) 5:(2,3)
// Faces: 0: verts 0-1-2, 1: verts 0-1-3, 2: verts 0-2-3, 3: verts 1-2-3.
inline Ref<ArrayPolyMesh4D> make_tetrahedron_cell_mesh(const PackedInt32Array &p_cell_faces = PackedInt32Array{ 0, 1, 2, 3 }) {
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
	mesh->append_poly_cell(2, PackedInt32Array{ 0, 3, 1 }, false);
	mesh->append_poly_cell(2, PackedInt32Array{ 0, 4, 2 }, false);
	mesh->append_poly_cell(2, PackedInt32Array{ 1, 5, 2 }, false);
	mesh->append_poly_cell(2, PackedInt32Array{ 3, 5, 4 }, false);
	mesh->append_poly_cell(3, p_cell_faces, false);
	return mesh;
}

// Two tetrahedral cells sharing face 3 (verts 1-2-3), both flat in the w=0 hyperplane.
// Edges: 0:(0,1) 1:(0,2) 2:(0,3) 3:(1,2) 4:(1,3) 5:(2,3) 6:(1,4) 7:(2,4) 8:(3,4)
// Faces: 0: 0-1-2, 1: 0-1-3, 2: 0-2-3, 3: 1-2-3 (shared), 4: 1-2-4, 5: 1-3-4, 6: 2-3-4.
inline Ref<ArrayPolyMesh4D> make_two_tetrahedra_cells_mesh() {
	Ref<ArrayPolyMesh4D> mesh;
	mesh.instantiate();
	mesh->append_vertex(Vector4(0, 0, 0, 0));
	mesh->append_vertex(Vector4(1, 0, 0, 0));
	mesh->append_vertex(Vector4(0, 1, 0, 0));
	mesh->append_vertex(Vector4(0, 0, 1, 0));
	mesh->append_vertex(Vector4(1, 1, 1, 0));
	mesh->append_edge_indices(0, 1);
	mesh->append_edge_indices(0, 2);
	mesh->append_edge_indices(0, 3);
	mesh->append_edge_indices(1, 2);
	mesh->append_edge_indices(1, 3);
	mesh->append_edge_indices(2, 3);
	mesh->append_edge_indices(1, 4);
	mesh->append_edge_indices(2, 4);
	mesh->append_edge_indices(3, 4);
	mesh->append_poly_cell(2, PackedInt32Array{ 0, 3, 1 }, false);
	mesh->append_poly_cell(2, PackedInt32Array{ 0, 4, 2 }, false);
	mesh->append_poly_cell(2, PackedInt32Array{ 1, 5, 2 }, false);
	mesh->append_poly_cell(2, PackedInt32Array{ 3, 5, 4 }, false);
	mesh->append_poly_cell(2, PackedInt32Array{ 3, 7, 6 }, false);
	mesh->append_poly_cell(2, PackedInt32Array{ 4, 8, 6 }, false);
	mesh->append_poly_cell(2, PackedInt32Array{ 5, 8, 7 }, false);
	mesh->append_poly_cell(3, PackedInt32Array{ 0, 1, 2, 3 }, false);
	mesh->append_poly_cell(3, PackedInt32Array{ 3, 4, 5, 6 }, false);
	return mesh;
}

// A closed ring of four tetrahedral cells around the shared edge 0-1, all flat in the w=0
// hyperplane, with manually set boundary normals rotated by 30 degrees per cell from +W
// toward +X. With the default 45 degree seam threshold, only the face shared by the first
// and last cells (face 0, verts 0-1-2, at a 90 degree angle) becomes a seam, but the ring
// stays connected as one island through the other three shared faces.
// Vertices: 0:(0,0,0,0) 1:(1,0,0,0), ring: 2:(0,1,0,0) 3:(0,0,1,0) 4:(0,-1,0,0) 5:(0,0,-1,0)
// Edges: 0:(0,1), 1-4: vertex 0 to ring, 5-8: vertex 1 to ring, 9-12: ring loop.
// Faces: 0-3: shared faces {0,1,ring[i]}, 4-7: {0,ring[i],ring[i+1]}, 8-11: {1,ring[i],ring[i+1]}.
inline Ref<ArrayPolyMesh4D> make_tetrahedron_ring_mesh() {
	Ref<ArrayPolyMesh4D> mesh;
	mesh.instantiate();
	mesh->append_vertex(Vector4(0, 0, 0, 0));
	mesh->append_vertex(Vector4(1, 0, 0, 0));
	mesh->append_vertex(Vector4(0, 1, 0, 0));
	mesh->append_vertex(Vector4(0, 0, 1, 0));
	mesh->append_vertex(Vector4(0, -1, 0, 0));
	mesh->append_vertex(Vector4(0, 0, -1, 0));
	mesh->append_edge_indices(0, 1); // Edge 0.
	for (int32_t i = 0; i < 4; i++) {
		mesh->append_edge_indices(0, 2 + i); // Edges 1 to 4.
	}
	for (int32_t i = 0; i < 4; i++) {
		mesh->append_edge_indices(1, 2 + i); // Edges 5 to 8.
	}
	for (int32_t i = 0; i < 4; i++) {
		mesh->append_edge_indices(2 + i, 2 + (i + 1) % 4); // Edges 9 to 12.
	}
	for (int32_t i = 0; i < 4; i++) {
		mesh->append_poly_cell(2, PackedInt32Array{ 0, 1 + i, 5 + i }, false); // Faces 0 to 3.
	}
	for (int32_t i = 0; i < 4; i++) {
		mesh->append_poly_cell(2, PackedInt32Array{ 1 + i, 9 + i, 1 + (i + 1) % 4 }, false); // Faces 4 to 7.
	}
	for (int32_t i = 0; i < 4; i++) {
		mesh->append_poly_cell(2, PackedInt32Array{ 5 + i, 9 + i, 5 + (i + 1) % 4 }, false); // Faces 8 to 11.
	}
	for (int32_t i = 0; i < 4; i++) {
		mesh->append_poly_cell(3, PackedInt32Array{ i, (i + 1) % 4, 4 + i, 8 + i }, false);
	}
	PackedVector4Array normals;
	for (int64_t i = 0; i < 4; i++) {
		const double angle = i * (Math_PI / 6.0);
		normals.append(Vector4(Math::sin(angle), 0, 0, Math::cos(angle)));
	}
	mesh->set_poly_cell_boundary_normals(normals);
	return mesh;
}

inline Ref<ArrayPolyMesh4D> make_box_array_mesh() {
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	return box->to_array_poly_mesh();
}

TEST_CASE("[ArrayPolyMesh4D] Append vertices and edges") {
	SUBCASE("Appending vertices deduplicates by default") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		CHECK(mesh->append_vertex(Vector4(1, 2, 3, 4)) == 0);
		CHECK(mesh->append_vertex(Vector4(5, 6, 7, 8)) == 1);
		CHECK_MESSAGE(mesh->append_vertex(Vector4(1, 2, 3, 4)) == 0, "Appending a duplicate vertex should return the existing index.");
		CHECK(mesh->get_poly_cell_vertex_positions().size() == 2);
		CHECK_MESSAGE(mesh->append_vertex(Vector4(1, 2, 3, 4), false) == 2, "Appending without deduplication should append a new vertex.");
		CHECK(mesh->get_poly_cell_vertex_positions().size() == 3);
	}

	SUBCASE("Appending multiple vertices returns their indices") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		const PackedInt32Array indices = mesh->append_vertices(PackedVector4Array{ Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 0, 0, 0) });
		CHECK((indices == PackedInt32Array{ 0, 1, 0 }));
		CHECK(mesh->get_poly_cell_vertex_positions().size() == 2);
	}

	SUBCASE("Appending edges stores sorted vertex indices and deduplicates both orders") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->append_vertex(Vector4(0, 0, 0, 0));
		mesh->append_vertex(Vector4(1, 0, 0, 0));
		mesh->append_vertex(Vector4(0, 1, 0, 0));
		CHECK(mesh->append_edge_indices(1, 0) == 0);
		CHECK_MESSAGE((mesh->get_edge_indices() == PackedInt32Array{ 0, 1 }), "Edges should be stored with sorted vertex indices.");
		CHECK_MESSAGE(mesh->append_edge_indices(0, 1) == 0, "The same edge in either vertex order should deduplicate.");
		CHECK_MESSAGE(mesh->append_edge_indices(1, 0) == 0, "The same edge in either vertex order should deduplicate.");
		CHECK(mesh->append_edge_indices(1, 2) == 1);
		CHECK(mesh->get_edge_indices().size() == 4);
		CHECK_MESSAGE(mesh->append_edge_indices(0, 1, false) == 2, "Appending without deduplication should append a new edge.");
	}

	SUBCASE("Appending edge points deduplicates the points") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		CHECK(mesh->append_edge_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0)) == 0);
		CHECK(mesh->append_edge_points(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0)) == 1);
		CHECK_MESSAGE(mesh->get_poly_cell_vertex_positions().size() == 3, "Shared points between edges should be deduplicated.");
		CHECK_MESSAGE(mesh->append_edge_points(Vector4(0, 1, 0, 0), Vector4(1, 0, 0, 0)) == 1, "An existing edge given by points should deduplicate.");
	}
}

TEST_CASE("[ArrayPolyMesh4D] Append poly cells") {
	SUBCASE("Appending cells requires vertices and edges to exist") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		ERR_PRINT_OFF;
		CHECK(mesh->append_poly_cell(2, PackedInt32Array{ 0, 1, 2 }) == -1);
		ERR_PRINT_ON;
	}

	SUBCASE("Appending cells of dimension below 2 is an error") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		ERR_PRINT_OFF;
		CHECK(mesh->append_poly_cell(1, PackedInt32Array{ 0, 1 }) == -1);
		CHECK(mesh->append_poly_cell(0, PackedInt32Array{ 0 }) == -1);
		ERR_PRINT_ON;
	}

	SUBCASE("Appending cells must not skip dimensions") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		// The mesh has up to 3D cells, so 4D is allowed but 5D is not.
		ERR_PRINT_OFF;
		CHECK(mesh->append_poly_cell(5, PackedInt32Array{ 0, 0, 0, 0, 0, 0 }) == -1);
		ERR_PRINT_ON;
	}

	SUBCASE("Appending cells referencing non-existent elements is an error") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		ERR_PRINT_OFF;
		CHECK(mesh->append_poly_cell(2, PackedInt32Array{ 0, 99, 1 }) == -1);
		CHECK(mesh->append_poly_cell(3, PackedInt32Array{ 0, 1, 2, 99 }) == -1);
		ERR_PRINT_ON;
	}

	SUBCASE("Appending deduplicates cells made of the same elements in any order") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		CHECK_MESSAGE(mesh->append_poly_cell(2, PackedInt32Array{ 1, 0, 3 }) == 0, "A face with the same edges in a different order should deduplicate.");
		CHECK(mesh->get_poly_cell_indices()[0].size() == 4);
		CHECK_MESSAGE(mesh->append_poly_cell(3, PackedInt32Array{ 3, 2, 1, 0 }) == 0, "A cell with the same faces in a different order should deduplicate.");
		CHECK(mesh->get_poly_cell_indices()[1].size() == 1);
		CHECK_MESSAGE(mesh->append_poly_cell(2, PackedInt32Array{ 1, 0, 3 }, false) == 4, "Appending without deduplication should append a new face.");
	}

	SUBCASE("Appending a cell of a new dimension creates that dimension") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		CHECK(mesh->get_poly_cell_indices().size() == 2);
		CHECK_MESSAGE(mesh->append_poly_cell(4, PackedInt32Array{ 0, 0, 0, 0, 0 }, false) == 0, "The first cell of a new dimension should be at index 0.");
		CHECK(mesh->get_poly_cell_indices().size() == 3);
	}
}

TEST_CASE("[ArrayPolyMesh4D] Delete poly elements") {
	SUBCASE("Deleting a vertex cascades to edges, faces, and cells") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		mesh->delete_poly_element(0, 3);
		CHECK(mesh->get_poly_cell_vertex_positions().size() == 3);
		CHECK_MESSAGE((mesh->get_edge_indices() == PackedInt32Array{ 0, 1, 0, 2, 1, 2 }), "Edges referencing the deleted vertex should be deleted, and the rest reindexed.");
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		REQUIRE_MESSAGE(poly_cell_indices.size() == 1, "The cell dimension should be trimmed once it becomes empty.");
		REQUIRE(poly_cell_indices[0].size() == 1);
		CHECK_MESSAGE((poly_cell_indices[0][0] == PackedInt32Array{ 0, 2, 1 }), "The surviving face should have its edge references reindexed.");
		CHECK(mesh->is_poly_mesh_data_valid());
	}

	SUBCASE("Deleting an edge cascades to faces and cells") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		mesh->delete_poly_element(1, 0);
		CHECK(mesh->get_poly_cell_vertex_positions().size() == 4);
		CHECK((mesh->get_edge_indices() == PackedInt32Array{ 0, 2, 0, 3, 1, 2, 1, 3, 2, 3 }));
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		REQUIRE(poly_cell_indices.size() == 1);
		REQUIRE_MESSAGE(poly_cell_indices[0].size() == 2, "The two faces using the deleted edge should be deleted.");
		CHECK((poly_cell_indices[0][0] == PackedInt32Array{ 0, 4, 1 }));
		CHECK((poly_cell_indices[0][1] == PackedInt32Array{ 2, 4, 3 }));
		CHECK(mesh->is_poly_mesh_data_valid());
	}

	SUBCASE("Deleting a face cascades to cells") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		mesh->delete_poly_element(2, 0);
		CHECK(mesh->get_edge_indices().size() == 12);
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		REQUIRE(poly_cell_indices.size() == 1);
		CHECK(poly_cell_indices[0].size() == 3);
		CHECK(mesh->is_poly_mesh_data_valid());
	}

	SUBCASE("Deleting a boundary cell from a box updates normals and texture maps") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		const PackedVector4Array original_normals = mesh->get_poly_cell_boundary_normals();
		mesh->delete_poly_element(3, 0);
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		REQUIRE_MESSAGE(poly_cell_indices.size() == 2, "The hyper-cell referencing the deleted cell should be deleted, trimming the 4D dimension.");
		CHECK(poly_cell_indices[0].size() == 24);
		CHECK(poly_cell_indices[1].size() == 7);
		const PackedVector4Array adjusted_normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE_MESSAGE(adjusted_normals.size() == 7, "Boundary normals should shrink with the deleted cell.");
		for (int64_t cell_index = 0; cell_index < 7; cell_index++) {
			CHECK(adjusted_normals[cell_index] == original_normals[cell_index + 1]);
		}
		CHECK(mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY).size() == 7);
		CHECK(mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY).size() == 7);
		CHECK(mesh->is_poly_mesh_data_valid());
	}

	SUBCASE("Deleting with invalid arguments fails gracefully") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		ERR_PRINT_OFF;
		mesh->delete_poly_element(-1, 0);
		mesh->delete_poly_element(4, 0);
		mesh->delete_poly_element(0, 99);
		mesh->delete_poly_element(1, 99);
		mesh->delete_poly_element(2, 99);
		ERR_PRINT_ON;
		CHECK(mesh->get_poly_cell_vertex_positions().size() == 4);
		CHECK(mesh->get_edge_indices().size() == 12);
		CHECK(mesh->get_poly_cell_indices().size() == 2);
		CHECK(mesh->is_poly_mesh_data_valid());
	}
}

TEST_CASE("[ArrayPolyMesh4D] Boundary normals for all cell face permutations") {
	// The orientation of a 3D cell is controlled by the order of its first two faces.
	// The test tetrahedron is flat in the w=0 hyperplane, so every orientation-derived
	// normal must be exactly +W or -W. This exhaustively tests all 24 face permutations.
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
				mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
				const PackedVector4Array normals = mesh->get_poly_cell_boundary_normals();
				REQUIRE(normals.size() == 1);
				const Vector4 normal = normals[0];
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
	// Verify the anchor orientations, computed by hand from the canonical spans.
	CHECK_MESSAGE(pair_normals[0][1].is_equal_approx(Vector4(0, 0, 0, 1)), "The face order {0, 1, 2, 3} must produce a +W normal.");
	CHECK_MESSAGE(pair_normals[2][3].is_equal_approx(Vector4(0, 0, 0, 1)), "The face order {2, 3, 0, 1} must produce a +W normal.");
	// Verify that swapping the first two faces flips the normal, for every possible pair.
	for (int32_t first = 0; first < 4; first++) {
		for (int32_t second = first + 1; second < 4; second++) {
			CHECK_MESSAGE(pair_normals[first][second].is_equal_approx(-pair_normals[second][first]), "Swapping the first two faces of a cell must flip its normal vector.");
		}
	}
}

TEST_CASE("[ArrayPolyMesh4D] Boundary normals match the curated box and orthoplex data") {
	SUBCASE("Box cell orientations encode the curated outward normals") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		const PackedVector4Array curated_normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE(curated_normals.size() == 8);
		mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
		const PackedVector4Array computed_normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE(computed_normals.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			CHECK_MESSAGE(computed_normals[cell_index].is_equal_approx(curated_normals[cell_index]), "The curated box cell orientations must reproduce the curated normals.");
		}
	}

	SUBCASE("Swapping the first two faces of one box cell flips only that cell's normal") {
		for (int64_t swap_cell = 0; swap_cell < 8; swap_cell++) {
			Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
			const PackedVector4Array curated_normals = mesh->get_poly_cell_boundary_normals();
			Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
			Vector<PackedInt32Array> cells = poly_cell_indices[1];
			PackedInt32Array cell = cells[swap_cell];
			const int32_t temp = cell[0];
			cell.set(0, cell[1]);
			cell.set(1, temp);
			cells.set(swap_cell, cell);
			poly_cell_indices.set(1, cells);
			mesh->set_poly_cell_indices(poly_cell_indices);
			mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
			const PackedVector4Array computed_normals = mesh->get_poly_cell_boundary_normals();
			REQUIRE(computed_normals.size() == 8);
			for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
				if (cell_index == swap_cell) {
					CHECK_MESSAGE(computed_normals[cell_index].is_equal_approx(-curated_normals[cell_index]), "The cell with swapped faces must have a flipped normal.");
				} else {
					CHECK_MESSAGE(computed_normals[cell_index].is_equal_approx(curated_normals[cell_index]), "Cells with unchanged faces must have unchanged normals.");
				}
			}
		}
	}
}

TEST_CASE("[ArrayPolyMesh4D] Force outward normal modes") {
	// Flip the orientation of two cells so their orientation-derived normals point inward.
	auto make_partially_inverted_box = []() -> Ref<ArrayPolyMesh4D> {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		Vector<PackedInt32Array> cells = poly_cell_indices[1];
		for (const int64_t swap_cell : { (int64_t)2, (int64_t)5 }) {
			PackedInt32Array cell = cells[swap_cell];
			const int32_t temp = cell[0];
			cell.set(0, cell[1]);
			cell.set(1, temp);
			cells.set(swap_cell, cell);
		}
		poly_cell_indices.set(1, cells);
		mesh->set_poly_cell_indices(poly_cell_indices);
		return mesh;
	};
	Ref<BoxPolyMesh4D> box;
	box.instantiate();
	const PackedVector4Array outward_normals = box->get_poly_cell_boundary_normals();

	SUBCASE("Cell orientation only preserves inward normals") {
		Ref<ArrayPolyMesh4D> mesh = make_partially_inverted_box();
		mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
		const PackedVector4Array normals = mesh->get_poly_cell_boundary_normals();
		CHECK(normals[2].is_equal_approx(-outward_normals[2]));
		CHECK(normals[5].is_equal_approx(-outward_normals[5]));
		CHECK(normals[0].is_equal_approx(outward_normals[0]));
	}

	SUBCASE("Force outward override flips normals but not cell orientation") {
		Ref<ArrayPolyMesh4D> mesh = make_partially_inverted_box();
		mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_FORCE_OUTWARD_OVERRIDE_CELL_ORIENTATION);
		const PackedVector4Array normals = mesh->get_poly_cell_boundary_normals();
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			CHECK_MESSAGE(normals[cell_index].is_equal_approx(outward_normals[cell_index]), "All normals must point outward.");
		}
		// The cell orientation was not fixed, so recomputing from orientation is inward again.
		mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
		const PackedVector4Array orientation_normals = mesh->get_poly_cell_boundary_normals();
		CHECK_MESSAGE(orientation_normals[2].is_equal_approx(-outward_normals[2]), "Override mode must not alter the cell data.");
		CHECK_MESSAGE(orientation_normals[5].is_equal_approx(-outward_normals[5]), "Override mode must not alter the cell data.");
	}

	SUBCASE("Force outward fix flips normals and fixes cell orientation") {
		Ref<ArrayPolyMesh4D> mesh = make_partially_inverted_box();
		mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_FORCE_OUTWARD_FIX_CELL_ORIENTATION);
		const PackedVector4Array normals = mesh->get_poly_cell_boundary_normals();
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			CHECK_MESSAGE(normals[cell_index].is_equal_approx(outward_normals[cell_index]), "All normals must point outward.");
		}
		// The cell orientation was fixed by swapping faces, so recomputing from orientation stays outward.
		mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
		const PackedVector4Array orientation_normals = mesh->get_poly_cell_boundary_normals();
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			CHECK_MESSAGE(orientation_normals[cell_index].is_equal_approx(outward_normals[cell_index]), "Fix mode must repair the cell orientation.");
		}
	}

	SUBCASE("Keep existing preserves non-zero normals") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		// Deliberately set a normal opposite to what the cell orientation would produce.
		mesh->set_poly_cell_boundary_normals(PackedVector4Array{ Vector4(0, 0, 0, -1) });
		mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, true);
		CHECK_MESSAGE((mesh->get_poly_cell_boundary_normals() == PackedVector4Array{ Vector4(0, 0, 0, -1) }), "Existing non-zero normals must be kept.");
		mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, false);
		CHECK_MESSAGE(mesh->get_poly_cell_boundary_normals()[0].is_equal_approx(Vector4(0, 0, 0, 1)), "Without keep existing, normals must be recomputed from orientation.");
	}

	SUBCASE("Calculating normals without cells fails gracefully") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->append_edge_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0));
		ERR_PRINT_OFF;
		mesh->calculate_boundary_normals();
		ERR_PRINT_ON;
		CHECK(mesh->get_poly_cell_boundary_normals().is_empty());
	}
}

TEST_CASE("[ArrayPolyMesh4D] Flat and smooth shading normals") {
	SUBCASE("Flat shading gives every vertex instance its cell's normal") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->set_flat_shading_normals();
		const PackedVector4Array boundary_normals = mesh->get_poly_cell_boundary_normals();
		const Vector<PackedVector4Array> vertex_normals = mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(vertex_normals.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			REQUIRE(vertex_normals[cell_index].size() == 8);
			for (int64_t vertex_in_cell = 0; vertex_in_cell < 8; vertex_in_cell++) {
				CHECK(vertex_normals[cell_index][vertex_in_cell].is_equal_approx(boundary_normals[cell_index]));
			}
		}
	}

	SUBCASE("Smooth shading on a box gives corner-diagonal normals") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->set_smooth_shading_normals();
		const PackedVector4Array vertices = mesh->get_poly_cell_vertex_positions();
		const Vector<PackedInt32Array> cell_vertex_indices = mesh->get_all_boundary_cell_vertex_indices(false);
		const Vector<PackedVector4Array> vertex_normals = mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(vertex_normals.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			REQUIRE(vertex_normals[cell_index].size() == cell_vertex_indices[cell_index].size());
			for (int64_t vertex_in_cell = 0; vertex_in_cell < cell_vertex_indices[cell_index].size(); vertex_in_cell++) {
				// Each box vertex is used by 4 cells whose normals are the 4 signed axes
				// matching the vertex's coordinate signs, so the average is the corner diagonal.
				const Vector4 expected = vertices[cell_vertex_indices[cell_index][vertex_in_cell]].normalized();
				CHECK_MESSAGE(vertex_normals[cell_index][vertex_in_cell].is_equal_approx(expected), "Smooth box normals must point along the corner diagonals.");
			}
		}
	}

	SUBCASE("Flat shading recovers from an empty normals binding") {
		// An empty binding for the boundary normals key can be set through the all-normals
		// map setter, and must be treated the same as having no normals at all.
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		HashMap<Vector2i, Vector<PackedInt32Array>> degenerate_normals;
		degenerate_normals.insert(PolyMesh4D::PER_CELL_KEY, Vector<PackedInt32Array>());
		mesh->set_all_poly_cell_normal_indices(degenerate_normals);
		mesh->set_flat_shading_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, false);
		const Vector<PackedVector4Array> vertex_normals = mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(vertex_normals.size() == 1);
		REQUIRE(vertex_normals[0].size() == 4);
		CHECK(vertex_normals[0][0].is_equal_approx(Vector4(0, 0, 0, 1)));
	}

	SUBCASE("Smooth shading respects seams as sharp borders") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		// All 24 box faces are seams at the default threshold, isolating each cell,
		// which makes smooth shading equivalent to flat shading.
		mesh->calculate_seam_faces();
		mesh->set_smooth_shading_normals();
		const PackedVector4Array boundary_normals = mesh->get_poly_cell_boundary_normals();
		const Vector<PackedVector4Array> vertex_normals = mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(vertex_normals.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			for (int64_t vertex_in_cell = 0; vertex_in_cell < vertex_normals[cell_index].size(); vertex_in_cell++) {
				CHECK_MESSAGE(vertex_normals[cell_index][vertex_in_cell].is_equal_approx(boundary_normals[cell_index]), "With every face a seam, smooth shading must match flat shading.");
			}
		}
	}
}

TEST_CASE("[ArrayPolyMesh4D] Make double sided") {
	SUBCASE("Doubling a single cell adds a flipped copy of everything") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Vector<PackedVector4Array> vertex_normals;
		vertex_normals.push_back(PackedVector4Array{ Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1) });
		mesh->set_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY, vertex_normals);
		Vector<PackedVector3Array> texture_map;
		texture_map.push_back(PackedVector3Array{ Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) });
		mesh->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, texture_map);
		mesh->make_double_sided();
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		REQUIRE(poly_cell_indices[1].size() == 2);
		CHECK_MESSAGE((poly_cell_indices[1][1] == PackedInt32Array{ 1, 0, 2, 3 }), "The flipped cell must have its first two faces swapped.");
		const PackedVector4Array boundary_normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE(boundary_normals.size() == 2);
		CHECK(boundary_normals[1].is_equal_approx(-boundary_normals[0]));
		const Vector<PackedVector4Array> doubled_vertex_normals = mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(doubled_vertex_normals.size() == 2);
		for (int64_t vertex_in_cell = 0; vertex_in_cell < 4; vertex_in_cell++) {
			CHECK_MESSAGE(doubled_vertex_normals[1][vertex_in_cell].is_equal_approx(Vector4(0, 0, 0, -1)), "The flipped cell's vertex normals must be negated.");
		}
		const Vector<PackedVector3Array> doubled_texture_map = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(doubled_texture_map.size() == 2);
		const PackedVector3Array expected_flipped_texture_map = { texture_map[0][0], texture_map[0][1], texture_map[0][3], texture_map[0][2] };
		CHECK_MESSAGE(doubled_texture_map[1] == expected_flipped_texture_map, "The flipped cell must preserve texture coordinates by vertex identity.");
		CHECK(mesh->is_poly_mesh_data_valid());
	}

	SUBCASE("Idempotence") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		mesh->make_double_sided(true);
		CHECK(mesh->get_poly_cell_indices()[1].size() == 2);
		mesh->make_double_sided(true);
		CHECK_MESSAGE(mesh->get_poly_cell_indices()[1].size() == 2, "Doubling twice with idempotence must not add more cells.");
		mesh->make_double_sided(false);
		CHECK_MESSAGE(mesh->get_poly_cell_indices()[1].size() == 4, "Doubling without idempotence must add more cells.");
	}

	SUBCASE("Doubling a box also extends the volumetric cells") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->make_double_sided();
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		CHECK(poly_cell_indices[1].size() == 16);
		REQUIRE(poly_cell_indices.size() == 3);
		CHECK_MESSAGE(poly_cell_indices[2][0].size() == 16, "The hyper-cell must gain the flipped cells.");
		CHECK(mesh->get_poly_cell_boundary_normals().size() == 16);
		CHECK(mesh->is_poly_mesh_data_valid());
	}
}

TEST_CASE("[ArrayPolyMesh4D] Single cell and single volume helpers") {
	SUBCASE("Single cell from all faces of a tetrahedron") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		const PackedInt32Array cell = mesh->make_single_cell_from_all_faces();
		CHECK_MESSAGE((cell == PackedInt32Array{ 0, 1, 2, 3 }), "Faces 0 and 1 already share an edge, so the order should be unchanged.");
	}

	SUBCASE("Single volume from all cells of a box") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		const PackedInt32Array volume = mesh->make_single_volume_from_all_cells();
		CHECK_MESSAGE((volume == PackedInt32Array{ 0, 1, 2, 3, 4, 5, 6, 7 }), "Cells 0 and 1 already share a face, so the order should be unchanged.");
	}

	SUBCASE("Single cell from disconnected faces fails gracefully") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->append_vertex(Vector4(0, 0, 0, 0));
		mesh->append_vertex(Vector4(1, 0, 0, 0));
		mesh->append_vertex(Vector4(0, 1, 0, 0));
		mesh->append_vertex(Vector4(5, 0, 0, 0));
		mesh->append_vertex(Vector4(6, 0, 0, 0));
		mesh->append_vertex(Vector4(5, 1, 0, 0));
		mesh->append_edge_indices(0, 1);
		mesh->append_edge_indices(0, 2);
		mesh->append_edge_indices(1, 2);
		mesh->append_edge_indices(3, 4);
		mesh->append_edge_indices(3, 5);
		mesh->append_edge_indices(4, 5);
		mesh->append_poly_cell(2, PackedInt32Array{ 0, 2, 1 }, false);
		mesh->append_poly_cell(2, PackedInt32Array{ 3, 5, 4 }, false);
		ERR_PRINT_OFF;
		const PackedInt32Array cell = mesh->make_single_cell_from_all_faces();
		ERR_PRINT_ON;
		CHECK_MESSAGE(cell.size() == 2, "The face list is still returned even when no shared edge exists.");
	}
}

TEST_CASE("[ArrayPolyMesh4D] Seam faces") {
	SUBCASE("All box faces are seams at the default threshold") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->calculate_seam_faces();
		const PackedInt32Array seams = mesh->get_seam_face_indices_bind();
		REQUIRE_MESSAGE(seams.size() == 24, "Every box face borders two perpendicular cells, above the default threshold.");
		for (int32_t face_index = 0; face_index < 24; face_index++) {
			CHECK_MESSAGE(seams[face_index] == face_index, "The seam face indices should be sorted.");
		}
	}

	SUBCASE("No box faces are seams at a high threshold") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->calculate_seam_faces(2.0);
		CHECK(mesh->get_seam_face_indices_bind().is_empty());
	}

	SUBCASE("Coplanar cells with matching normals produce no seams") {
		Ref<ArrayPolyMesh4D> mesh = make_two_tetrahedra_cells_mesh();
		mesh->set_poly_cell_boundary_normals(PackedVector4Array{ Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1) });
		mesh->calculate_seam_faces();
		CHECK(mesh->get_seam_face_indices_bind().is_empty());
	}

	SUBCASE("Adjacent cells with opposite normals produce a seam on the shared face") {
		Ref<ArrayPolyMesh4D> mesh = make_two_tetrahedra_cells_mesh();
		mesh->set_poly_cell_boundary_normals(PackedVector4Array{ Vector4(0, 0, 0, 1), Vector4(0, 0, 0, -1) });
		mesh->calculate_seam_faces();
		CHECK((mesh->get_seam_face_indices_bind() == PackedInt32Array{ 3 }));
	}

	SUBCASE("Discarding seams within islands removes seams that do not separate cells") {
		// The ring's sharp joint (face 0) is a seam by angle, but the ring stays connected
		// as one island through its other three joints, so the seam separates nothing.
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_ring_mesh();
		mesh->calculate_seam_faces(Math_TAU / 8.0, false);
		CHECK_MESSAGE((mesh->get_seam_face_indices_bind() == PackedInt32Array{ 0 }), "Only the face between the first and last ring cells is above the angle threshold.");
		CHECK_MESSAGE(mesh->collect_all_islands().size() == 1, "The ring stays connected as one island through the other three shared faces.");
		mesh->calculate_seam_faces(Math_TAU / 8.0, true);
		CHECK_MESSAGE(mesh->get_seam_face_indices_bind().is_empty(), "A seam between two cells of the same island must be discarded.");
	}

	SUBCASE("Discarding seams within islands works for islands with offset cell indices") {
		// Two disconnected rings: the second island's cell indices (4-7) differ from their
		// positions within the island (0-3), which is a regression test for the discard loop
		// confusing cell indices with positions in the island.
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_ring_mesh();
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_ring_mesh();
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		mesh->calculate_seam_faces(Math_TAU / 8.0, false);
		CHECK_MESSAGE((mesh->get_seam_face_indices_bind() == PackedInt32Array{ 0, 12 }), "Each ring has one seam face above the angle threshold.");
		CHECK(mesh->collect_all_islands().size() == 2);
		mesh->calculate_seam_faces(Math_TAU / 8.0, true);
		CHECK_MESSAGE(mesh->get_seam_face_indices_bind().is_empty(), "Seams within islands must be discarded in every island, not only the first.");
	}

	SUBCASE("Seam setters and getters round trip") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->set_seam_face_indices_bind(PackedInt32Array{ 9, 3, 5 });
		CHECK_MESSAGE((mesh->get_seam_face_indices_bind() == PackedInt32Array{ 3, 5, 9 }), "Seam face indices should be returned sorted.");
		CHECK(mesh->get_seam_face_indices().size() == 3);
	}
}

TEST_CASE("[ArrayPolyMesh4D] Islands") {
	SUBCASE("A box with no seams is one island") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		const Vector<PackedInt32Array> islands = mesh->collect_all_islands();
		REQUIRE(islands.size() == 1);
		CHECK(islands[0].size() == 8);
		const PackedInt32Array island = mesh->collect_cells_in_island(3);
		CHECK(island.size() == 8);
		CHECK(island.has(3));
	}

	SUBCASE("A box with all faces as seams is eight islands") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->calculate_seam_faces();
		const Vector<PackedInt32Array> islands = mesh->collect_all_islands();
		REQUIRE(islands.size() == 8);
		for (int64_t island_index = 0; island_index < 8; island_index++) {
			CHECK(islands[island_index].size() == 1);
		}
	}

	SUBCASE("Two connected cells are one island, split by a seam on the shared face") {
		Ref<ArrayPolyMesh4D> mesh = make_two_tetrahedra_cells_mesh();
		CHECK(mesh->collect_all_islands().size() == 1);
		mesh->set_seam_face_indices_bind(PackedInt32Array{ 3 });
		CHECK(mesh->collect_all_islands().size() == 2);
	}

	SUBCASE("Island collection with invalid arguments fails gracefully") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		ERR_PRINT_OFF;
		CHECK(mesh->collect_cells_in_island(99).is_empty());
		ERR_PRINT_ON;
	}
}

TEST_CASE("[ArrayPolyMesh4D] Unwrap texture map") {
	SUBCASE("Tile cells mode gives each box cell a half-size tile") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->unwrap_texture_map(ArrayPolyMesh4D::UNWRAP_MODE_TILE_CELLS);
		const Vector<PackedVector3Array> texture_map = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(texture_map.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			REQUIRE(texture_map[cell_index].size() == 8);
			AABB cell_aabb = AABB(texture_map[cell_index][0], Vector3());
			for (int64_t vertex_in_cell = 0; vertex_in_cell < 8; vertex_in_cell++) {
				const Vector3 texcoord = texture_map[cell_index][vertex_in_cell];
				CHECK(texcoord.x >= (real_t)-0.001);
				CHECK(texcoord.y >= (real_t)-0.001);
				CHECK(texcoord.z >= (real_t)-0.001);
				CHECK(texcoord.x <= (real_t)1.001);
				CHECK(texcoord.y <= (real_t)1.001);
				CHECK(texcoord.z <= (real_t)1.001);
				cell_aabb.expand_to(texcoord);
			}
			// 8 islands tile as a 2x2x2 grid, and an unwrapped cube fills its half-size tile.
			CHECK_MESSAGE(cell_aabb.size.is_equal_approx(Vector3(0.5, 0.5, 0.5)), "Each cube cell must fill a half-size tile.");
		}
	}

	SUBCASE("Each cell fills mode maps each box cell to the full unit cube") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->unwrap_texture_map(ArrayPolyMesh4D::UNWRAP_MODE_EACH_CELL_FILLS);
		const Vector<PackedVector3Array> texture_map = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(texture_map.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			AABB cell_aabb = AABB(texture_map[cell_index][0], Vector3());
			for (int64_t vertex_in_cell = 0; vertex_in_cell < 8; vertex_in_cell++) {
				cell_aabb.expand_to(texture_map[cell_index][vertex_in_cell]);
			}
			CHECK(cell_aabb.position.is_equal_approx(Vector3(0, 0, 0)));
			CHECK(cell_aabb.size.is_equal_approx(Vector3(1, 1, 1)));
		}
	}

	SUBCASE("Padding insets the unwrapped cells") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->unwrap_texture_map(ArrayPolyMesh4D::UNWRAP_MODE_EACH_CELL_FILLS, 1.0);
		const Vector<PackedVector3Array> texture_map = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(texture_map.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			AABB cell_aabb = AABB(texture_map[cell_index][0], Vector3());
			for (int64_t vertex_in_cell = 0; vertex_in_cell < 8; vertex_in_cell++) {
				cell_aabb.expand_to(texture_map[cell_index][vertex_in_cell]);
			}
			// A padding of 1.0 means half the space is padding: 0.25 on each side.
			CHECK(cell_aabb.position.is_equal_approx(Vector3(0.25, 0.25, 0.25)));
			CHECK(cell_aabb.size.is_equal_approx(Vector3(0.5, 0.5, 0.5)));
		}
	}

	SUBCASE("Tile islands mode with no seams unwraps the box as one island") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->unwrap_texture_map(ArrayPolyMesh4D::UNWRAP_MODE_TILE_ISLANDS);
		const Vector<PackedVector3Array> texture_map = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(texture_map.size() == 8);
		for (int64_t cell_index = 0; cell_index < 8; cell_index++) {
			REQUIRE(texture_map[cell_index].size() == 8);
			for (int64_t vertex_in_cell = 0; vertex_in_cell < 8; vertex_in_cell++) {
				const Vector3 texcoord = texture_map[cell_index][vertex_in_cell];
				CHECK(texcoord.x >= (real_t)-0.001);
				CHECK(texcoord.y >= (real_t)-0.001);
				CHECK(texcoord.z >= (real_t)-0.001);
				CHECK(texcoord.x <= (real_t)1.001);
				CHECK(texcoord.y <= (real_t)1.001);
				CHECK(texcoord.z <= (real_t)1.001);
			}
		}
	}

	SUBCASE("Unwrapping a single island only fills that island") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		mesh->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, Vector<PackedVector3Array>());
		mesh->unwrap_texture_map_island(PackedInt32Array{ 0 });
		const Vector<PackedVector3Array> texture_map = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(texture_map.size() == 8);
		CHECK(texture_map[0].size() == 8);
		for (int64_t cell_index = 1; cell_index < 8; cell_index++) {
			CHECK_MESSAGE(texture_map[cell_index].is_empty(), "Cells outside the island must not be mapped.");
		}
	}
}

TEST_CASE("[ArrayPolyMesh4D] Transform texture map and vertices") {
	SUBCASE("Transforming the texture map offsets all coordinates") {
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		const Vector<PackedVector3Array> original = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		mesh->transform_texture_map(Transform3D(Basis(), Vector3(10, 20, 30)));
		const Vector<PackedVector3Array> transformed = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(transformed.size() == original.size());
		for (int64_t cell_index = 0; cell_index < original.size(); cell_index++) {
			REQUIRE(transformed[cell_index].size() == original[cell_index].size());
			for (int64_t vertex_in_cell = 0; vertex_in_cell < original[cell_index].size(); vertex_in_cell++) {
				CHECK(transformed[cell_index][vertex_in_cell].is_equal_approx(original[cell_index][vertex_in_cell] + Vector3(10, 20, 30)));
			}
		}
	}

	SUBCASE("Transforming vertices applies the basis and offset") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		mesh->transform_vertices(Transform4D(Basis4D::from_scale_uniform(2.0), Vector4(1, 2, 3, 4)));
		const PackedVector4Array vertices = mesh->get_poly_cell_vertex_positions();
		REQUIRE(vertices.size() == 4);
		CHECK(vertices[0].is_equal_approx(Vector4(1, 2, 3, 4)));
		CHECK(vertices[1].is_equal_approx(Vector4(3, 2, 3, 4)));
		CHECK(vertices[2].is_equal_approx(Vector4(1, 4, 3, 4)));
		CHECK(vertices[3].is_equal_approx(Vector4(1, 2, 5, 4)));
		CHECK(mesh->is_poly_mesh_data_valid());
	}
}

TEST_CASE("[ArrayPolyMesh4D] Merge meshes") {
	SUBCASE("Merging two tetrahedra with an offset adjusts all indices") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_cell_mesh();
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		const PackedVector4Array vertices = mesh->get_poly_cell_vertex_positions();
		REQUIRE(vertices.size() == 8);
		CHECK(vertices[4].is_equal_approx(Vector4(10, 0, 0, 0)));
		CHECK(vertices[5].is_equal_approx(Vector4(11, 0, 0, 0)));
		const PackedInt32Array edge_indices = mesh->get_edge_indices();
		REQUIRE(edge_indices.size() == 24);
		CHECK_MESSAGE(edge_indices[12] == 4, "The merged edges must reference the offset vertices.");
		CHECK(edge_indices[13] == 5);
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		REQUIRE(poly_cell_indices.size() == 2);
		REQUIRE(poly_cell_indices[0].size() == 8);
		CHECK_MESSAGE((poly_cell_indices[0][4] == PackedInt32Array{ 6, 9, 7 }), "The merged faces must reference the offset edges.");
		REQUIRE(poly_cell_indices[1].size() == 2);
		CHECK_MESSAGE((poly_cell_indices[1][1] == PackedInt32Array{ 4, 5, 6, 7 }), "The merged cells must reference the offset faces.");
		CHECK(mesh->is_poly_mesh_data_valid());
		CHECK_MESSAGE(mesh->collect_all_islands().size() == 2, "Two disconnected tetrahedra form two islands.");
	}

	SUBCASE("Merging combines boundary normals") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_cell_mesh();
		mesh->calculate_boundary_normals();
		other->calculate_boundary_normals();
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		const PackedVector4Array normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE(normals.size() == 2);
		CHECK(normals[0].is_equal_approx(Vector4(0, 0, 0, 1)));
		CHECK(normals[1].is_equal_approx(Vector4(0, 0, 0, 1)));
	}

	SUBCASE("Merging generates missing boundary normals for the original mesh") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_cell_mesh();
		other->calculate_boundary_normals();
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		const PackedVector4Array normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE_MESSAGE(normals.size() == 2, "The original mesh's missing normals must be generated during the merge.");
		CHECK_MESSAGE(normals[0].is_equal_approx(Vector4(0, 0, 0, 1)), "The generated normal must come from the cell orientation.");
		CHECK(normals[1].is_equal_approx(Vector4(0, 0, 0, 1)));
		CHECK(mesh->is_poly_mesh_data_valid());
	}

	SUBCASE("Merging transforms the other mesh's normals by the basis") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_cell_mesh();
		mesh->calculate_boundary_normals();
		other->calculate_boundary_normals();
		// Negating all four axes is a rotation in 4D, so the mesh stays valid.
		// Note: Basis4D::from_scale_uniform(-1.0) would only flip the W axis, so build this by hand.
		const Basis4D negate_all = Basis4D(Vector4(-1, 0, 0, 0), Vector4(0, -1, 0, 0), Vector4(0, 0, -1, 0), Vector4(0, 0, 0, -1));
		mesh->merge_with(other, Transform4D(negate_all, Vector4(10, 0, 0, 0)));
		const PackedVector4Array normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE(normals.size() == 2);
		CHECK(normals[1].is_equal_approx(Vector4(0, 0, 0, -1)));
		const PackedVector4Array vertices = mesh->get_poly_cell_vertex_positions();
		CHECK(vertices[5].is_equal_approx(Vector4(9, 0, 0, 0)));
	}

	SUBCASE("Merging offsets the other mesh's seam face indices") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_cell_mesh();
		other->set_seam_face_indices_bind(PackedInt32Array{ 0, 2 });
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		CHECK((mesh->get_seam_face_indices_bind() == PackedInt32Array{ 4, 6 }));
	}

	SUBCASE("Merging a mesh with fewer dimensions does not crash") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		// A face-only mesh has one less poly cell dimension than a mesh with cells.
		Ref<ArrayPolyMesh4D> other;
		other.instantiate();
		other->append_vertex(Vector4(10, 0, 0, 0));
		other->append_vertex(Vector4(11, 0, 0, 0));
		other->append_vertex(Vector4(10, 1, 0, 0));
		other->append_edge_indices(0, 1);
		other->append_edge_indices(0, 2);
		other->append_edge_indices(1, 2);
		other->append_poly_cell(2, PackedInt32Array{ 0, 2, 1 }, false);
		mesh->merge_with(other, Transform4D());
		CHECK(mesh->get_poly_cell_vertex_positions().size() == 7);
		CHECK(mesh->get_edge_indices().size() == 18);
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		REQUIRE(poly_cell_indices.size() == 2);
		REQUIRE(poly_cell_indices[0].size() == 5);
		CHECK_MESSAGE((poly_cell_indices[0][4] == PackedInt32Array{ 6, 8, 7 }), "The merged face must reference the offset edges.");
		CHECK(poly_cell_indices[1].size() == 1);
		CHECK(mesh->is_poly_mesh_data_valid());
	}

	SUBCASE("Merging a mesh with more dimensions expands this mesh") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->append_vertex(Vector4(10, 0, 0, 0));
		mesh->append_vertex(Vector4(11, 0, 0, 0));
		mesh->append_vertex(Vector4(10, 1, 0, 0));
		mesh->append_edge_indices(0, 1);
		mesh->append_edge_indices(0, 2);
		mesh->append_edge_indices(1, 2);
		mesh->append_poly_cell(2, PackedInt32Array{ 0, 2, 1 }, false);
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_cell_mesh();
		mesh->merge_with(other, Transform4D());
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		REQUIRE(poly_cell_indices.size() == 2);
		CHECK(poly_cell_indices[0].size() == 5);
		REQUIRE(poly_cell_indices[1].size() == 1);
		CHECK_MESSAGE((poly_cell_indices[1][0] == PackedInt32Array{ 1, 2, 3, 4 }), "The merged cell must reference the offset faces.");
		CHECK(mesh->is_poly_mesh_data_valid());
	}

	SUBCASE("Merging generates missing boundary normals for the other mesh") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_cell_mesh();
		mesh->calculate_boundary_normals();
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		const PackedVector4Array normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE_MESSAGE(normals.size() == 2, "The other mesh's missing normals must be generated during the merge.");
		CHECK(normals[0].is_equal_approx(Vector4(0, 0, 0, 1)));
		CHECK_MESSAGE(normals[1].is_equal_approx(Vector4(0, 0, 0, 1)), "The generated normal must come from the cell orientation.");
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Merging two valid meshes must produce a valid mesh.");
	}

	SUBCASE("Merging generates missing vertex normals for the other mesh") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_cell_mesh();
		mesh->set_flat_shading_normals();
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		const Vector<PackedVector4Array> vertex_normals = mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE_MESSAGE(vertex_normals.size() == 2, "The other mesh's missing vertex normals must be generated during the merge.");
		REQUIRE(vertex_normals[1].size() == 4);
		for (int64_t vertex_in_cell = 0; vertex_in_cell < 4; vertex_in_cell++) {
			CHECK_MESSAGE(vertex_normals[1][vertex_in_cell].is_equal_approx(Vector4(0, 0, 0, 1)), "The generated vertex normals must be flat shading normals.");
		}
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Merging two valid meshes must produce a valid mesh.");
	}

	SUBCASE("Merging pads missing texture maps for the other mesh") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_cell_mesh();
		Vector<PackedVector3Array> texture_map;
		texture_map.push_back(PackedVector3Array{ Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) });
		mesh->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, texture_map);
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		const Vector<PackedVector3Array> merged_texture_map = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE_MESSAGE(merged_texture_map.size() == 2, "The texture map must be padded to cover the merged cells.");
		CHECK(merged_texture_map[0].size() == 4);
		CHECK_MESSAGE(merged_texture_map[1].is_empty(), "The other mesh's cell must be padded as unmapped.");
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "Merging two valid meshes must produce a valid mesh.");
	}

	SUBCASE("Merging a non-array poly mesh converts it") {
		Ref<BoxPolyMesh4D> box;
		box.instantiate();
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->merge_with(box, Transform4D());
		CHECK(mesh->get_poly_cell_vertex_positions() == box->get_poly_cell_vertex_positions());
		CHECK(mesh->get_edge_indices() == box->get_edge_indices());
		CHECK(mesh->get_poly_cell_indices().size() == 3);
		CHECK(mesh->get_poly_cell_boundary_normals() == box->get_poly_cell_boundary_normals());
		CHECK(mesh->is_poly_mesh_data_valid());
	}
}

TEST_CASE("[ArrayPolyMesh4D] Deduplicate all elements") {
	SUBCASE("Duplicate vertices, edges, and faces are removed") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		mesh->append_vertex(Vector4(0, 0, 1, 0), false); // Duplicate of vertex 3.
		mesh->append_edge_indices(2, 4, false); // Becomes a duplicate of edge 5 (2, 3) after vertex dedup.
		mesh->append_poly_cell(2, PackedInt32Array{ 3, 5, 4 }, false); // Duplicate of face 3.
		CHECK(mesh->get_poly_cell_vertex_positions().size() == 5);
		CHECK(mesh->get_edge_indices().size() == 14);
		CHECK(mesh->get_poly_cell_indices()[0].size() == 5);
		mesh->deduplicate_all_elements();
		CHECK(mesh->get_poly_cell_vertex_positions().size() == 4);
		CHECK(mesh->get_edge_indices().size() == 12);
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		CHECK(poly_cell_indices[0].size() == 4);
		CHECK(poly_cell_indices[1].size() == 1);
		CHECK(mesh->is_poly_mesh_data_valid());
		const PackedVector4Array normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE(normals.size() == 1);
		CHECK_MESSAGE(normals[0].is_equal_approx(Vector4(0, 0, 0, 1)), "Deduplication must preserve the boundary normal direction.");
	}

	SUBCASE("Deduplicating a mesh without boundary cells does not crash") {
		// A face-only mesh has no 3D cells, so the boundary normal handling must be skipped.
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->append_vertex(Vector4(0, 0, 0, 0));
		mesh->append_vertex(Vector4(1, 0, 0, 0));
		mesh->append_vertex(Vector4(0, 1, 0, 0));
		mesh->append_vertex(Vector4(0, 1, 0, 0), false); // Duplicate of vertex 2.
		mesh->append_edge_indices(0, 1);
		mesh->append_edge_indices(0, 2);
		mesh->append_edge_indices(1, 2);
		mesh->append_edge_indices(1, 3, false); // Becomes a duplicate of edge 2 (1, 2) after vertex dedup.
		mesh->append_poly_cell(2, PackedInt32Array{ 0, 2, 1 }, false);
		mesh->append_poly_cell(2, PackedInt32Array{ 2, 0, 1 }, false); // Duplicate of face 0.
		mesh->deduplicate_all_elements();
		CHECK(mesh->get_poly_cell_vertex_positions().size() == 3);
		CHECK(mesh->get_edge_indices().size() == 6);
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		REQUIRE(poly_cell_indices.size() == 1);
		CHECK(poly_cell_indices[0].size() == 1);
		CHECK(mesh->is_poly_mesh_data_valid());
	}

	SUBCASE("Deduplication preserves custom boundary normals as the reference") {
		// The cell orientation gives +W, but the custom normal says -W. Deduplication must
		// keep the custom normal and align the cell orientation to it, instead of silently
		// recalculating the reference normals from the orientation.
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		mesh->set_poly_cell_boundary_normals(PackedVector4Array{ Vector4(0, 0, 0, -1) });
		mesh->append_vertex(Vector4(0, 0, 1, 0), false); // Duplicate of vertex 3.
		mesh->append_edge_indices(2, 4, false); // Becomes a duplicate of edge 5 (2, 3) after vertex dedup.
		mesh->deduplicate_all_elements();
		const PackedVector4Array normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE(normals.size() == 1);
		CHECK_MESSAGE(normals[0].is_equal_approx(Vector4(0, 0, 0, -1)), "The custom boundary normal must be preserved.");
		mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
		CHECK_MESSAGE(mesh->get_poly_cell_boundary_normals()[0].is_equal_approx(Vector4(0, 0, 0, -1)), "The cell orientation must be aligned to the custom normal.");
		CHECK(mesh->is_poly_mesh_data_valid());
	}

	SUBCASE("Two merged boxes share an interface cell after deduplication") {
		// Merging a translated copy of a box and deduplicating produces two adjacent
		// hyper-cells sharing one boundary cell. The shared cell is used by two volumetric
		// cells, so the simplex decomposition must skip it as an invisible internal cell.
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		Ref<ArrayPolyMesh4D> other = make_box_array_mesh();
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(1, 0, 0, 0)));
		mesh->deduplicate_all_elements();
		CHECK_MESSAGE(mesh->get_poly_cell_vertex_positions().size() == 24, "The 8 interface vertices must be deduplicated.");
		CHECK_MESSAGE(mesh->get_edge_indices().size() == 104, "The 12 interface edges must be deduplicated.");
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		REQUIRE(poly_cell_indices.size() == 3);
		CHECK_MESSAGE(poly_cell_indices[0].size() == 42, "The 6 interface faces must be deduplicated.");
		CHECK_MESSAGE(poly_cell_indices[1].size() == 15, "The interface cell must be deduplicated.");
		CHECK(poly_cell_indices[2].size() == 2);
		CHECK(mesh->is_poly_mesh_data_valid());
		// Find the interface cell: the one whose vertices all have x at the interface plane.
		const PackedVector4Array vertices = mesh->get_poly_cell_vertex_positions();
		const Vector<PackedInt32Array> cell_vertex_indices = mesh->get_all_boundary_cell_vertex_indices(false);
		int32_t interface_cell = -1;
		for (int64_t cell_index = 0; cell_index < cell_vertex_indices.size(); cell_index++) {
			bool all_at_interface = true;
			for (const int32_t vertex_index : cell_vertex_indices[cell_index]) {
				if (!Math::is_equal_approx(vertices[vertex_index].x, (real_t)0.5)) {
					all_at_interface = false;
					break;
				}
			}
			if (all_at_interface) {
				interface_cell = (int32_t)cell_index;
				break;
			}
		}
		REQUIRE_MESSAGE(interface_cell != -1, "There must be a cell entirely at the interface plane.");
		const PackedInt32Array simplex_vertex_indices = mesh->get_simplex_cell_vertex_indices();
		REQUIRE(simplex_vertex_indices.size() > 0);
		const int64_t simplex_count = simplex_vertex_indices.size() / 4;
		for (int64_t simplex_index = 0; simplex_index < simplex_count; simplex_index++) {
			CHECK_MESSAGE(mesh->get_source_poly_cell_for_simplex_cell(simplex_index) != interface_cell, "The interface cell between two volumetric cells must not be tetrahedralized.");
		}
	}
}

TEST_CASE("[ArrayPolyMesh4D] Getters and setters") {
	SUBCASE("Boundary normals") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		CHECK(mesh->get_poly_cell_boundary_normals().is_empty());
		mesh->set_poly_cell_boundary_normals(PackedVector4Array{ Vector4(0, 0, 0, 1) });
		CHECK((mesh->get_poly_cell_boundary_normals() == PackedVector4Array{ Vector4(0, 0, 0, 1) }));
		mesh->set_poly_cell_boundary_normals(PackedVector4Array());
		CHECK_MESSAGE(mesh->get_poly_cell_boundary_normals().is_empty(), "Setting empty boundary normals should erase them.");
	}

	SUBCASE("Vertex normals") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		CHECK(mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY).is_empty());
		Vector<PackedVector4Array> vertex_normals;
		vertex_normals.push_back(PackedVector4Array{ Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1) });
		mesh->set_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY, vertex_normals);
		CHECK(mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY).size() == 1);
		mesh->set_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY, Vector<PackedVector4Array>());
		CHECK(mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY).is_empty());
	}

	SUBCASE("Texture map") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		CHECK(mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY).is_empty());
		Vector<PackedVector3Array> texture_map;
		texture_map.push_back(PackedVector3Array{ Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) });
		mesh->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, texture_map);
		CHECK(mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY).size() == 1);
		mesh->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, Vector<PackedVector3Array>());
		CHECK(mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY).is_empty());
	}

	SUBCASE("All normals and texture maps by binding key") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		mesh->calculate_boundary_normals();
		HashMap<Vector2i, Vector<PackedInt32Array>> all_normal_indices = mesh->get_all_poly_cell_normal_indices();
		REQUIRE(all_normal_indices.has(PolyMesh4D::PER_CELL_KEY));
		CHECK(all_normal_indices[PolyMesh4D::PER_CELL_KEY][0].size() == 1);
		// Round trip through the setters. The value pool must be copied along with the indices.
		Ref<ArrayPolyMesh4D> other = make_tetrahedron_cell_mesh();
		other->set_poly_cell_normal_values(mesh->get_poly_cell_normal_values());
		other->set_all_poly_cell_normal_indices(all_normal_indices);
		CHECK(other->get_poly_cell_boundary_normals() == mesh->get_poly_cell_boundary_normals());
		Vector<PackedVector3Array> texture_map;
		texture_map.push_back(PackedVector3Array{ Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) });
		mesh->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, texture_map);
		HashMap<Vector2i, Vector<PackedInt32Array>> all_texture_map_indices = mesh->get_all_poly_cell_texture_map_indices();
		REQUIRE(all_texture_map_indices.has(PolyMesh4D::CELL_TO_VERT_KEY));
		other->set_poly_cell_texture_map_values(mesh->get_poly_cell_texture_map_values());
		other->set_all_poly_cell_texture_map_indices(all_texture_map_indices);
		CHECK(other->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY).size() == 1);
	}

	SUBCASE("Edge indices and poly cell indices round trip") {
		Ref<ArrayPolyMesh4D> source = make_tetrahedron_cell_mesh();
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->set_poly_cell_vertex_positions(source->get_poly_cell_vertex_positions());
		mesh->set_edge_vertex_indices(source->get_edge_indices());
		mesh->set_poly_cell_indices(source->get_poly_cell_indices());
		CHECK(mesh->get_poly_cell_vertex_positions() == source->get_poly_cell_vertex_positions());
		CHECK(mesh->get_edge_indices() == source->get_edge_indices());
		CHECK(mesh->get_poly_cell_indices().size() == 2);
		CHECK(mesh->is_poly_mesh_data_valid());
	}
}

TEST_CASE("[ArrayPolyMesh4D] Duplicate preserves the normals and texture map dictionaries") {
	// A flat mesh stores its normals under the per-face key, which only survives
	// duplication if the dictionaries are bound as properties on all Godot versions.
	Ref<ArrayPolyMesh4D> mesh;
	mesh.instantiate();
	PackedVector4Array vertices = { Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) };
	mesh->set_poly_cell_vertex_positions(vertices);
	mesh->set_edge_vertex_indices(PackedInt32Array{ 0, 1, 1, 2, 0, 2 });
	Vector<PackedInt32Array> faces;
	faces.append(PackedInt32Array{ 0, 1, 2 });
	mesh->set_poly_cell_indices(Vector<Vector<PackedInt32Array>>{ faces });
	const Vector4 pos_z = Vector4(0, 0, 1, 0);
	mesh->set_poly_cell_dense_normals(PolyMesh4D::PER_FACE_KEY, Vector<PackedVector4Array>{ PackedVector4Array{ pos_z } });
	mesh->set_poly_cell_dense_texture_map(PolyMesh4D::FACE_TO_VERT_KEY, Vector<PackedVector3Array>{ PackedVector3Array{ Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0) } });
	Ref<ArrayPolyMesh4D> duplicated = mesh->duplicate();
	REQUIRE(duplicated.is_valid());
	const Vector<PackedVector4Array> duplicated_normals = duplicated->get_poly_cell_dense_normals(PolyMesh4D::PER_FACE_KEY);
	REQUIRE_MESSAGE(!duplicated_normals.is_empty(), "Duplicating a mesh must preserve the per-face normals.");
	REQUIRE(duplicated_normals.size() == 1);
	REQUIRE(duplicated_normals[0].size() == 1);
	CHECK(duplicated_normals[0][0].is_equal_approx(pos_z));
	const Vector<PackedVector3Array> duplicated_texture_maps = duplicated->get_poly_cell_dense_texture_map(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE_MESSAGE(!duplicated_texture_maps.is_empty(), "Duplicating a mesh must preserve the face texture maps.");
	REQUIRE(duplicated_texture_maps.size() == 1);
	CHECK(duplicated_texture_maps[0].size() == 3);
}

TEST_CASE("[ArrayPolyMesh4D] Boundary pivot overrides follow mesh edits") {
	for (const bool source_has_overrides : { false, true }) {
		Ref<ArrayPolyMesh4D> mesh = make_two_tetrahedra_cells_mesh();
		Ref<ArrayPolyMesh4D> other = make_two_tetrahedra_cells_mesh();
		// The omitted destination entry and an explicit source sentinel both mean no override.
		mesh->set_poly_cell_boundary_pivot_overrides(PackedInt32Array{ 2 });
		if (source_has_overrides) {
			other->set_poly_cell_boundary_pivot_overrides(PackedInt32Array{ -1, 4 });
		}
		mesh->merge_with(other, Transform4D(Basis4D(), Vector4(10, 0, 0, 0)));
		CHECK(mesh->get_poly_cell_boundary_pivot_overrides() == PackedInt32Array({ 2, -1, -1, source_has_overrides ? 9 : -1 }));
		CHECK(mesh->is_poly_mesh_data_valid());
	}
	{
		Ref<ArrayPolyMesh4D> mesh = make_two_tetrahedra_cells_mesh();
		mesh->set_poly_cell_boundary_pivot_overrides(PackedInt32Array{ -1, -1, -1 });
		ERR_PRINT_OFF;
		CHECK_FALSE(mesh->is_poly_mesh_data_valid());
		ERR_PRINT_ON;
		mesh->set_poly_cell_boundary_pivot_overrides(PackedInt32Array{ -1 });
		CHECK(mesh->is_poly_mesh_data_valid());
	}
	{
		Ref<ArrayPolyMesh4D> mesh = make_two_tetrahedra_cells_mesh();
		mesh->set_poly_cell_boundary_pivot_overrides(PackedInt32Array{ 2, 4 });
		mesh->delete_poly_element(3, 0);
		CHECK(mesh->get_poly_cell_boundary_pivot_overrides() == PackedInt32Array({ 4 }));
		CHECK(mesh->is_poly_mesh_data_valid());
	}
	{
		Ref<ArrayPolyMesh4D> mesh = make_two_tetrahedra_cells_mesh();
		// External pivot vertices have no incident edges, so deleting them leaves both cells intact.
		mesh->append_vertex(Vector4(7, 0, 0, 0));
		mesh->append_vertex(Vector4(8, 0, 0, 0));
		mesh->set_poly_cell_boundary_pivot_overrides(PackedInt32Array{ 5, 6 });
		mesh->delete_poly_element(0, 5);
		CHECK(mesh->get_poly_cell_boundary_pivot_overrides() == PackedInt32Array({ -1, 5 }));
		CHECK(mesh->is_poly_mesh_data_valid());
		mesh->delete_poly_element(0, 5);
		CHECK(mesh->get_poly_cell_boundary_pivot_overrides() == PackedInt32Array({ -1, -1 }));
		CHECK(mesh->is_poly_mesh_data_valid());
	}
}

static int64_t dense_binding_element_count(const Ref<ArrayPolyMesh4D> &p_mesh, const int p_dimension) {
	if (p_dimension == 0) {
		return p_mesh->get_poly_cell_vertex_positions().size();
	}
	if (p_dimension == 1) {
		return p_mesh->get_edge_indices().size() / 2;
	}
	const Vector<Vector<PackedInt32Array>> poly = p_mesh->get_poly_cell_indices();
	return p_dimension - 2 < poly.size() ? poly[p_dimension - 2].size() : 0;
}

static void clear_dense_test_bindings(const Ref<ArrayPolyMesh4D> &p_mesh) {
	set_all_dense_normals(p_mesh, HashMap<Vector2i, Vector<PackedVector4Array>>());
	set_all_dense_texture_maps(p_mesh, HashMap<Vector2i, Vector<PackedVector3Array>>());
}

static void set_dense_test_binding(const Ref<ArrayPolyMesh4D> &p_mesh, const Vector2i p_key, const int64_t p_count = -1, const int p_seed = 1) {
	const Vector<PackedInt32Array> elements = p_mesh->get_all_poly_cell_poly_indices(p_key.x, p_key.y);
	const int64_t count = p_count < 0 ? elements.size() : p_count;
	Vector<PackedVector4Array> normals;
	Vector<PackedVector3Array> textures;
	if (p_key.x == p_key.y) {
		PackedVector4Array flat_normals;
		PackedVector3Array flat_textures;
		for (int64_t i = 0; i < count; i++) {
			flat_normals.append(Vector4(i + 1, p_seed, 2, 3));
			flat_textures.append(Vector3(i + 1, p_seed, 2));
		}
		normals.append(flat_normals);
		textures.append(flat_textures);
	} else {
		for (int64_t i = 0; i < count; i++) {
			PackedVector4Array cell_normals;
			PackedVector3Array cell_textures;
			for (const int32_t element : elements[i]) {
				cell_normals.append(Vector4(element + 1, p_seed, i + 1, 3));
				cell_textures.append(Vector3(element + 1, p_seed, i + 1));
			}
			normals.append(cell_normals);
			textures.append(cell_textures);
		}
	}
	HashMap<Vector2i, Vector<PackedVector4Array>> all_normals = get_all_dense_normals(p_mesh);
	HashMap<Vector2i, Vector<PackedVector3Array>> all_textures = get_all_dense_texture_maps(p_mesh);
	all_normals.insert(p_key, normals);
	all_textures.insert(p_key, textures);
	set_all_dense_normals(p_mesh, all_normals);
	set_all_dense_texture_maps(p_mesh, all_textures);
}

TEST_CASE("[ArrayPolyMesh4D] Dense binding validation checks every geometry dimension") {
	for (int geometry_dimension = 0; geometry_dimension <= 4; geometry_dimension++) {
		for (int decomposition_dimension = 0; decomposition_dimension <= geometry_dimension; decomposition_dimension++) {
			const Vector2i key(geometry_dimension, decomposition_dimension);
			CAPTURE(key);
			Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
			clear_dense_test_bindings(mesh);
			set_dense_test_binding(mesh, key);
			CHECK(mesh->is_poly_mesh_data_valid());
			const Vector<PackedVector4Array> original_normals = get_all_dense_normals(mesh)[key];
			const Vector<PackedVector3Array> original_textures = get_all_dense_texture_maps(mesh)[key];
			for (const bool change_normals : { false, true }) {
				for (int invalid_shape = 0; invalid_shape < 2; invalid_shape++) {
					HashMap<Vector2i, Vector<PackedVector4Array>> normals;
					HashMap<Vector2i, Vector<PackedVector3Array>> textures;
					Vector<PackedVector4Array> normal_data = original_normals;
					Vector<PackedVector3Array> texture_data = original_textures;
					if (invalid_shape == 0) {
						if (change_normals) {
							normal_data.append(PackedVector4Array());
						} else {
							texture_data.append(PackedVector3Array());
						}
					} else if (change_normals) {
						normal_data.ptrw()[0].append(Vector4());
					} else {
						texture_data.ptrw()[0].append(Vector3());
					}
					normals.insert(key, normal_data);
					textures.insert(key, texture_data);
					set_all_dense_normals(mesh, normals);
					set_all_dense_texture_maps(mesh, textures);
					ERR_PRINT_OFF;
					CHECK_FALSE(mesh->is_poly_mesh_data_valid());
					ERR_PRINT_ON;
				}
			}
		}
	}
	for (const Vector2i key : { Vector2i(-1, 0), Vector2i(1, -1), Vector2i(1, 2) }) {
		for (const bool change_normals : { false, true }) {
			Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
			HashMap<Vector2i, Vector<PackedVector4Array>> normals;
			HashMap<Vector2i, Vector<PackedVector3Array>> textures;
			if (change_normals) {
				normals.insert(key, Vector<PackedVector4Array>());
			} else {
				textures.insert(key, Vector<PackedVector3Array>());
			}
			set_all_dense_normals(mesh, normals);
			set_all_dense_texture_maps(mesh, textures);
			ERR_PRINT_OFF;
			CHECK_FALSE(mesh->is_poly_mesh_data_valid());
			ERR_PRINT_ON;
		}
	}
}

TEST_CASE("[ArrayPolyMesh4D] Every indexed binding validates its value range") {
	for (int geometry_dimension = 0; geometry_dimension <= 4; geometry_dimension++) {
		for (int decomposition_dimension = 0; decomposition_dimension <= geometry_dimension; decomposition_dimension++) {
			const Vector2i key(geometry_dimension, decomposition_dimension);
			for (const bool normals : { false, true }) {
				for (const bool negative : { false, true }) {
					CAPTURE(key);
					CAPTURE(normals);
					CAPTURE(negative);
					Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
					clear_dense_test_bindings(mesh);
					set_dense_test_binding(mesh, key);
					REQUIRE(mesh->is_poly_mesh_data_valid());
					HashMap<Vector2i, Vector<PackedInt32Array>> bindings = normals ? mesh->get_all_poly_cell_normal_indices() : mesh->get_all_poly_cell_texture_map_indices();
					const int64_t value_count = normals ? mesh->get_poly_cell_normal_values().size() : mesh->get_poly_cell_texture_map_values().size();
					bindings[key].ptrw()[0].set(0, negative ? -1 : value_count);
					if (normals) {
						mesh->set_all_poly_cell_normal_indices(bindings);
					} else {
						mesh->set_all_poly_cell_texture_map_indices(bindings);
					}
					ERR_PRINT_OFF;
					CHECK_FALSE(mesh->is_poly_mesh_data_valid());
					ERR_PRINT_ON;
				}
			}
		}
	}
}

TEST_CASE("[ArrayPolyMesh4D] Dense bindings follow vertex edge and higher-element deletion") {
	for (int dimension = 0; dimension <= 4; dimension++) {
		CAPTURE(dimension);
		Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
		clear_dense_test_bindings(mesh);
		for (int geometry_dimension = 0; geometry_dimension <= 4; geometry_dimension++) {
			for (int decomposition_dimension = 0; decomposition_dimension <= geometry_dimension; decomposition_dimension++) {
				set_dense_test_binding(mesh, Vector2i(geometry_dimension, decomposition_dimension));
			}
		}
		mesh->delete_poly_element(dimension, 0);
		REQUIRE(mesh->is_poly_mesh_data_valid());
		const HashMap<Vector2i, Vector<PackedVector4Array>> normals = get_all_dense_normals(mesh);
		const HashMap<Vector2i, Vector<PackedVector3Array>> textures = get_all_dense_texture_maps(mesh);
		for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &binding : normals) {
			const int64_t count = dense_binding_element_count(mesh, binding.key.x);
			if (binding.key.x == binding.key.y) {
				REQUIRE(binding.value.size() == 1);
				CHECK(binding.value[0].size() == count);
				CHECK(textures[binding.key][0].size() == count);
			} else {
				CHECK(binding.value.size() == count);
				CHECK(textures[binding.key].size() == count);
			}
		}
		const Vector2i flat_key(dimension, dimension);
		if (dense_binding_element_count(mesh, dimension) > 0) {
			CHECK(normals[flat_key][0][0].x == 2);
			CHECK(textures[flat_key][0][0].x == 2);
		}
	}
	{
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		set_dense_test_binding(mesh, PolyMesh4D::FACE_TO_VERT_KEY, 1);
		mesh->delete_poly_element(2, 3); // Outside the stored prefix.
		CHECK(get_all_dense_normals(mesh)[PolyMesh4D::FACE_TO_VERT_KEY].size() == 1);
		CHECK(mesh->is_poly_mesh_data_valid());
		mesh->delete_poly_element(2, 0);
		CHECK(get_all_dense_normals(mesh)[PolyMesh4D::FACE_TO_VERT_KEY].is_empty());
		CHECK(get_all_dense_texture_maps(mesh)[PolyMesh4D::FACE_TO_VERT_KEY].is_empty());
		CHECK(mesh->is_poly_mesh_data_valid());
	}
}

TEST_CASE("[ArrayPolyMesh4D] Empty bindings survive deletion and deduplication") {
	Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
	HashMap<Vector2i, Vector<PackedVector4Array>> normals;
	HashMap<Vector2i, Vector<PackedVector3Array>> textures;
	for (const Vector2i key : { Vector2i(2, 2), Vector2i(3, 3), Vector2i(5, 5) }) {
		normals.insert(key, Vector<PackedVector4Array>{ PackedVector4Array() });
		textures.insert(key, Vector<PackedVector3Array>{ PackedVector3Array() });
	}
	normals.insert(Vector2i(5, 0), Vector<PackedVector4Array>());
	textures.insert(Vector2i(5, 0), Vector<PackedVector3Array>());
	set_all_dense_normals(mesh, normals);
	set_all_dense_texture_maps(mesh, textures);
	REQUIRE(mesh->is_poly_mesh_data_valid());
	mesh->delete_poly_element(3, 0);
	while (!mesh->get_poly_cell_indices().is_empty()) {
		mesh->delete_poly_element(2, 0);
	}
	REQUIRE(mesh->is_poly_mesh_data_valid());
	mesh->deduplicate_all_elements();
	CHECK(get_all_dense_normals(mesh).size() == normals.size());
	CHECK(get_all_dense_texture_maps(mesh).size() == textures.size());
	CHECK(get_all_dense_normals(mesh)[Vector2i(5, 0)].is_empty());
	CHECK(get_all_dense_texture_maps(mesh)[Vector2i(5, 5)][0].is_empty());
	CHECK(mesh->is_poly_mesh_data_valid());
}

TEST_CASE("[ArrayPolyMesh4D] Deduplication resamples partial and empty dense bindings") {
	SUBCASE("Coincident vertices shrink the derived binding cardinality") {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->set_poly_cell_vertex_positions(PackedVector4Array{ Vector4(), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 1, 0, 0) });
		mesh->set_edge_vertex_indices(PackedInt32Array{ 0, 1, 1, 2, 2, 3, 3, 0 });
		mesh->set_poly_cell_indices(Vector<Vector<PackedInt32Array>>{ Vector<PackedInt32Array>{ PackedInt32Array{ 0, 1, 2, 3 } } });
		set_dense_test_binding(mesh, PolyMesh4D::FACE_TO_VERT_KEY);
		REQUIRE(get_all_dense_normals(mesh)[PolyMesh4D::FACE_TO_VERT_KEY][0].size() == 4);
		mesh->deduplicate_all_elements();
		const PackedVector4Array normals = get_all_dense_normals(mesh)[PolyMesh4D::FACE_TO_VERT_KEY][0];
		const PackedVector3Array textures = get_all_dense_texture_maps(mesh)[PolyMesh4D::FACE_TO_VERT_KEY][0];
		REQUIRE(normals.size() == 3);
		REQUIRE(textures.size() == 3);
		CHECK(normals[2].x == 3); // Keep the first attached value when vertices 2 and 3 merge.
		CHECK(textures[2].x == 3);
		CHECK(mesh->is_poly_mesh_data_valid());
	}
	SUBCASE("A reversed retained edge changes the face vertex traversal") {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		PackedInt32Array edges = { 1, 0 };
		edges.append_array(mesh->get_edge_indices());
		mesh->set_edge_vertex_indices(edges);
		Vector<Vector<PackedInt32Array>> poly = mesh->get_poly_cell_indices();
		for (int64_t face = 0; face < poly[0].size(); face++) {
			PackedInt32Array face_edges = poly[0][face];
			for (int64_t i = 0; i < face_edges.size(); i++) {
				face_edges.set(i, face_edges[i] + 1);
			}
			poly.ptrw()[0].set(face, face_edges);
		}
		mesh->set_poly_cell_indices(poly);
		set_dense_test_binding(mesh, Vector2i(1, 0));
		set_dense_test_binding(mesh, Vector2i(2, 0));
		mesh->deduplicate_all_elements();
		for (const Vector2i key : { Vector2i(1, 0), Vector2i(2, 0) }) {
			const Vector<PackedInt32Array> elements = mesh->get_all_poly_cell_poly_indices(key.x, key.y);
			const Vector<PackedVector4Array> normals = get_all_dense_normals(mesh)[key];
			const Vector<PackedVector3Array> textures = get_all_dense_texture_maps(mesh)[key];
			REQUIRE(normals.size() == elements.size());
			for (int64_t i = 0; i < elements.size(); i++) {
				REQUIRE(normals[i].size() == elements[i].size());
				for (int64_t j = 0; j < elements[i].size(); j++) {
					CHECK(normals[i][j].x == elements[i][j] + 1);
					CHECK(textures[i][j].x == elements[i][j] + 1);
				}
			}
		}
		CHECK(mesh->is_poly_mesh_data_valid());
	}
	SUBCASE("Orientation repair resamples only the stored prefix and skips empty cells") {
		Ref<ArrayPolyMesh4D> mesh = make_two_tetrahedra_cells_mesh();
		set_dense_test_binding(mesh, Vector2i(3, 1), 1);
		set_dense_test_binding(mesh, Vector2i(3, 2), 1);
		mesh->calculate_boundary_normals();
		PackedVector4Array boundary_normals = mesh->get_poly_cell_boundary_normals();
		boundary_normals.set(0, -boundary_normals[0]);
		mesh->set_poly_cell_boundary_normals(boundary_normals);
		mesh->set_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY, Vector<PackedVector4Array>{ PackedVector4Array(), PackedVector4Array() });
		mesh->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, Vector<PackedVector3Array>{ PackedVector3Array(), PackedVector3Array() });
		mesh->deduplicate_all_elements();
		for (const Vector2i key : { Vector2i(3, 1), Vector2i(3, 2) }) {
			const Vector<PackedInt32Array> elements = mesh->get_all_poly_cell_poly_indices(key.x, key.y);
			const Vector<PackedVector4Array> normals = get_all_dense_normals(mesh)[key];
			const Vector<PackedVector3Array> textures = get_all_dense_texture_maps(mesh)[key];
			REQUIRE(normals.size() == 1);
			REQUIRE(textures.size() == 1);
			for (int64_t j = 0; j < elements[0].size(); j++) {
				CHECK(normals[0][j].x == elements[0][j] + 1);
				CHECK(textures[0][j].x == elements[0][j] + 1);
			}
		}
		CHECK(mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY)[0].is_empty());
		CHECK(mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY)[0].is_empty());
		CHECK(mesh->is_poly_mesh_data_valid());
	}
}

TEST_CASE("[ArrayPolyMesh4D] Dense merge preserves prefixes and pads at the geometry offset") {
	const Transform4D transform(Basis4D::from_scale_uniform(2.0), Vector4(10, 0, 0, 0));
	for (int geometry_dimension = 0; geometry_dimension <= 4; geometry_dimension++) {
		for (int decomposition_dimension = 0; decomposition_dimension <= geometry_dimension; decomposition_dimension++) {
			const Vector2i key(geometry_dimension, decomposition_dimension);
			for (int destination_data = 0; destination_data <= 2; destination_data++) {
				for (int source_data = 0; source_data <= 2; source_data++) {
					CAPTURE(key);
					CAPTURE(destination_data);
					CAPTURE(source_data);
					Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
					Ref<ArrayPolyMesh4D> source = make_box_array_mesh();
					clear_dense_test_bindings(mesh);
					clear_dense_test_bindings(source);
					const int64_t count = dense_binding_element_count(mesh, key.x);
					const bool boundary_binding = key == PolyMesh4D::PER_CELL_KEY || key == PolyMesh4D::CELL_TO_VERT_KEY;
					if (destination_data > 0) {
						set_dense_test_binding(mesh, key, destination_data == 1 && !boundary_binding ? 1 : count, 5);
					}
					if (source_data > 0) {
						set_dense_test_binding(source, key, source_data == 1 && !boundary_binding ? 1 : count, 7);
					}
					const HashMap<Vector2i, Vector<PackedVector4Array>> original_normals = get_all_dense_normals(mesh);
					const HashMap<Vector2i, Vector<PackedVector3Array>> original_textures = get_all_dense_texture_maps(mesh);
					const HashMap<Vector2i, Vector<PackedVector4Array>> source_normals = get_all_dense_normals(source);
					const HashMap<Vector2i, Vector<PackedVector3Array>> source_textures = get_all_dense_texture_maps(source);
					const int64_t destination_count = destination_data == 0 ? 0 : (destination_data == 1 && !boundary_binding ? 1 : count);
					const int64_t source_count = source_data == 0 ? 0 : (source_data == 1 && !boundary_binding ? 1 : count);
					const bool expect_padding_warning = (source_data > 0 && destination_count < count) ||
							(!boundary_binding && (destination_data > 0 || source_data > 0) && source_count < count);
					if (expect_padding_warning) {
						ERR_PRINT_OFF;
					}
					mesh->merge_with(source, transform);
					if (expect_padding_warning) {
						ERR_PRINT_ON;
					}
					REQUIRE(mesh->is_poly_mesh_data_valid());
					const HashMap<Vector2i, Vector<PackedVector4Array>> normals = get_all_dense_normals(mesh);
					const HashMap<Vector2i, Vector<PackedVector3Array>> textures = get_all_dense_texture_maps(mesh);
					if (destination_data == 0 && source_data == 0) {
						CHECK_FALSE(normals.has(key));
						CHECK_FALSE(textures.has(key));
						continue;
					}
					REQUIRE(normals.has(key));
					REQUIRE(textures.has(key));
					if (key.x == key.y) {
						REQUIRE(normals[key].size() == 1);
						REQUIRE(normals[key][0].size() == count * 2);
						REQUIRE(textures[key][0].size() == count * 2);
						for (int64_t i = 0; i < count * 2; i++) {
							Vector4 expected_normal;
							Vector3 expected_texture;
							bool has_normal = false;
							if (i < count && destination_data > 0 && i < original_normals[key][0].size()) {
								expected_normal = original_normals[key][0][i];
								expected_texture = original_textures[key][0][i];
								has_normal = true;
							} else if (i >= count && source_data > 0 && i - count < source_normals[key][0].size()) {
								expected_normal = transform.basis.xform(source_normals[key][0][i - count]);
								expected_texture = source_textures[key][0][i - count];
								has_normal = true;
							}
							if (has_normal || key != PolyMesh4D::PER_CELL_KEY) {
								CHECK(normals[key][0][i] == expected_normal);
							}
							CHECK(textures[key][0][i] == expected_texture);
						}
					} else {
						REQUIRE(normals[key].size() == count * 2);
						REQUIRE(textures[key].size() == count * 2);
						for (int64_t i = 0; i < count * 2; i++) {
							PackedVector4Array expected_normals;
							PackedVector3Array expected_textures;
							bool has_normals = false;
							if (i < count && destination_data > 0 && i < original_normals[key].size()) {
								expected_normals = original_normals[key][i];
								expected_textures = original_textures[key][i];
								has_normals = true;
							} else if (i >= count && source_data > 0 && i - count < source_normals[key].size()) {
								expected_normals = source_normals[key][i - count];
								for (int64_t j = 0; j < expected_normals.size(); j++) {
									expected_normals.set(j, transform.basis.xform(expected_normals[j]));
								}
								expected_textures = source_textures[key][i - count];
								has_normals = true;
							}
							if (has_normals || key != PolyMesh4D::CELL_TO_VERT_KEY) {
								CHECK(normals[key][i] == expected_normals);
							}
							CHECK(textures[key][i] == expected_textures);
						}
					}
					if (source_data > 0) {
						CHECK(get_all_dense_normals(source)[key] == source_normals[key]);
						CHECK(get_all_dense_texture_maps(source)[key] == source_textures[key]);
					}
				}
			}
		}
	}
}

TEST_CASE("[ArrayPolyMesh4D] Empty incoming bindings do not materialize data") {
	for (const bool destination_has_empty_keys : { false, true }) {
		Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
		Ref<ArrayPolyMesh4D> source = make_tetrahedron_cell_mesh();
		HashMap<Vector2i, Vector<PackedVector4Array>> normals;
		HashMap<Vector2i, Vector<PackedVector3Array>> textures;
		for (const Vector2i key : { Vector2i(0, 0), Vector2i(2, 2), Vector2i(3, 3), Vector2i(5, 5) }) {
			normals.insert(key, Vector<PackedVector4Array>{ PackedVector4Array() });
			textures.insert(key, Vector<PackedVector3Array>{ PackedVector3Array() });
		}
		normals.insert(Vector2i(3, 0), Vector<PackedVector4Array>());
		textures.insert(Vector2i(3, 0), Vector<PackedVector3Array>());
		set_all_dense_normals(source, normals);
		set_all_dense_texture_maps(source, textures);
		if (destination_has_empty_keys) {
			set_all_dense_normals(mesh, normals);
			set_all_dense_texture_maps(mesh, textures);
		}
		mesh->merge_with(source);
		const HashMap<Vector2i, Vector<PackedVector4Array>> merged_normals = get_all_dense_normals(mesh);
		const HashMap<Vector2i, Vector<PackedVector3Array>> merged_textures = get_all_dense_texture_maps(mesh);
		CHECK(merged_normals.size() == (destination_has_empty_keys ? normals.size() : 0));
		CHECK(merged_textures.size() == (destination_has_empty_keys ? textures.size() : 0));
		if (destination_has_empty_keys) {
			for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &binding : normals) {
				CHECK(merged_normals[binding.key] == binding.value);
				CHECK(merged_textures[binding.key] == textures[binding.key]);
			}
		}
		CHECK(mesh->is_poly_mesh_data_valid());
	}
}

TEST_CASE("[ArrayPolyMesh4D] Self merge snapshots dense attributes and clears primed caches") {
	Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
	set_dense_test_binding(mesh, PolyMesh4D::CELL_TO_VERT_KEY);
	set_dense_test_binding(mesh, PolyMesh4D::PER_FACE_KEY);
	mesh->set_poly_cell_boundary_normals(PackedVector4Array{ Vector4(0, 0, 0, 1) });
	const HashMap<Vector2i, Vector<PackedVector4Array>> original_normals = get_all_dense_normals(mesh);
	const HashMap<Vector2i, Vector<PackedVector3Array>> original_textures = get_all_dense_texture_maps(mesh);
	const int64_t simplex_index_count = mesh->get_simplex_cell_vertex_indices().size();
	const int64_t simplex_position_count = mesh->get_simplex_cell_positions().size();
	const int64_t texture_count = sample_texture_map_values(mesh).size();
	const int64_t normal_count = sample_normal_values(mesh).size();
	const Transform4D transform(Basis4D::from_scale_uniform(2.0), Vector4(10, 0, 0, 0));
	mesh->merge_with(mesh, transform);
	CHECK(mesh->get_simplex_cell_vertex_indices().size() == simplex_index_count * 2);
	CHECK(mesh->get_simplex_cell_positions().size() == simplex_position_count * 2);
	CHECK(sample_texture_map_values(mesh).size() == texture_count * 2);
	CHECK(sample_normal_values(mesh).size() == normal_count * 2);
	for (const KeyValue<Vector2i, Vector<PackedVector4Array>> &binding : original_normals) {
		const Vector<PackedVector4Array> merged = get_all_dense_normals(mesh)[binding.key];
		if (binding.key.x == binding.key.y) {
			const int64_t count = binding.value[0].size();
			REQUIRE(merged[0].size() == count * 2);
			for (int64_t i = 0; i < count; i++) {
				CHECK(merged[0][i] == binding.value[0][i]);
				CHECK(merged[0][i + count] == transform.basis.xform(binding.value[0][i]));
			}
		} else {
			REQUIRE(merged.size() == binding.value.size() * 2);
			CHECK(merged[0] == binding.value[0]);
			for (int64_t i = 0; i < merged[1].size(); i++) {
				CHECK(merged[1][i] == transform.basis.xform(binding.value[0][i]));
			}
		}
	}
	for (const KeyValue<Vector2i, Vector<PackedVector3Array>> &binding : original_textures) {
		const Vector<PackedVector3Array> merged = get_all_dense_texture_maps(mesh)[binding.key];
		if (binding.key.x == binding.key.y) {
			PackedVector3Array expected = binding.value[0];
			expected.append_array(binding.value[0]);
			CHECK(merged[0] == expected);
		} else {
			CHECK(merged[0] == binding.value[0]);
			CHECK(merged[1] == binding.value[0]);
		}
	}
	CHECK(mesh->is_mesh_data_valid());
}

TEST_CASE("[ArrayPolyMesh4D] Double sided attributes follow vertex identity and preserve missing data") {
	const int dimension = 4;
	{
		// Missing keys, empty arrays, missing cells, full data, texture-only data, and normal-only data.
		for (int pattern = 0; pattern < 7; pattern++) {
			CAPTURE(pattern);
			Ref<ArrayPolyMesh4D> mesh = make_box_array_mesh();
			set_all_dense_normals(mesh, HashMap<Vector2i, Vector<PackedVector4Array>>());
			set_all_dense_texture_maps(mesh, HashMap<Vector2i, Vector<PackedVector3Array>>());
			const Vector<PackedInt32Array> cell_vertices = mesh->get_all_boundary_cell_vertex_indices(false);
			Vector<PackedVector4Array> normals;
			Vector<PackedVector3Array> texture_map;
			for (int64_t cell = 0; cell < cell_vertices.size(); cell++) {
				PackedVector4Array cell_normals;
				PackedVector3Array cell_map;
				for (const int32_t vertex : cell_vertices[cell]) {
					cell_normals.append(Vector4(vertex + 1, cell + 2, 3, 4).normalized());
					cell_map.append(Vector3(vertex + 0.25, cell + 0.5, (vertex + cell) * 0.125));
				}
				if (pattern == 2 || (pattern == 3 && (cell == 0 || cell == cell_vertices.size() - 1))) {
					cell_normals.clear();
				}
				if (pattern == 2 || (pattern == 3 && cell == cell_vertices.size() / 2)) {
					cell_map.clear();
				}
				normals.append(cell_normals);
				texture_map.append(cell_map);
			}
			if (pattern == 1 || pattern == 5) {
				normals.clear();
			}
			if (pattern == 1 || pattern == 6) {
				texture_map.clear();
			}
			if (pattern != 0) {
				mesh->set_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY, normals);
				mesh->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, texture_map);
			}
			if (pattern == 1) {
				HashMap<Vector2i, Vector<PackedInt32Array>> empty_binding;
				empty_binding.insert(PolyMesh4D::CELL_TO_VERT_KEY, Vector<PackedInt32Array>());
				mesh->set_all_poly_cell_normal_indices(empty_binding);
				mesh->set_all_poly_cell_texture_map_indices(empty_binding);
			}
			// A short pivot array includes an explicit missing sentinel and implicit missing tail.
			mesh->set_poly_cell_boundary_pivot_overrides(PackedInt32Array{ cell_vertices[0][0], -1, cell_vertices[2][0] });
			mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY, false);
			REQUIRE(mesh->is_poly_mesh_data_valid());
			if (pattern == 3) {
				ERR_PRINT_OFF; // Validation samples the deliberately partial attributes.
			}
			const bool initially_valid = mesh->is_mesh_data_valid();
			if (pattern == 3) {
				ERR_PRINT_ON;
			}
			REQUIRE(initially_valid);
			for (int pass = 0; pass < 3; pass++) {
				CAPTURE(pass);
				const auto before_geometry = mesh->get_poly_cell_indices();
				const auto before_vertices = mesh->get_all_boundary_cell_vertex_indices(false);
				const auto before_normals = mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
				const auto before_texture_map = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
				const auto before_boundary_normals = mesh->get_poly_cell_boundary_normals();
				const PackedInt32Array before_pivots = mesh->get_poly_cell_boundary_pivot_overrides();
				mesh->make_double_sided(pass != 2);
				const auto after_geometry = mesh->get_poly_cell_indices();
				if (pattern == 3) {
					ERR_PRINT_OFF; // This getter validates and samples the deliberately partial attributes.
				}
				const auto after_vertices = mesh->get_all_boundary_cell_vertex_indices(false);
				if (pattern == 3) {
					ERR_PRINT_ON;
				}
				const auto after_normals = mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
				const auto after_texture_map = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
				const auto after_boundary_normals = mesh->get_poly_cell_boundary_normals();
				const PackedInt32Array after_pivots = mesh->get_poly_cell_boundary_pivot_overrides();
				if (pass == 1) {
					CHECK(after_geometry == before_geometry);
					CHECK(after_normals == before_normals);
					CHECK(after_texture_map == before_texture_map);
					CHECK(after_boundary_normals == before_boundary_normals);
					CHECK(after_pivots == before_pivots);
					continue;
				}
				const int64_t original_count = before_vertices.size();
				REQUIRE(after_vertices.size() == original_count * 2);
				REQUIRE(after_boundary_normals.size() == original_count * 2);
				REQUIRE(after_normals.size() == before_normals.size() * 2);
				REQUIRE(after_texture_map.size() == before_texture_map.size() * 2);
				bool order_changed = false;
				for (int64_t cell = 0; cell < original_count; cell++) {
					const int64_t flipped = original_count + cell;
					CHECK(after_geometry[dimension - 3][cell] == before_geometry[dimension - 3][cell]);
					CHECK(after_vertices[cell] == before_vertices[cell]);
					CHECK(after_boundary_normals[cell] == before_boundary_normals[cell]);
					const auto expected_boundary_normal = -before_boundary_normals[cell];
					CHECK(after_boundary_normals[flipped] == expected_boundary_normal);
					const int32_t expected_pivot = cell < before_pivots.size() ? before_pivots[cell] : -1;
					const int32_t original_pivot = cell < after_pivots.size() ? after_pivots[cell] : -1;
					const int32_t flipped_pivot = flipped < after_pivots.size() ? after_pivots[flipped] : -1;
					CHECK(original_pivot == expected_pivot);
					CHECK(flipped_pivot == expected_pivot);
					order_changed |= before_vertices[cell] != after_vertices[flipped];
					if (!before_normals.is_empty()) {
						CHECK(after_normals[cell] == before_normals[cell]);
						REQUIRE(after_normals[flipped].size() == before_normals[cell].size());
					}
					if (!before_texture_map.is_empty()) {
						CHECK(after_texture_map[cell] == before_texture_map[cell]);
						REQUIRE(after_texture_map[flipped].size() == before_texture_map[cell].size());
					}
					for (int64_t slot = 0; slot < after_vertices[flipped].size(); slot++) {
						const int64_t source_slot = before_vertices[cell].find(after_vertices[flipped][slot]);
						REQUIRE(source_slot >= 0);
						if (!before_normals.is_empty() && !before_normals[cell].is_empty()) {
							const auto expected_normal = -before_normals[cell][source_slot];
							CHECK(after_normals[flipped][slot] == expected_normal);
						}
						if (!before_texture_map.is_empty() && !before_texture_map[cell].is_empty()) {
							CHECK(after_texture_map[flipped][slot] == before_texture_map[cell][source_slot]);
						}
					}
				}
				CHECK(order_changed);
				for (int64_t volume = 0; volume < before_geometry[dimension - 2].size(); volume++) {
					CHECK(after_geometry[dimension - 2][volume].size() == before_geometry[dimension - 2][volume].size() * 2);
					for (const int32_t cell : before_geometry[dimension - 2][volume]) {
						CHECK(after_geometry[dimension - 2][volume].has(cell + original_count));
					}
				}
				CHECK(mesh->is_poly_mesh_data_valid());
				const bool valid = mesh->is_mesh_data_valid();
				CHECK(valid);
			}
		}
	}
}

TEST_CASE("[ArrayPolyMesh4D] Double sided empty boundary levels are unchanged") {
	for (const bool idempotent : { false, true }) {
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		mesh->set_poly_cell_vertex_positions(PackedVector4Array{ Vector4() });
		Vector<Vector<PackedInt32Array>> poly;
		poly.resize(2);
		mesh->set_poly_cell_indices(poly);
		REQUIRE(mesh->is_mesh_data_valid());
		mesh->make_double_sided(idempotent);
		CHECK(mesh->get_poly_cell_indices() == poly);
		CHECK(mesh->get_all_poly_cell_normal_indices().is_empty());
		CHECK(mesh->get_all_poly_cell_texture_map_indices().is_empty());
		CHECK(mesh->get_poly_cell_normal_values().is_empty());
		CHECK(mesh->get_poly_cell_texture_map_values().is_empty());
		CHECK(mesh->get_poly_cell_boundary_pivot_overrides().is_empty());
		CHECK(mesh->get_simplex_cell_vertex_indices().is_empty());
		CHECK(mesh->get_normal_values().is_empty());
		CHECK(mesh->get_texture_map_values().is_empty());
		CHECK(mesh->is_mesh_data_valid());
	}
}

TEST_CASE("[ArrayPolyMesh4D] Explicit compaction removes unreferenced data and remaps bindings") {
	Ref<ArrayPolyMesh4D> mesh = make_tetrahedron_cell_mesh();
	const Vector4 normal_a = Vector4(1, 0, 0, 0);
	const Vector4 normal_b = Vector4(0, 0, 0, 1);
	const Vector3 texture_a = Vector3(1, 2, 3);
	const Vector3 texture_b = Vector3(4, 5, 6);
	const Vector2i vertex_key(3, 0);
	const Vector2i boundary_key(3, 3);
	mesh->set_poly_cell_dense_normals(vertex_key, Vector<PackedVector4Array>{ PackedVector4Array{ normal_a, normal_b, normal_a, normal_b } });
	mesh->set_poly_cell_dense_normals(boundary_key, Vector<PackedVector4Array>{ PackedVector4Array{ normal_b } });
	mesh->set_poly_cell_dense_texture_map(vertex_key, Vector<PackedVector3Array>{ PackedVector3Array{ texture_a, texture_b, texture_a, texture_b } });
	mesh->set_poly_cell_dense_texture_map(boundary_key, Vector<PackedVector3Array>{ PackedVector3Array{ texture_b } });
	CHECK(mesh->get_poly_cell_normal_values().size() == 2);
	CHECK(mesh->get_poly_cell_texture_map_values().size() == 2);
	// Remove one binding. The now-unreferenced values stay in the pools until compaction,
	// which must remap the surviving binding when pool slot zero is removed.
	mesh->set_poly_cell_dense_normals(vertex_key, Vector<PackedVector4Array>());
	mesh->set_poly_cell_dense_texture_map(vertex_key, Vector<PackedVector3Array>());
	CHECK(mesh->get_poly_cell_normal_values().size() == 2);
	CHECK(mesh->get_poly_cell_texture_map_values().size() == 2);
	mesh->compact_normal_values();
	mesh->compact_texture_map_values();
	REQUIRE(mesh->get_poly_cell_normal_values().size() == 1);
	REQUIRE(mesh->get_poly_cell_texture_map_values().size() == 1);
	CHECK(mesh->get_poly_cell_normal_values()[0] == normal_b);
	CHECK(mesh->get_poly_cell_texture_map_values()[0] == texture_b);
	CHECK(mesh->get_all_poly_cell_normal_indices()[boundary_key][0][0] == 0);
	CHECK(mesh->get_all_poly_cell_texture_map_indices()[boundary_key][0][0] == 0);
	CHECK(mesh->get_poly_cell_boundary_normals()[0] == normal_b);
}
} // namespace TestArrayPolyMesh4D
