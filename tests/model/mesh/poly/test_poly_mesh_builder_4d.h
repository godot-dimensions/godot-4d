#pragma once

#include "../../../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../../../../model/mesh/poly/poly_mesh_builder_4d.h"
#include "../../../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../../../model/mesh/tetra/box_tetra_mesh_4d.h"

#include "scene/resources/3d/primitive_meshes.h"
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
		CHECK_MESSAGE(mesh->get_poly_cell_vertex_positions().size() == 16 + 32 + 24 + 8, "The subdivided tesseract must add edge midpoints, face centers, and cube centers.");
		// Every face must have its edges in a connected loop order, including internal walls.
		const PackedInt32Array all_edges = mesh->get_edge_indices();
		for (const PackedInt32Array &face : poly_cell_indices[0]) {
			for (int64_t i = 0; i < face.size(); i++) {
				const int32_t edge_a = face[i];
				const int32_t edge_b = face[(i + 1) % face.size()];
				const bool connected = all_edges[edge_a * 2] == all_edges[edge_b * 2] || all_edges[edge_a * 2] == all_edges[edge_b * 2 + 1] || all_edges[edge_a * 2 + 1] == all_edges[edge_b * 2] || all_edges[edge_a * 2 + 1] == all_edges[edge_b * 2 + 1];
				CHECK_MESSAGE(connected, "Every face of the subdivided tesseract must have its edges in a connected loop order.");
			}
		}
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
		mesh->set_poly_cell_vertex_positions(vertices);
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
		CHECK_MESSAGE(mesh->get_poly_cell_vertex_positions().size() == 5 + 10, "The subdivided pentachoron must only add the 10 edge midpoints, with no center vertices.");
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
	const Vector<PackedVector4Array> face_normals = converted->get_poly_cell_dense_normals(PolyMesh4D::PER_FACE_KEY);
	REQUIRE_MESSAGE(!face_normals.is_empty(), "The per-face normals of a flat mesh must be preserved by subdivision.");
	REQUIRE(face_normals[0].size() == 8);
	const Vector4 pos_z = Vector4(0, 0, 1, 0);
	for (int64_t i = 0; i < 8; i++) {
		CHECK_MESSAGE(face_normals[0][i].is_equal_approx(pos_z), "Each face piece must inherit the +Z face normal.");
	}
	// The texture map was set from the UVs, which match half the vertex XY positions,
	// so interpolation must preserve that linear relationship at the new vertices.
	const Vector<PackedVector3Array> texture_maps = converted->get_poly_cell_dense_texture_map(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE_MESSAGE(!texture_maps.is_empty(), "The face texture maps of a flat mesh must be preserved by subdivision.");
	const PackedVector4Array flat_vertices = converted->get_poly_cell_vertex_positions();
	const Vector<PackedInt32Array> face_vertex_indices = converted->get_all_poly_cell_vertex_indices(2, false);
	for (int64_t face_index = 0; face_index < 8; face_index++) {
		const PackedVector3Array &face_texture_map = texture_maps[face_index];
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

TEST_CASE("[SceneTree][PolyMeshBuilder4D] Extrude linear orients the caps along the extrusion vector") {
	// Build a 3D tetrahedron surface, convert it to 4D faces, and close it into a single 3D cell.
	Ref<ArrayMesh> tetra_mesh_3d;
	tetra_mesh_3d.instantiate();
	PackedVector3Array tetra_vertices = { Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) };
	PackedInt32Array tetra_indices = { 0, 1, 2, 0, 3, 1, 0, 2, 3, 1, 3, 2 }; // Godot 3D uses clockwise winding order.
	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = tetra_vertices;
	arrays[Mesh::ARRAY_INDEX] = tetra_indices;
	tetra_mesh_3d->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	Ref<ArrayPolyMesh4D> slab = PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(tetra_mesh_3d);
	REQUIRE(slab->is_poly_mesh_data_valid());
	slab->append_poly_cell(3, slab->make_single_cell_from_all_faces());
	REQUIRE(slab->is_poly_mesh_data_valid());
	REQUIRE(slab->get_poly_cell_indices().size() == 2);
	// Tilt the slab slightly out of the XYZ hyperplane and move it far from the origin along X. The two copies
	// of the cell become the caps of the extrusion, and must face away from each other along the extrusion
	// vector. Deciding this by "away from the origin" would fail here, since the X offset dominates.
	slab->transform_mesh(Transform4D(Basis4D::from_xw(0.2), Vector4(10, 0, 0, 0)));
	const Vector4 extrusion = Vector4(0, 0, 0, 1);
	Ref<ArrayPolyMesh4D> extruded = PolyMeshBuilder4D::extrude_linear(slab, extrusion);
	REQUIRE(extruded->is_poly_mesh_data_valid());
	const PackedVector4Array extruded_normals = extruded->get_poly_cell_boundary_normals();
	REQUIRE(extruded_normals.size() >= 2);
	CHECK_MESSAGE(extruded_normals[0].dot(extrusion) < 0.0, "The cap moved by the negative extrusion vector must face that way.");
	CHECK_MESSAGE(extruded_normals[1].dot(extrusion) > 0.0, "The cap moved by the positive extrusion vector must face that way.");
}

TEST_CASE("[SceneTree][PolyMeshBuilder4D] Extrude linear gives the side faces edge normals") {
	// A cube converted to 4D faces carries per-face normals from its winding, corner normals from its 3D normals,
	// and corner texture maps from its UVs. Extruding it along W sweeps each cube edge into a side face.
	Ref<BoxMesh> box_mesh;
	box_mesh.instantiate();
	box_mesh->set_size(Vector3(2, 2, 2));
	Ref<ArrayPolyMesh4D> flat = PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(box_mesh);
	REQUIRE(flat->is_poly_mesh_data_valid());
	const int64_t input_face_count = flat->get_poly_cell_indices()[0].size();
	const int64_t input_edge_count = flat->get_edge_indices().size() / 2;
	Ref<ArrayPolyMesh4D> extruded = PolyMeshBuilder4D::extrude_linear(flat, Vector4(0, 0, 0, 1));
	REQUIRE(extruded->is_poly_mesh_data_valid());
	const int64_t face_count = extruded->get_poly_cell_indices()[0].size();
	REQUIRE(face_count == input_face_count * 2 + input_edge_count);
	const Vector<PackedVector4Array> face_normals = extruded->get_poly_cell_dense_normals(PolyMesh4D::PER_FACE_KEY);
	REQUIRE(face_normals.size() == 1);
	REQUIRE_MESSAGE(face_normals[0].size() == face_count, "Every face of the extruded mesh must have a per-face normal.");
	const Vector<PackedVector4Array> corner_normals = extruded->get_poly_cell_dense_normals(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE_MESSAGE(corner_normals.size() == face_count, "Every face of the extruded mesh must have corner normals.");
	const Vector<PackedVector3Array> corner_texture_maps = extruded->get_poly_cell_dense_texture_map(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE_MESSAGE(corner_texture_maps.size() == face_count, "The corner texture map binding must cover every face, with the side faces unmapped.");
	const PackedVector4Array vertices = extruded->get_poly_cell_vertex_positions();
	const Vector<PackedInt32Array> face_vertices = extruded->get_all_poly_cell_vertex_indices(2, false);
	for (int64_t face_index = input_face_count * 2; face_index < face_count; face_index++) {
		// A side face's midpoint, projected back into XYZ, is the midpoint of the cube edge it was swept from.
		// For a cube centered at the origin, every edge normal points from the center through that midpoint.
		// This includes the triangulation diagonals, whose two faces are coplanar, so their sum is the face normal.
		const PackedInt32Array &side_face_vertices = face_vertices[face_index];
		Vector4 midpoint;
		for (const int32_t vertex_index : side_face_vertices) {
			midpoint += vertices[vertex_index];
		}
		midpoint /= side_face_vertices.size();
		midpoint.w = 0.0;
		const Vector4 expected = midpoint.normalized();
		CHECK_MESSAGE(face_normals[0][face_index].is_equal_approx(expected), "Each side face must take the edge normal of the cube edge it was swept from.");
		// The cube is flat shaded, so every corner normal of a side face equals its per-face normal. The corner
		// normals come from the 3D mesh's normal array, which Godot stores compressed, so allow for that error.
		const PackedVector4Array &side_corner_normals = corner_normals[face_index];
		REQUIRE(side_corner_normals.size() == side_face_vertices.size());
		for (const Vector4 &corner_normal : side_corner_normals) {
			CHECK_MESSAGE((corner_normal - expected).length() < 0.001, "Each corner of a side face must sum its vertex's corner normals in the adjacent faces.");
		}
		CHECK_MESSAGE(corner_texture_maps[face_index].is_empty(), "Side faces cannot be texture mapped by extrusion, so they must be left unmapped.");
	}
}

TEST_CASE("[SceneTree][PolyMeshBuilder4D] Extrude spin gives the swept faces edge normals") {
	// Two non-coplanar triangles sharing an edge, all at X >= 1 so that the spin moves every vertex. The per-face
	// normals come from the winding, and the corner normals and texture maps come from the supplied arrays.
	const Vector3 v0 = Vector3(1.5, 0, 0);
	const Vector3 v1 = Vector3(1.5, 1, 0);
	const Vector3 v2 = Vector3(1, 0.5, 0);
	const Vector3 v3 = Vector3(2, 0.5, 1);
	const Vector3 normal_a = Vector3(0, 0, 1);
	const Vector3 normal_b = Vector3(1, 0, -0.5).normalized();
	Ref<ArrayMesh> mesh_3d;
	mesh_3d.instantiate();
	PackedVector3Array vertices = { v0, v1, v2, v0, v3, v1 };
	PackedVector3Array normals = { normal_a, normal_a, normal_a, normal_b, normal_b, normal_b };
	PackedVector2Array uvs = { Vector2(0, 0), Vector2(0, 1), Vector2(1, 0), Vector2(0, 0), Vector2(1, 1), Vector2(0, 1) };
	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = vertices;
	arrays[Mesh::ARRAY_NORMAL] = normals;
	arrays[Mesh::ARRAY_TEX_UV] = uvs;
	mesh_3d->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	Ref<ArrayPolyMesh4D> flat = PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(mesh_3d);
	REQUIRE(flat->is_poly_mesh_data_valid());
	const PackedVector4Array input_vertices = flat->get_poly_cell_vertex_positions();
	REQUIRE(input_vertices.size() == 4);
	const Vector<PackedInt32Array> input_face_vertices = flat->get_all_poly_cell_vertex_indices(2, false);
	REQUIRE(input_face_vertices.size() == 2);
	const Vector<PackedVector4Array> input_face_normals = flat->get_poly_cell_dense_normals(PolyMesh4D::PER_FACE_KEY);
	REQUIRE(input_face_normals.size() == 1);
	const Vector<PackedVector4Array> input_corner_normals = flat->get_poly_cell_dense_normals(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE(input_corner_normals.size() == 2);
	const int steps = 4;
	const double radians_per_step = Math_TAU / steps;
	Ref<ArrayPolyMesh4D> spun = PolyMeshBuilder4D::extrude_spin_from_faces_xw(flat, steps);
	REQUIRE(spun->is_poly_mesh_data_valid());
	const PackedVector4Array spun_vertices = spun->get_poly_cell_vertex_positions();
	const Vector<PackedInt32Array> spun_face_vertices = spun->get_all_poly_cell_vertex_indices(2, false);
	const int64_t face_count = spun_face_vertices.size();
	REQUIRE(face_count == 2 * steps + 5 * steps); // Two faces and five edges, each copied or swept once per step.
	const Vector<PackedVector4Array> face_normals = spun->get_poly_cell_dense_normals(PolyMesh4D::PER_FACE_KEY);
	REQUIRE(face_normals.size() == 1);
	REQUIRE_MESSAGE(face_normals[0].size() == face_count, "Every face of the spun mesh must have a per-face normal.");
	const Vector<PackedVector4Array> corner_normals = spun->get_poly_cell_dense_normals(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE_MESSAGE(corner_normals.size() == face_count, "Every face of the spun mesh must have corner normals.");
	const Vector<PackedVector3Array> corner_texture_maps = spun->get_poly_cell_dense_texture_map(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE_MESSAGE(corner_texture_maps.size() == face_count, "The corner texture map binding must cover every face, with the swept faces unmapped.");
	// Every output vertex is a rotated copy of an input vertex at some step, which identifies each face's origin.
	auto identify_vertex = [&](const Vector4 &p_position, int32_t &r_input_vertex, int &r_step) -> bool {
		for (int step = 0; step < steps; step++) {
			const Basis4D rotation = Basis4D::from_xw(step * radians_per_step);
			for (int32_t input_vertex = 0; input_vertex < input_vertices.size(); input_vertex++) {
				if (rotation.xform(input_vertices[input_vertex]).is_equal_approx(p_position)) {
					r_input_vertex = input_vertex;
					r_step = step;
					return true;
				}
			}
		}
		return false;
	};
	auto input_faces_containing = [&](const int32_t p_vertex_a, const int32_t p_vertex_b) -> PackedInt32Array {
		PackedInt32Array faces;
		for (int32_t face_index = 0; face_index < input_face_vertices.size(); face_index++) {
			if (input_face_vertices[face_index].has(p_vertex_a) && input_face_vertices[face_index].has(p_vertex_b)) {
				faces.append(face_index);
			}
		}
		return faces;
	};
	for (int64_t face_index = 0; face_index < face_count; face_index++) {
		const PackedInt32Array &face_vertices = spun_face_vertices[face_index];
		PackedInt32Array corner_input_vertices;
		PackedInt32Array corner_steps;
		for (const int32_t vertex_index : face_vertices) {
			int32_t input_vertex = -1;
			int step = -1;
			REQUIRE_MESSAGE(identify_vertex(spun_vertices[vertex_index], input_vertex, step), "Every spun vertex must be a rotated copy of an input vertex.");
			corner_input_vertices.append(input_vertex);
			corner_steps.append(step);
		}
		if (face_index < 2 * steps) {
			// A rotated copy of an input face: all corners share one step, and its normal is the input normal rotated by that step.
			const int step = corner_steps[0];
			int32_t input_face = -1;
			for (int32_t candidate = 0; candidate < input_face_vertices.size(); candidate++) {
				if (input_face_vertices[candidate].size() == corner_input_vertices.size() && input_face_vertices[candidate].has(corner_input_vertices[0]) && input_face_vertices[candidate].has(corner_input_vertices[1]) && input_face_vertices[candidate].has(corner_input_vertices[2])) {
					input_face = candidate;
				}
			}
			REQUIRE(input_face != -1);
			const Vector4 expected = Basis4D::from_xw(step * radians_per_step).xform(input_face_normals[0][input_face]);
			CHECK_MESSAGE(face_normals[0][face_index].is_equal_approx(expected), "Each rotated copy of an input face must take the input face normal rotated by its step.");
			continue;
		}
		// A swept face: two input vertices (the swept edge) at two consecutive steps.
		const int32_t vertex_a = corner_input_vertices[0];
		int32_t vertex_b = -1;
		int base_step = -1;
		for (int64_t corner = 0; corner < corner_input_vertices.size(); corner++) {
			if (corner_input_vertices[corner] != vertex_a) {
				vertex_b = corner_input_vertices[corner];
			}
			if (corner_steps.has((corner_steps[corner] + 1) % steps)) {
				base_step = corner_steps[corner];
			}
		}
		REQUIRE(vertex_b != -1);
		REQUIRE(base_step != -1);
		const PackedInt32Array adjacent_faces = input_faces_containing(vertex_a, vertex_b);
		REQUIRE(!adjacent_faces.is_empty());
		Vector4 edge_normal;
		for (const int32_t adjacent_face : adjacent_faces) {
			edge_normal += input_face_normals[0][adjacent_face];
		}
		const Vector4 expected = Basis4D::from_xw((base_step + 0.5) * radians_per_step).xform(edge_normal.normalized());
		CHECK_MESSAGE(face_normals[0][face_index].is_equal_approx(expected), "Each swept face must take the edge normal of its input edge, rotated by the half step it spans.");
		// Corner normals follow the same rule at the corner's own step. They come from the 3D mesh's compressed normal array, so allow for that error.
		const PackedVector4Array &swept_corner_normals = corner_normals[face_index];
		REQUIRE(swept_corner_normals.size() == face_vertices.size());
		for (int64_t corner = 0; corner < face_vertices.size(); corner++) {
			Vector4 corner_normal;
			for (const int32_t adjacent_face : adjacent_faces) {
				const int64_t vert_in_face = input_face_vertices[adjacent_face].find(corner_input_vertices[corner]);
				REQUIRE(vert_in_face != -1);
				corner_normal += input_corner_normals[adjacent_face][vert_in_face];
			}
			const Vector4 expected_corner = Basis4D::from_xw(corner_steps[corner] * radians_per_step).xform(corner_normal.normalized());
			CHECK_MESSAGE((swept_corner_normals[corner] - expected_corner).length() < 0.001, "Each corner of a swept face must sum its vertex's corner normals in the adjacent faces, rotated by its step.");
		}
		CHECK_MESSAGE(corner_texture_maps[face_index].is_empty(), "Swept faces cannot be texture mapped by spinning, so they must be left unmapped.");
	}
}

TEST_CASE("[PolyMeshBuilder4D] Reconstruct a thin box without merging its parallel faces") {
	// The two large faces of a very thin cell are parallel planes a tiny distance apart. The coplanarity test
	// that groups triangles into faces must keep them apart, so the thin box reconstructs with exactly the same
	// structure as a box of regular proportions.
	Ref<BoxTetraMesh4D> regular_box;
	regular_box.instantiate();
	Ref<ArrayPolyMesh4D> regular = PolyMeshBuilder4D::reconstruct_from_tetra_mesh(regular_box);
	REQUIRE(regular->is_poly_mesh_data_valid());
	Ref<BoxTetraMesh4D> thin_box;
	thin_box.instantiate();
	thin_box->set_size(Vector4(0.5, 0.001, 10.0, 10.0));
	Ref<ArrayPolyMesh4D> thin = PolyMeshBuilder4D::reconstruct_from_tetra_mesh(thin_box);
	REQUIRE(thin->is_poly_mesh_data_valid());
	const Vector<Vector<PackedInt32Array>> regular_indices = regular->get_poly_cell_indices();
	const Vector<Vector<PackedInt32Array>> thin_indices = thin->get_poly_cell_indices();
	REQUIRE(regular_indices.size() >= 2);
	REQUIRE(thin_indices.size() == regular_indices.size());
	CHECK_MESSAGE(thin->get_edge_indices().size() == regular->get_edge_indices().size(), "The thin box must have the same edges as a regular box.");
	CHECK_MESSAGE(thin_indices[0].size() == regular_indices[0].size(), "The thin box must have the same faces as a regular box, so its parallel faces were not merged.");
	CHECK_MESSAGE(thin_indices[1].size() == regular_indices[1].size(), "The thin box must have the same cells as a regular box.");
}

} // namespace TestPolyMeshBuilder4D
