#pragma once

#include "../../../../math/math_4d.h"
#include "../../../../math/vector_4d.h"
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

TEST_CASE("[SceneTree][PolyMeshBuilder4D] Extrude linear carries corner data into the extruded cells") {
	// Each vertex gets its own normal and UV, so any cell corner that receives another vertex's data is detectable.
	Ref<ArrayMesh> quad_mesh;
	quad_mesh.instantiate();
	PackedVector3Array quad_vertices = { Vector3(0, 0, 0), Vector3(2, 0, 0), Vector3(0, 2, 0), Vector3(2, 2, 0) };
	PackedVector3Array quad_normals = { Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(-1, 0, 0) };
	PackedVector2Array quad_uvs = { Vector2(0, 0), Vector2(1, 0), Vector2(0, 1), Vector2(1, 1) };
	PackedInt32Array quad_indices = { 0, 2, 1, 3, 1, 2 };
	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = quad_vertices;
	arrays[Mesh::ARRAY_NORMAL] = quad_normals;
	arrays[Mesh::ARRAY_TEX_UV] = quad_uvs;
	arrays[Mesh::ARRAY_INDEX] = quad_indices;
	quad_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	Ref<ArrayPolyMesh4D> flat = PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(quad_mesh);
	REQUIRE(flat->is_poly_mesh_data_valid());
	const PackedVector4Array input_vertices = flat->get_poly_cell_vertex_positions();
	const Vector<PackedInt32Array> input_face_vertices = flat->get_all_poly_cell_vertex_indices(2, false);
	const Vector<PackedVector4Array> input_corner_normals = flat->get_poly_cell_dense_normals(PolyMesh4D::FACE_TO_VERT_KEY);
	const Vector<PackedVector3Array> input_corner_texture_maps = flat->get_poly_cell_dense_texture_map(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE(input_face_vertices.size() == 2);
	REQUIRE(input_corner_normals.size() == 2);
	REQUIRE(input_corner_texture_maps.size() == 2);
	Ref<ArrayPolyMesh4D> extruded = PolyMeshBuilder4D::extrude_linear(flat, Vector4(0, 0, 0, 1));
	REQUIRE(extruded->is_poly_mesh_data_valid());
	const PackedVector4Array vertices = extruded->get_poly_cell_vertex_positions();
	const Vector<PackedInt32Array> cell_vertices = extruded->get_all_poly_cell_vertex_indices(3, false);
	REQUIRE_MESSAGE(cell_vertices.size() == 2, "Each input face must extrude into one cell.");
	const Vector<PackedVector4Array> cell_corner_normals = extruded->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
	const Vector<PackedVector3Array> cell_corner_texture_maps = extruded->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
	REQUIRE_MESSAGE(cell_corner_normals.size() == 2, "Every extruded cell must have corner normals.");
	REQUIRE_MESSAGE(cell_corner_texture_maps.size() == 2, "Every extruded cell must have corner texture maps.");
	for (int64_t cell_index = 0; cell_index < 2; cell_index++) {
		const PackedInt32Array &corners = cell_vertices[cell_index];
		REQUIRE(corners.size() == 6);
		REQUIRE(cell_corner_normals[cell_index].size() == 6);
		REQUIRE(cell_corner_texture_maps[cell_index].size() == 6);
		// Each corner is a copy of an input vertex, on the negative or positive W side of the extrusion.
		PackedInt32Array corner_input_vertices;
		for (const int32_t vertex_index : corners) {
			Vector4 flattened = vertices[vertex_index];
			flattened.w = 0.0;
			const int64_t input_vertex = input_vertices.find(flattened);
			REQUIRE_MESSAGE(input_vertex != -1, "Every extruded vertex must be a copy of an input vertex.");
			corner_input_vertices.append(input_vertex);
		}
		// The cell is a prism over the one input face that has all of its corners' input vertices.
		int32_t input_face = -1;
		for (int32_t candidate = 0; candidate < input_face_vertices.size(); candidate++) {
			bool has_all = true;
			for (const int32_t input_vertex : corner_input_vertices) {
				has_all = has_all && input_face_vertices[candidate].has(input_vertex);
			}
			if (has_all) {
				input_face = candidate;
			}
		}
		REQUIRE(input_face != -1);
		for (int64_t corner = 0; corner < corners.size(); corner++) {
			const int64_t vert_in_face = input_face_vertices[input_face].find(corner_input_vertices[corner]);
			// The second copy of the input faces, on the positive W side, has its texture maps offset by 1 in Z.
			const real_t expected_z_offset = vertices[corners[corner]].w > (real_t)0.0 ? (real_t)1.0 : (real_t)0.0;
			CHECK_MESSAGE(cell_corner_normals[cell_index][corner].is_equal_approx(input_corner_normals[input_face][vert_in_face]), "Each cell corner must take its vertex's corner normal from the input face.");
			CHECK_MESSAGE(cell_corner_texture_maps[cell_index][corner].is_equal_approx(input_corner_texture_maps[input_face][vert_in_face] + Vector3(0, 0, expected_z_offset)), "Each cell corner must take its vertex's texture map from the copy of the input face it lies on.");
		}
	}
}

TEST_CASE("[SceneTree][PolyMeshBuilder4D] Extrude spin carries corner data into the swept cells") {
	// One triangle with a vertex on the spin axis (X = 0, W = 0), which every step's copy of the triangle shares.
	// Each vertex gets its own normal and UV, so any cell corner that receives another vertex's data is detectable.
	Ref<ArrayMesh> mesh_3d;
	mesh_3d.instantiate();
	PackedVector3Array vertices_3d = { Vector3(0, 0.5, 0), Vector3(1.5, 0, 0), Vector3(1.5, 1, 0.5) };
	PackedVector3Array normals_3d = { Vector3(0.6, 0, 0.8), Vector3(0, 0, 1), Vector3(0.8, 0.6, 0) };
	PackedVector2Array uvs_3d = { Vector2(0, 0), Vector2(1, 0), Vector2(0, 1) };
	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = vertices_3d;
	arrays[Mesh::ARRAY_NORMAL] = normals_3d;
	arrays[Mesh::ARRAY_TEX_UV] = uvs_3d;
	mesh_3d->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	Ref<ArrayPolyMesh4D> flat = PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(mesh_3d);
	REQUIRE(flat->is_poly_mesh_data_valid());
	const PackedVector4Array input_vertices = flat->get_poly_cell_vertex_positions();
	REQUIRE(input_vertices.size() == 3);
	const PackedInt32Array input_face_vertices = flat->get_all_poly_cell_vertex_indices(2, false)[0];
	const PackedVector4Array input_corner_normals = flat->get_poly_cell_dense_normals(PolyMesh4D::FACE_TO_VERT_KEY)[0];
	const PackedVector3Array input_corner_texture_maps = flat->get_poly_cell_dense_texture_map(PolyMesh4D::FACE_TO_VERT_KEY)[0];
	REQUIRE(input_corner_normals.size() == 3);
	REQUIRE(input_corner_texture_maps.size() == 3);
	const int32_t axis_vertex = input_vertices.find(Vector4(0, 0.5, 0, 0));
	REQUIRE(axis_vertex != -1);
	const int steps = 4;
	const double radians_per_step = Math_TAU / steps;
	Ref<ArrayPolyMesh4D> spun = PolyMeshBuilder4D::extrude_spin_from_faces_xw(flat, steps);
	REQUIRE(spun->is_poly_mesh_data_valid());
	const PackedVector4Array spun_vertices = spun->get_poly_cell_vertex_positions();
	const Vector<PackedInt32Array> cell_vertices = spun->get_all_poly_cell_vertex_indices(3, false);
	REQUIRE_MESSAGE(cell_vertices.size() == steps, "The triangle must sweep into one cell per step.");
	const Vector<PackedVector4Array> cell_corner_normals = spun->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
	const Vector<PackedVector3Array> cell_corner_texture_maps = spun->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
	REQUIRE_MESSAGE(cell_corner_normals.size() == steps, "Every swept cell must have corner normals.");
	REQUIRE_MESSAGE(cell_corner_texture_maps.size() == steps, "Every swept cell must have corner texture maps.");
	PackedInt32Array seen_base_steps;
	for (int64_t cell_index = 0; cell_index < steps; cell_index++) {
		const PackedInt32Array &corners = cell_vertices[cell_index];
		REQUIRE(cell_corner_normals[cell_index].size() == corners.size());
		REQUIRE(cell_corner_texture_maps[cell_index].size() == corners.size());
		// Every corner off the axis is a rotated copy of an input vertex at one of two consecutive steps.
		PackedInt32Array corner_input_vertices;
		PackedInt32Array corner_steps;
		for (const int32_t vertex_index : corners) {
			int32_t found_vertex = -1;
			int found_step = -1;
			for (int step = 0; step < steps && found_vertex == -1; step++) {
				const Basis4D rotation = Basis4D::from_xw(step * radians_per_step);
				for (int32_t input_vertex = 0; input_vertex < input_vertices.size(); input_vertex++) {
					if (rotation.xform(input_vertices[input_vertex]).is_equal_approx(spun_vertices[vertex_index])) {
						found_vertex = input_vertex;
						found_step = step;
						break;
					}
				}
			}
			REQUIRE_MESSAGE(found_vertex != -1, "Every spun vertex must be a rotated copy of an input vertex.");
			corner_input_vertices.append(found_vertex);
			corner_steps.append(found_vertex == axis_vertex ? -1 : found_step);
		}
		int base_step = -1;
		for (const int32_t step : corner_steps) {
			if (step != -1 && corner_steps.has((step + 1) % steps)) {
				base_step = step;
			}
		}
		REQUIRE(base_step != -1);
		seen_base_steps.append(base_step);
		for (int64_t corner = 0; corner < corners.size(); corner++) {
			const int64_t vert_in_face = input_face_vertices.find(corner_input_vertices[corner]);
			REQUIRE(vert_in_face != -1);
			const Vector4 input_normal = input_corner_normals[vert_in_face];
			const Vector3 input_texture_map = input_corner_texture_maps[vert_in_face];
			Vector4 expected_normal;
			real_t expected_z;
			if (corner_steps[corner] == -1) {
				// The axis vertex is shared by both copies of the triangle, so it averages their values. Averaging the
				// two rotated normals shortens them, so the result must be renormalized.
				expected_normal = (Basis4D::from_xw(base_step * radians_per_step).xform(input_normal) + Basis4D::from_xw((base_step + 1) * radians_per_step).xform(input_normal)).normalized();
				expected_z = (real_t)((base_step + 0.5) / steps);
			} else {
				expected_normal = Basis4D::from_xw(corner_steps[corner] * radians_per_step).xform(input_normal).normalized();
				// The copy of the triangle at the next step is one step further along Z, even when it wraps to step 0.
				expected_z = (base_step + (corner_steps[corner] == base_step ? 0 : 1)) / (real_t)steps;
			}
			CHECK_MESSAGE((cell_corner_normals[cell_index][corner] - expected_normal).length() < 0.0001, "Each cell corner must take its vertex's corner normal, rotated by its step.");
			CHECK_MESSAGE((cell_corner_texture_maps[cell_index][corner] - (input_texture_map + Vector3(0, 0, expected_z))).length() < 0.0001, "Each cell corner must take its vertex's texture map, offset in Z by its step.");
		}
	}
	for (int step = 0; step < steps; step++) {
		CHECK_MESSAGE(seen_base_steps.has(step), "Every step, including the last one that wraps around to the first, must sweep a cell.");
	}
}

TEST_CASE("[SceneTree][PolyMeshBuilder4D] Subdivide interpolates corner normals and texture maps") {
	// Corner values set from an affine function of the vertex position stay affine under subdivision, since every new
	// vertex is the centroid of the vertices it averages. The normals are not unit length, so every subdivided corner,
	// including those on the original vertices, must be normalized.
	auto normal_at = [](const Vector4 &p_position) -> Vector4 {
		return Vector4(1.0 + p_position.x, 2.0 + p_position.y, 3.0 + p_position.z, 4.0 + p_position.w);
	};
	auto texture_map_at = [](const Vector4 &p_position) -> Vector3 {
		return Vector3(p_position.x * 0.5 + 0.5, p_position.y * 0.5 + 0.5, (p_position.z + p_position.w) * 0.25 + 0.5);
	};
	SUBCASE("Boundary cell corners of a tesseract") {
		Ref<BoxPolyMesh4D> box;
		box.instantiate();
		box->set_size(Vector4(2, 2, 2, 2));
		Ref<ArrayPolyMesh4D> mesh = box->to_array_poly_mesh();
		const PackedVector4Array old_vertices = mesh->get_poly_cell_vertex_positions();
		const Vector<PackedInt32Array> old_cell_vertices = mesh->get_all_poly_cell_vertex_indices(3, false);
		Vector<PackedVector4Array> old_cell_normals;
		Vector<PackedVector3Array> old_cell_texture_maps;
		for (const PackedInt32Array &corners : old_cell_vertices) {
			PackedVector4Array cell_normals;
			PackedVector3Array cell_texture_maps;
			for (const int32_t vertex_index : corners) {
				cell_normals.append(normal_at(old_vertices[vertex_index]));
				cell_texture_maps.append(texture_map_at(old_vertices[vertex_index]));
			}
			old_cell_normals.append(cell_normals);
			old_cell_texture_maps.append(cell_texture_maps);
		}
		mesh->set_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY, old_cell_normals);
		mesh->set_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY, old_cell_texture_maps);
		REQUIRE(mesh->is_poly_mesh_data_valid());
		PolyMeshBuilder4D::subdivide_elements(mesh, 3, PackedInt32Array());
		REQUIRE(mesh->is_poly_mesh_data_valid());
		const PackedVector4Array vertices = mesh->get_poly_cell_vertex_positions();
		const Vector<PackedInt32Array> cell_vertices = mesh->get_all_poly_cell_vertex_indices(3, false);
		const Vector<PackedVector4Array> cell_normals = mesh->get_poly_cell_dense_normals(PolyMesh4D::CELL_TO_VERT_KEY);
		const Vector<PackedVector3Array> cell_texture_maps = mesh->get_poly_cell_dense_texture_map(PolyMesh4D::CELL_TO_VERT_KEY);
		REQUIRE(cell_vertices.size() == 64);
		REQUIRE(cell_normals.size() == 64);
		REQUIRE(cell_texture_maps.size() == 64);
		for (int64_t cell_index = 0; cell_index < 64; cell_index++) {
			const PackedInt32Array &corners = cell_vertices[cell_index];
			REQUIRE(cell_normals[cell_index].size() == corners.size());
			REQUIRE(cell_texture_maps[cell_index].size() == corners.size());
			for (int64_t corner = 0; corner < corners.size(); corner++) {
				const Vector4 position = vertices[corners[corner]];
				CHECK_MESSAGE(cell_normals[cell_index][corner].is_equal_approx(normal_at(position).normalized()), "Each cell corner normal must be the normalized interpolation of its parent cell's corner normals.");
				CHECK_MESSAGE(cell_texture_maps[cell_index][corner].is_equal_approx(texture_map_at(position)), "Each cell corner texture map must be the interpolation of its parent cell's corner texture maps.");
			}
		}
	}
	SUBCASE("Face corners of a flat mesh") {
		Ref<ArrayMesh> quad_mesh;
		quad_mesh.instantiate();
		PackedVector3Array quad_vertices = { Vector3(0, 0, 0), Vector3(2, 0, 0), Vector3(2, 2, 0), Vector3(0, 2, 0) };
		PackedInt32Array quad_indices = { 0, 2, 1, 0, 3, 2 };
		Array arrays;
		arrays.resize(Mesh::ARRAY_MAX);
		arrays[Mesh::ARRAY_VERTEX] = quad_vertices;
		arrays[Mesh::ARRAY_INDEX] = quad_indices;
		quad_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
		Ref<ArrayPolyMesh4D> mesh = PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(quad_mesh);
		REQUIRE(mesh->is_poly_mesh_data_valid());
		const PackedVector4Array old_vertices = mesh->get_poly_cell_vertex_positions();
		const Vector<PackedInt32Array> old_face_vertices = mesh->get_all_poly_cell_vertex_indices(2, false);
		Vector<PackedVector4Array> old_face_normals;
		for (const PackedInt32Array &corners : old_face_vertices) {
			PackedVector4Array face_normals;
			for (const int32_t vertex_index : corners) {
				face_normals.append(normal_at(old_vertices[vertex_index]));
			}
			old_face_normals.append(face_normals);
		}
		mesh->set_poly_cell_dense_normals(PolyMesh4D::FACE_TO_VERT_KEY, old_face_normals);
		REQUIRE(mesh->is_poly_mesh_data_valid());
		PolyMeshBuilder4D::subdivide_elements(mesh, 2, PackedInt32Array());
		REQUIRE(mesh->is_poly_mesh_data_valid());
		const PackedVector4Array vertices = mesh->get_poly_cell_vertex_positions();
		const Vector<PackedInt32Array> face_vertices = mesh->get_all_poly_cell_vertex_indices(2, false);
		const Vector<PackedVector4Array> face_normals = mesh->get_poly_cell_dense_normals(PolyMesh4D::FACE_TO_VERT_KEY);
		REQUIRE(face_vertices.size() == 2 * 4);
		REQUIRE(face_normals.size() == 2 * 4);
		for (int64_t face_index = 0; face_index < face_vertices.size(); face_index++) {
			const PackedInt32Array &corners = face_vertices[face_index];
			REQUIRE(face_normals[face_index].size() == corners.size());
			for (int64_t corner = 0; corner < corners.size(); corner++) {
				CHECK_MESSAGE(face_normals[face_index][corner].is_equal_approx(normal_at(vertices[corners[corner]]).normalized()), "Each face corner normal must be the normalized interpolation of its parent face's corner normals.");
			}
		}
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

TEST_CASE("[SceneTree][PolyMeshBuilder4D] Convert a 3D mesh keeps corner data on the right vertices") {
	// Each vertex gets its own normal and UV, so any corner that receives another vertex's data is detectable.
	// The second triangle starts at vertex 3 and continues to vertex 2, so its first edge is stored as (2, 3)
	// and the mesh reads that face's corners as 2, 3, 1 rather than in the triangle's listed order.
	Ref<ArrayMesh> quad_mesh;
	quad_mesh.instantiate();
	PackedVector3Array quad_vertices = { Vector3(0, 0, 0), Vector3(2, 0, 0), Vector3(0, 2, 0), Vector3(2, 2, 0) };
	PackedVector3Array quad_normals = { Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(-1, 0, 0) };
	PackedVector2Array quad_uvs = { Vector2(0, 0), Vector2(1, 0), Vector2(0, 1), Vector2(1, 1) };
	PackedInt32Array quad_indices = { 0, 2, 1, 3, 1, 2 };
	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = quad_vertices;
	arrays[Mesh::ARRAY_NORMAL] = quad_normals;
	arrays[Mesh::ARRAY_TEX_UV] = quad_uvs;
	arrays[Mesh::ARRAY_INDEX] = quad_indices;
	quad_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	Ref<ArrayPolyMesh4D> converted = PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(quad_mesh);
	REQUIRE(converted->is_mesh_data_valid());
	const PackedVector4Array positions = converted->get_poly_cell_vertex_positions();
	REQUIRE(positions.size() == 4);
	const Vector<PackedInt32Array> face_corners = converted->get_all_poly_cell_vertex_indices(2, false);
	const Vector<PackedVector4Array> corner_normals = converted->get_poly_cell_dense_normals(PolyMesh4D::FACE_TO_VERT_KEY);
	const Vector<PackedVector3Array> corner_uvs = converted->get_poly_cell_dense_texture_map(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE(face_corners.size() == 2);
	REQUIRE(corner_normals.size() == 2);
	REQUIRE(corner_uvs.size() == 2);
	for (int64_t face_index = 0; face_index < 2; face_index++) {
		REQUIRE(face_corners[face_index].size() == 3);
		REQUIRE(corner_normals[face_index].size() == 3);
		REQUIRE(corner_uvs[face_index].size() == 3);
		for (int64_t corner = 0; corner < 3; corner++) {
			// The converted vertices keep the 3D mesh's order, so the corner's vertex index is its 3D index.
			const int32_t vertex_index = face_corners[face_index][corner];
			REQUIRE(Vector4D::from_3d(quad_vertices[vertex_index]).is_equal_approx(positions[vertex_index]));
			// The 3D mesh compresses its normals, so allow for that error.
			CHECK_MESSAGE((corner_normals[face_index][corner] - Vector4D::from_3d(quad_normals[vertex_index])).length() < 0.001, "Each corner must carry its own vertex's normal.");
			CHECK_MESSAGE(corner_uvs[face_index][corner].is_equal_approx(Vector3(quad_uvs[vertex_index].x, quad_uvs[vertex_index].y, 0)), "Each corner must carry its own vertex's UV.");
		}
	}
}

TEST_CASE("[SceneTree][PolyMeshBuilder4D] Merge coplanar faces") {
	// A cube from a 3D mesh arrives as 12 triangles with 18 edges. Merging must give 6 quads and 12 edges, keep the
	// per-face and corner normals, and a single cell made from those faces must keep its face count and normal.
	Ref<BoxMesh> box_mesh;
	box_mesh.instantiate();
	box_mesh->set_size(Vector3(2, 2, 2));
	Ref<ArrayPolyMesh4D> cube = PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(box_mesh);
	REQUIRE(cube->is_poly_mesh_data_valid());
	REQUIRE(cube->get_poly_cell_indices()[0].size() == 12);
	REQUIRE(cube->get_edge_indices().size() == 18 * 2);
	// Build the single cell so that its first two faces share an edge but are not coplanar. The canonical span of
	// two coplanar triangles is degenerate and gives a zero normal, which would make the orientation check vacuous.
	PackedInt32Array cell = cube->make_single_cell_from_all_faces();
	{
		const Vector<PackedInt32Array> face_edges = cube->get_poly_cell_indices()[0];
		const Vector<PackedVector4Array> per_face_normals = cube->get_poly_cell_dense_normals(PolyMesh4D::PER_FACE_KEY);
		REQUIRE(per_face_normals.size() == 1);
		for (int64_t i = 1; i < cell.size(); i++) {
			int64_t index_in_first = 0;
			int64_t index_in_candidate = 0;
			const bool shares_edge = Math4D::find_common_int32(face_edges[cell[0]], face_edges[cell[i]], index_in_first, index_in_candidate) != INT32_MIN;
			if (shares_edge && !per_face_normals[0][cell[i]].is_equal_approx(per_face_normals[0][cell[0]])) {
				const int32_t temp = cell[1];
				cell.set(1, cell[i]);
				cell.set(i, temp);
				break;
			}
		}
	}
	cube->append_poly_cell(3, cell);
	REQUIRE(cube->is_mesh_data_valid());
	// Also bind a value to each of the cell's faces, using that face's normal, to check that bindings of cells to
	// their faces follow the merge even though the faces they list are replaced.
	{
		const PackedInt32Array cell_faces = cube->get_all_poly_cell_poly_indices(3, 2)[0];
		const PackedVector4Array per_face_normals = cube->get_poly_cell_dense_normals(PolyMesh4D::PER_FACE_KEY)[0];
		PackedVector4Array cell_face_values;
		for (const int32_t face_index : cell_faces) {
			cell_face_values.append(per_face_normals[face_index]);
		}
		cube->set_poly_cell_dense_normals(Vector2i(3, 2), Vector<PackedVector4Array>{ cell_face_values });
		REQUIRE(cube->is_mesh_data_valid());
	}
	cube->calculate_boundary_normals();
	REQUIRE(cube->get_poly_cell_boundary_normals().size() == 1);
	const Vector4 normal_before = cube->get_poly_cell_boundary_normals()[0];
	REQUIRE_MESSAGE(!normal_before.is_zero_approx(), "The cell must start with a well-defined orientation for the check to mean anything.");
	const int64_t merges = PolyMeshBuilder4D::merge_coplanar_faces(cube);
	CHECK_MESSAGE(merges == 6, "Each pair of triangles on a cube face must merge once.");
	REQUIRE(cube->is_mesh_data_valid());
	const Vector<Vector<PackedInt32Array>> indices = cube->get_poly_cell_indices();
	CHECK_MESSAGE(indices[0].size() == 6, "The cube must end up with 6 faces.");
	CHECK_MESSAGE(cube->get_edge_indices().size() == 12 * 2, "The 6 diagonals must be deleted, leaving the 12 cube edges.");
	for (const PackedInt32Array &face : indices[0]) {
		CHECK_MESSAGE(face.size() == 4, "Each merged face must be a quad.");
	}
	CHECK_MESSAGE(indices[1][0].size() == 6, "The cell must reference each merged face once.");
	REQUIRE(cube->get_poly_cell_boundary_normals().size() == 1);
	CHECK_MESSAGE(cube->get_poly_cell_boundary_normals()[0].is_equal_approx(normal_before), "The cell's orientation and normal must survive the merge.");
	// Flat shading: every resampled corner normal must match its face's normal. The corner normals come from the
	// 3D mesh's compressed normal array, so allow for that error.
	const Vector<PackedVector4Array> face_normals = cube->get_poly_cell_dense_normals(PolyMesh4D::PER_FACE_KEY);
	const Vector<PackedVector4Array> corner_normals = cube->get_poly_cell_dense_normals(PolyMesh4D::FACE_TO_VERT_KEY);
	REQUIRE(face_normals.size() == 1);
	REQUIRE(face_normals[0].size() == 6);
	REQUIRE(corner_normals.size() == 6);
	for (int64_t face_index = 0; face_index < 6; face_index++) {
		REQUIRE(corner_normals[face_index].size() == 4);
		for (const Vector4 &corner_normal : corner_normals[face_index]) {
			CHECK_MESSAGE(corner_normal.dot(face_normals[0][face_index]) > 0.999, "Each resampled corner normal must match its merged face's normal.");
		}
	}
	// The cell-to-faces binding must now list one value per merged face, each being that merged face's normal.
	{
		const Vector<PackedVector4Array> cell_face_values = cube->get_poly_cell_dense_normals(Vector2i(3, 2));
		REQUIRE(cell_face_values.size() == 1);
		const PackedInt32Array cell_faces = cube->get_all_poly_cell_poly_indices(3, 2)[0];
		REQUIRE(cell_face_values[0].size() == cell_faces.size());
		REQUIRE(cell_faces.size() == 6);
		for (int64_t i = 0; i < cell_faces.size(); i++) {
			CHECK_MESSAGE(cell_face_values[0][i].dot(face_normals[0][cell_faces[i]]) > 0.999, "Each merged face's value in the cell-to-faces binding must come from one of the faces it absorbed.");
		}
	}
	// A quad bent along its diagonal is two non-coplanar triangles, which must not merge.
	Ref<ArrayMesh> bent_mesh;
	bent_mesh.instantiate();
	PackedVector3Array bent_vertices = { Vector3(0, 0, 0), Vector3(2, 0, 0), Vector3(2, 2, 0), Vector3(0, 2, 1) };
	PackedInt32Array bent_indices = { 0, 2, 1, 0, 3, 2 };
	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = bent_vertices;
	arrays[Mesh::ARRAY_INDEX] = bent_indices;
	bent_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	Ref<ArrayPolyMesh4D> bent = PolyMeshBuilder4D::convert_mesh_3d_to_4d_faces_only(bent_mesh);
	CHECK_MESSAGE(PolyMeshBuilder4D::merge_coplanar_faces(bent) == 0, "Non-coplanar triangles must not merge.");
	CHECK(bent->get_poly_cell_indices()[0].size() == 2);
	// A 2 by 1 strip of two unit squares. Vertices 1 and 4 are the midpoints of the long sides, which stay in the
	// merged face as straight continuations. The merged face must not start its edge list at one of them.
	const PackedVector4Array strip_positions = { Vector4(0, 0, 0, 0), Vector4(1, 0, 0, 0), Vector4(2, 0, 0, 0), Vector4(2, 1, 0, 0), Vector4(1, 1, 0, 0), Vector4(0, 1, 0, 0) };
	auto make_strip = [&strip_positions](const Vector<PackedInt32Array> &p_face_vertex_loops) -> Ref<ArrayPolyMesh4D> {
		Ref<ArrayPolyMesh4D> strip;
		strip.instantiate();
		strip->append_vertices(strip_positions, false);
		for (const PackedInt32Array &loop : p_face_vertex_loops) {
			PackedInt32Array face;
			for (int64_t i = 0; i < loop.size(); i++) {
				face.append((int32_t)strip->append_edge_indices(loop[i], loop[(i + 1) % loop.size()]));
			}
			strip->append_poly_cell(2, face);
		}
		return strip;
	};
	Ref<ArrayPolyMesh4D> strip = make_strip({ { 0, 1, 4 }, { 0, 4, 5 }, { 1, 2, 3 }, { 1, 3, 4 } });
	REQUIRE(strip->is_mesh_data_valid());
	CHECK_MESSAGE(PolyMeshBuilder4D::merge_coplanar_faces(strip) == 3, "Four coplanar triangles must merge into one face.");
	REQUIRE(strip->is_mesh_data_valid());
	REQUIRE(strip->get_poly_cell_indices()[0].size() == 1);
	const PackedInt32Array strip_face = strip->get_poly_cell_indices()[0][0];
	CHECK_MESSAGE(strip_face.size() == 6, "The merged face must keep the midpoint vertices and their edges.");
	CHECK_MESSAGE(strip->get_edge_indices().size() == 6 * 2, "The shared diagonal and middle edges must be deleted.");
	{
		const PackedInt32Array edges = strip->get_edge_indices();
		int64_t index_in_first = 0;
		int64_t index_in_second = 0;
		const PackedInt32Array first_edge = { edges[strip_face[0] * 2], edges[strip_face[0] * 2 + 1] };
		const PackedInt32Array second_edge = { edges[strip_face[1] * 2], edges[strip_face[1] * 2 + 1] };
		const int32_t span_vertex = Math4D::find_common_int32(first_edge, second_edge, index_in_first, index_in_second);
		REQUIRE(span_vertex != INT32_MIN);
		CHECK_MESSAGE(strip->get_poly_cell_vertex_positions()[span_vertex].x != 1.0, "The first two edges of the merged face must meet at a corner, not at a straight continuation.");
	}
	// The same strip with the left square as a quad whose edges are stored out of loop order. The mesh is valid,
	// but merging must refuse it instead of producing a bad merge.
	Ref<ArrayPolyMesh4D> unordered = make_strip({ { 1, 2, 3 }, { 1, 3, 4 } });
	{
		const int32_t e01 = (int32_t)unordered->append_edge_indices(0, 1);
		const int32_t e14 = (int32_t)unordered->append_edge_indices(1, 4);
		const int32_t e45 = (int32_t)unordered->append_edge_indices(4, 5);
		const int32_t e50 = (int32_t)unordered->append_edge_indices(5, 0);
		unordered->append_poly_cell(2, PackedInt32Array{ e01, e50, e14, e45 });
	}
	ERR_PRINT_OFF;
	REQUIRE(unordered->is_mesh_data_valid());
	CHECK_MESSAGE(PolyMeshBuilder4D::merge_coplanar_faces(unordered) == 0, "A face whose edges are not stored in loop order must be rejected.");
	ERR_PRINT_ON;
	CHECK(unordered->get_poly_cell_indices()[0].size() == 3);
}

} // namespace TestPolyMeshBuilder4D
