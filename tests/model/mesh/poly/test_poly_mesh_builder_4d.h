#pragma once

#include "../../../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../../../../model/mesh/poly/poly_mesh_builder_4d.h"
#include "../../../../model/mesh/tetra/array_tetra_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestPolyMeshBuilder4D {
TEST_CASE("[PolyMeshBuilder4D] Reconstruct From Tetra Mesh") {
	SUBCASE("Single tetra reconstructs to valid poly mesh") {
		Ref<ArrayTetraMesh4D> tetra_mesh;
		tetra_mesh.instantiate();
		tetra_mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), true);

		Ref<ArrayPolyMesh4D> poly_mesh = PolyMeshBuilder4D::reconstruct_from_tetra_mesh(tetra_mesh);
		CHECK_MESSAGE(poly_mesh.is_valid(), "Reconstruct should return a valid mesh reference.");
		CHECK_MESSAGE(poly_mesh->is_poly_mesh_data_valid(), "A single tetra should reconstruct into valid poly mesh data.");

		const Vector<Vector<PackedInt32Array>> indices = poly_mesh->get_poly_cell_indices();
		CHECK_MESSAGE(indices.size() >= 2, "Reconstructed mesh should contain faces and cells.");
		CHECK_MESSAGE(indices[0].size() >= 4, "A reconstructed tetra cell should have at least four boundary faces.");
		CHECK_MESSAGE(indices[1].size() >= 1, "A reconstructed tetra should produce at least one 3D cell.");
	}

	SUBCASE("Disconnected coplanar triangle islands no longer collapse to empty face output") {
		Ref<ArrayTetraMesh4D> tetra_mesh;
		tetra_mesh.instantiate();
		// Two tetrahedra sharing only the pivot vertex. Their opposite faces are coplanar (z = 0, w = 0)
		// but disconnected, which exercises the coplanar-island split path.
		tetra_mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), true);
		tetra_mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(4, 0, 0, 0), Vector4(4, 1, 0, 0), Vector4(4, 0, 1, 0), true);

		Ref<ArrayPolyMesh4D> poly_mesh = PolyMeshBuilder4D::reconstruct_from_tetra_mesh(tetra_mesh);
		CHECK_MESSAGE(poly_mesh.is_valid(), "Reconstruct should return a mesh reference for disconnected coplanar islands.");

		const Vector<Vector<PackedInt32Array>> indices = poly_mesh->get_poly_cell_indices();
		CHECK_MESSAGE(indices.size() >= 2, "Reconstructed mesh should still contain faces and cells arrays.");
		CHECK_MESSAGE(indices[0].size() > 0, "Face list should not be empty after splitting disconnected coplanar islands.");
		CHECK_MESSAGE(indices[1].size() > 0, "Cell list should not be empty after splitting disconnected coplanar islands.");
		for (int64_t face_index = 0; face_index < indices[0].size(); face_index++) {
			CHECK_MESSAGE(indices[0][face_index].size() >= 3, "Each reconstructed face should have at least three edges.");
		}
	}

	SUBCASE("Duplicate tetrahedra produce invalid poly mesh data") {
		Ref<ArrayTetraMesh4D> tetra_mesh;
		tetra_mesh.instantiate();
		// Duplicate tetrahedra cancel all boundary triangles, which is malformed for poly reconstruction.
		tetra_mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), true);
		tetra_mesh->append_tetra_cell_points(Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), true);

		ERR_PRINT_OFF;
		Ref<ArrayPolyMesh4D> poly_mesh = PolyMeshBuilder4D::reconstruct_from_tetra_mesh(tetra_mesh);
		CHECK_MESSAGE(poly_mesh.is_valid(), "Reconstruct should still return a mesh reference for malformed input.");
		CHECK_MESSAGE(!poly_mesh->is_poly_mesh_data_valid(), "Duplicate tetra input should not reconstruct to valid poly mesh data.");
		ERR_PRINT_ON;
	}
}

TEST_CASE("[PolyMeshBuilder4D] Subdivide elements") {
	SUBCASE("Subdividing the boundary cells of a tesseract gives 8 sub-cubes per cell") {
		Ref<BoxPolyMesh4D> box;
		box.instantiate();
		box->set_size(Vector4(2, 2, 2, 2));
		Ref<ArrayPolyMesh4D> mesh = box->to_array_poly_mesh();
		const PackedVector4Array old_normals = mesh->get_poly_cell_boundary_normals();
		const PackedInt32Array new_pieces = PolyMeshBuilder4D::subdivide_elements(mesh, 3, PackedInt32Array());
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "The subdivided tesseract must have valid poly mesh data.");
		CHECK_MESSAGE(new_pieces.size() == 8 * 8, "Each of the 8 boundary cubes must subdivide into 8 sub-cubes.");
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		CHECK_MESSAGE(poly_cell_indices[0].size() == 24 * 4 + 8 * 12, "The faces must be the 96 face pieces plus 12 internal walls per cube.");
		CHECK_MESSAGE(poly_cell_indices[1].size() == 64, "The boundary level must contain exactly the sub-cubes.");
		CHECK_MESSAGE(poly_cell_indices[2].size() == 1, "The volumetric cell must be conformed, not subdivided.");
		CHECK_MESSAGE(poly_cell_indices[2][0].size() == 64, "The conformed volumetric cell must reference all sub-cubes.");
		CHECK_MESSAGE(mesh->get_poly_cell_vertices().size() == 16 + 32 + 24 + 8, "The subdivided tesseract must add edge midpoints, face centers, and cube centers.");
		// Each piece must inherit its parent's boundary normal, and the cell orientations must match.
		const PackedVector4Array new_normals = mesh->get_poly_cell_boundary_normals();
		REQUIRE(new_normals.size() == 64);
		for (int64_t parent = 0; parent < 8; parent++) {
			for (int64_t piece_num = 0; piece_num < 8; piece_num++) {
				CHECK_MESSAGE(new_normals[new_pieces[parent * 8 + piece_num]].is_equal_approx(old_normals[parent]), "Each sub-cube must inherit its parent's boundary normal.");
			}
		}
		Ref<ArrayPolyMesh4D> recalculated = mesh->duplicate();
		recalculated->set_poly_cell_boundary_normals(PackedVector4Array());
		recalculated->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_CELL_ORIENTATION_ONLY);
		const PackedVector4Array oriented_normals = recalculated->get_poly_cell_boundary_normals();
		for (int64_t i = 0; i < new_normals.size(); i++) {
			CHECK_MESSAGE(oriented_normals[i].is_equal_approx(new_normals[i]), "The cell orientations must reproduce the inherited normals.");
		}
		// The geometry is unchanged, so the signed distance must be unchanged. The center point
		// ties more candidate tetrahedra than the intended cap considers, which prints warnings,
		// but the distances remain exact here.
		mesh->populate_inverse_metric_cache();
		ERR_PRINT_OFF;
		CHECK_MESSAGE(mesh->get_signed_distance_to_mesh(Vector4(2, 0, 0, 0), nullptr, nullptr) == doctest::Approx(1.0), "The subdivided tesseract must have the same signed distances as before.");
		CHECK_MESSAGE(mesh->get_signed_distance_to_mesh(Vector4(0, 0, 0, 0), nullptr, nullptr) == doctest::Approx(-1.0), "The subdivided tesseract must have the same signed distances as before.");
		ERR_PRINT_ON;
	}
	SUBCASE("A pentachoron subdivides into 5 corner pentachora and a central rectified pentachoron") {
		// A solid 4D simplex: 5 vertices, 10 edges, 10 triangles, 5 tetrahedra, 1 volumetric cell.
		Ref<ArrayPolyMesh4D> mesh;
		mesh.instantiate();
		PackedVector4Array vertices = {
			Vector4(0, 0, 0, 0),
			Vector4(1, 0, 0, 0),
			Vector4(0, 1, 0, 0),
			Vector4(0, 0, 1, 0),
			Vector4(0, 0, 0, 1),
		};
		mesh->set_poly_cell_vertices(vertices);
		PackedInt32Array edge_indices;
		HashMap<int32_t, int32_t> edge_map;
		for (int32_t a = 0; a < 5; a++) {
			for (int32_t b = a + 1; b < 5; b++) {
				edge_map[a * 8 + b] = (int32_t)(edge_indices.size() / 2);
				edge_indices.append(a);
				edge_indices.append(b);
			}
		}
		mesh->set_edge_vertex_indices(edge_indices);
		Vector<PackedInt32Array> faces;
		HashMap<int32_t, int32_t> face_map;
		for (int32_t a = 0; a < 5; a++) {
			for (int32_t b = a + 1; b < 5; b++) {
				for (int32_t c = b + 1; c < 5; c++) {
					face_map[(a * 8 + b) * 8 + c] = (int32_t)faces.size();
					faces.append(PackedInt32Array{ edge_map[a * 8 + b], edge_map[b * 8 + c], edge_map[a * 8 + c] });
				}
			}
		}
		Vector<PackedInt32Array> cells;
		for (int32_t a = 0; a < 5; a++) {
			for (int32_t b = a + 1; b < 5; b++) {
				for (int32_t c = b + 1; c < 5; c++) {
					for (int32_t d = c + 1; d < 5; d++) {
						cells.append(PackedInt32Array{
								face_map[(a * 8 + b) * 8 + c],
								face_map[(a * 8 + b) * 8 + d],
								face_map[(a * 8 + c) * 8 + d],
								face_map[(b * 8 + c) * 8 + d] });
					}
				}
			}
		}
		Vector<PackedInt32Array> volumes;
		volumes.append(PackedInt32Array{ 0, 1, 2, 3, 4 });
		mesh->set_poly_cell_indices(Vector<Vector<PackedInt32Array>>{ faces, cells, volumes });
		mesh->calculate_boundary_normals(ArrayPolyMesh4D::COMPUTE_NORMALS_MODE_FORCE_OUTWARD_FIX_CELL_ORIENTATION);
		REQUIRE(mesh->is_poly_mesh_data_valid());
		const PackedInt32Array new_pieces = PolyMeshBuilder4D::subdivide_elements(mesh, 4, PackedInt32Array());
		CHECK_MESSAGE(mesh->is_poly_mesh_data_valid(), "The subdivided pentachoron must have valid poly mesh data.");
		CHECK_MESSAGE(new_pieces.size() == 6, "A pentachoron must subdivide into 5 corner pentachora and 1 central rectified pentachoron.");
		CHECK_MESSAGE(mesh->get_poly_cell_vertices().size() == 5 + 10, "The subdivided pentachoron must only add the 10 edge midpoints, with no center vertices.");
		const Vector<Vector<PackedInt32Array>> poly_cell_indices = mesh->get_poly_cell_indices();
		CHECK_MESSAGE(poly_cell_indices[1].size() == 5 * 5 + 5, "The subdivided pentachoron must have 25 boundary cell pieces and 5 interior cut cells.");
		CHECK_MESSAGE(poly_cell_indices[2].size() == 6, "The subdivided pentachoron must have 6 volumetric cells.");
		int64_t corner_count = 0;
		int64_t rectified_count = 0;
		for (const int32_t piece : new_pieces) {
			const int64_t member_count = poly_cell_indices[2][piece].size();
			if (member_count == 5) {
				corner_count++;
			} else if (member_count == 10) {
				rectified_count++;
			}
		}
		CHECK_MESSAGE(corner_count == 5, "The subdivided pentachoron must have 5 corner pentachora with 5 members each.");
		CHECK_MESSAGE(rectified_count == 1, "The central rectified pentachoron must have 10 members: 5 octahedra and 5 tetrahedra.");
	}
}

TEST_CASE("[SceneTree][PolyMeshBuilder4D] Subdivide a converted flat mesh and extrude it") {
	// Build a 3D quad mesh, convert it to a flat 4D mesh, subdivide its faces, then extrude it.
	Ref<ArrayMesh> quad_mesh;
	quad_mesh.instantiate();
	PackedVector3Array quad_vertices = { Vector3(0, 0, 0), Vector3(2, 0, 0), Vector3(2, 2, 0), Vector3(0, 2, 0) };
	PackedVector3Array quad_normals = { Vector3(0, 0, 1), Vector3(0, 0, 1), Vector3(0, 0, 1), Vector3(0, 0, 1) };
	PackedVector2Array quad_uvs = { Vector2(0, 0), Vector2(1, 0), Vector2(1, 1), Vector2(0, 1) };
	PackedInt32Array quad_indices = { 0, 2, 1, 0, 3, 2 }; // Godot 3D uses clockwise winding order.
	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = quad_vertices;
	arrays[Mesh::ARRAY_NORMAL] = quad_normals;
	arrays[Mesh::ARRAY_TEX_UV] = quad_uvs;
	arrays[Mesh::ARRAY_INDEX] = quad_indices;
	quad_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	Ref<ArrayPolyMesh4D> converted = PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(quad_mesh);
	REQUIRE(converted->is_poly_mesh_data_valid());
	const PackedInt32Array new_pieces = PolyMeshBuilder4D::subdivide_elements(converted, 2, PackedInt32Array());
	CHECK_MESSAGE(converted->is_poly_mesh_data_valid(), "The subdivided flat mesh must have valid poly mesh data.");
	CHECK_MESSAGE(new_pieces.size() == 2 * 4, "Each triangle must subdivide into 4 triangles.");
	// The flat mesh's per-face normals must be inherited by the pieces.
	const HashMap<Vector2i, Vector<PackedVector4Array>> all_normals = converted->get_all_poly_cell_normals();
	const Vector<PackedVector4Array> *face_normals = all_normals.getptr(PolyMesh4D::PER_FACE_KEY);
	REQUIRE_MESSAGE(face_normals != nullptr, "The per-face normals of a flat mesh must be preserved by subdivision.");
	REQUIRE((*face_normals)[0].size() == 8);
	const Vector4 pos_z = Vector4(0, 0, 1, 0);
	for (int64_t i = 0; i < 8; i++) {
		CHECK_MESSAGE((*face_normals)[0][i].is_equal_approx(pos_z), "Each face piece must inherit the +Z face normal.");
	}
	// The texture map was set from the UVs, which match half the vertex XY positions,
	// so interpolation must preserve that linear relationship at the new vertices.
	const HashMap<Vector2i, Vector<PackedVector3Array>> all_texture_maps = converted->get_all_poly_cell_texture_maps();
	const Vector<PackedVector3Array> *texture_maps = all_texture_maps.getptr(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE_MESSAGE(texture_maps != nullptr, "The face texture maps of a flat mesh must be preserved by subdivision.");
	const PackedVector4Array flat_vertices = converted->get_poly_cell_vertices();
	const Vector<PackedInt32Array> face_vertex_indices = converted->get_all_poly_cell_vertex_indices(2, false);
	for (int64_t face_index = 0; face_index < 8; face_index++) {
		const PackedVector3Array &face_texture_map = (*texture_maps)[face_index];
		REQUIRE(face_texture_map.size() == face_vertex_indices[face_index].size());
		for (int64_t vert_num = 0; vert_num < face_texture_map.size(); vert_num++) {
			const Vector4 vertex = flat_vertices[face_vertex_indices[face_index][vert_num]];
			CHECK_MESSAGE(face_texture_map[vert_num].x == doctest::Approx(vertex.x / 2.0), "The interpolated texture map must remain linear in the vertex positions.");
			CHECK_MESSAGE(face_texture_map[vert_num].y == doctest::Approx(vertex.y / 2.0), "The interpolated texture map must remain linear in the vertex positions.");
		}
	}
	// The subdivided flat mesh must extrude into a valid 4D mesh with the carried-over normals.
	Ref<ArrayPolyMesh4D> extruded = PolyMeshBuilder4D::extrude_linear(converted);
	CHECK_MESSAGE(extruded->is_poly_mesh_data_valid(), "The subdivided flat mesh must extrude into a valid 4D mesh.");
	const PackedVector4Array extruded_normals = extruded->get_poly_cell_boundary_normals();
	REQUIRE(extruded_normals.size() == 8);
	for (int64_t i = 0; i < 8; i++) {
		CHECK_MESSAGE(extruded_normals[i].is_equal_approx(pos_z), "The extruded cells must carry over the subdivided face normals.");
	}
}

} // namespace TestPolyMeshBuilder4D
