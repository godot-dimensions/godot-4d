#pragma once

#include "../../../math/vector_4d.h"
#include "../../../model/hox/hox_document_4d.h"
#include "../../../model/mesh/mesh_instance_4d.h"
#include "../../../model/mesh/multi_surface_mesh_4d.h"
#include "../../../model/mesh/poly/array_poly_mesh_4d.h"

#include "scene/resources/packed_scene.h"
#include "tests/test_macros.h"

namespace TestHoxDocument4D {
// Two adjacent unit hoxels along X, sharing the cell on the x=1 hyperplane.
static const char *TWO_HOXELS_JSON = R"({"version":"test","materials":[{"albedo":{"r":1.0,"g":0.0,"b":0.0}}],"scene":{"hoxelGrids":[{"data":[{"i":[0,0,0,0],"m":0},{"i":[1,0,0,0],"m":0}],"dimensions":{"x":2,"y":1,"z":1,"w":1},"translation":{"x":0,"y":0,"z":0,"w":0}}]}})";

static bool is_hoxel(const Vector4 &p_point) {
	// The hoxels occupy [0,1) and [1,2) along X, and [0,1) along the other axes.
	const Vector4i cell = Vector4i(Math::floor(p_point.x), Math::floor(p_point.y), Math::floor(p_point.z), Math::floor(p_point.w));
	return cell.y == 0 && cell.z == 0 && cell.w == 0 && (cell.x == 0 || cell.x == 1);
}

// A normal at a cell centroid points outward when the hoxel is behind it and nothing is in front of it.
static void check_outward(const Vector4 &p_centroid, const Vector4 &p_normal, int &r_outward, int &r_inward, int &r_interior) {
	const bool has_inner = is_hoxel(p_centroid - p_normal * 0.5f);
	const bool has_outer = is_hoxel(p_centroid + p_normal * 0.5f);
	if (has_inner && has_outer) {
		r_interior++;
	} else if (has_inner) {
		r_outward++;
	} else {
		r_inward++;
	}
}

static void check_tetrahedra_outward(const Ref<TetraMesh4D> &p_mesh, int &r_outward, int &r_inward, int &r_interior) {
	const PackedVector4Array positions = p_mesh->get_simplex_cell_positions();
	for (int64_t t = 0; t < positions.size() / 4; t++) {
		const Vector4 &a = positions[t * 4];
		const Vector4 centroid = (a + positions[t * 4 + 1] + positions[t * 4 + 2] + positions[t * 4 + 3]) / 4.0f;
		// The vertex order is what the renderer uses to decide which side is the front.
		const Vector4 perp = Vector4D::perpendicular(positions[t * 4 + 1] - a, positions[t * 4 + 2] - a, positions[t * 4 + 3] - a);
		check_outward(centroid, perp.normalized(), r_outward, r_inward, r_interior);
	}
}

TEST_CASE("[HoxDocument4D] Boundary cells face away from their hoxel") {
	const Ref<HoxDocument4D> doc = HoxDocument4D::import_read_from_byte_array(String(TWO_HOXELS_JSON).to_utf8_buffer(), "two.hox");
	REQUIRE(doc.is_valid());
	for (const bool include_interior : { false, true }) {
		CAPTURE(include_interior);
		const Ref<MultiSurfaceMesh4D> mesh = doc->import_generate_multi_surface_mesh_4d(include_interior, HoxDocument4D::HOX_MESH_FORMAT_POLYTOPE);
		REQUIRE(mesh.is_valid());
		REQUIRE(mesh->is_mesh_data_valid());
		REQUIRE(mesh->get_surface_meshes().size() == 1);
		const Ref<ArrayPolyMesh4D> poly = mesh->get_surface_meshes()[0];
		REQUIRE(poly.is_valid());
		// Two hoxels have 16 cells, but the shared one is culled when hollow, or interior when included.
		const int64_t cell_count = poly->get_poly_cell_indices()[1].size();
		CHECK(cell_count == (include_interior ? 15 : 14));
		const PackedVector4Array normals = poly->get_poly_cell_boundary_normals();
		REQUIRE(normals.size() == cell_count);
		// Every cell's explicit boundary normal points away from its hoxel.
		const Vector<PackedInt32Array> cell_vertices = poly->get_all_boundary_cell_vertex_indices(false);
		const PackedVector4Array vertices = poly->get_poly_cell_vertex_positions();
		int outward = 0;
		int inward = 0;
		int interior = 0;
		for (int64_t cell = 0; cell < cell_vertices.size(); cell++) {
			Vector4 centroid;
			for (const int32_t v : cell_vertices[cell]) {
				centroid += vertices[v];
			}
			centroid /= (real_t)cell_vertices[cell].size();
			CHECK(Math::is_equal_approx(normals[cell].length(), (real_t)1.0));
			check_outward(centroid, normals[cell], outward, inward, interior);
		}
		CHECK(outward == 14);
		CHECK(inward == 0);
		CHECK(interior == (include_interior ? 1 : 0));
		// The tetrahedra decomposed from those cells follow the normals.
		outward = 0;
		inward = 0;
		interior = 0;
		check_tetrahedra_outward(poly, outward, inward, interior);
		CHECK(outward > 0);
		CHECK(inward == 0);
		// The tetrahedral mesh format bakes the same orientation in.
		const Ref<MultiSurfaceMesh4D> tetra_mesh = doc->import_generate_multi_surface_mesh_4d(include_interior, HoxDocument4D::HOX_MESH_FORMAT_TETRAHEDRAL);
		REQUIRE(tetra_mesh.is_valid());
		const Ref<ArrayTetraMesh4D> tetra = tetra_mesh->get_surface_meshes()[0];
		REQUIRE(tetra.is_valid());
		outward = 0;
		inward = 0;
		interior = 0;
		check_tetrahedra_outward(tetra, outward, inward, interior);
		CHECK(outward > 0);
		CHECK(inward == 0);
	}
}

TEST_CASE("[HoxDocument4D] Grid translation keeps its fractional part") {
	const char *json = R"({"version":"test","materials":[{"albedo":{"r":1.0,"g":1.0,"b":1.0}}],"scene":{"hoxelGrids":[{"data":[{"i":[0,0,0,0],"m":0}],"dimensions":{"x":1,"y":1,"z":1,"w":1},"translation":{"x":0.5,"y":0.25,"z":-0.75,"w":2.0}}]}})";
	const Ref<HoxDocument4D> doc = HoxDocument4D::import_read_from_byte_array(String(json).to_utf8_buffer(), "offset.hox");
	REQUIRE(doc.is_valid());
	const Ref<MultiSurfaceMesh4D> mesh = doc->import_generate_multi_surface_mesh_4d(false, HoxDocument4D::HOX_MESH_FORMAT_POLYTOPE);
	REQUIRE(mesh.is_valid());
	REQUIRE(mesh->get_surface_meshes().size() == 1);
	const PackedVector4Array vertices = mesh->get_surface_meshes()[0]->get_vertex_positions();
	REQUIRE(vertices.size() == 16);
	// The hoxel spans [0,1] on every axis before the translation, so its corners are the translation plus 0 or 1.
	Vector4 min_corner = vertices[0];
	for (const Vector4 &vertex : vertices) {
		min_corner = Vector4(MIN(min_corner.x, vertex.x), MIN(min_corner.y, vertex.y), MIN(min_corner.z, vertex.z), MIN(min_corner.w, vertex.w));
	}
	CHECK(min_corner.is_equal_approx(Vector4(0.5f, 0.25f, -0.75f, 2.0f)));
}

TEST_CASE("[HoxDocument4D] Including interior geometry keeps only material interfaces visible") {
	for (const bool different_materials : { false, true }) {
		CAPTURE(different_materials);
		// Give the second hoxel the default material instead of the first hoxel's red material.
		const String json = different_materials ? String(TWO_HOXELS_JSON).replace("[1,0,0,0],\"m\":0", "[1,0,0,0],\"m\":-1") : String(TWO_HOXELS_JSON);
		const Ref<HoxDocument4D> doc = HoxDocument4D::import_read_from_byte_array(json.to_utf8_buffer(), "interface.hox");
		REQUIRE(doc.is_valid());
		for (const bool include_interior : { false, true }) {
			CAPTURE(include_interior);
			for (const HoxDocument4D::HoxMeshFormat format : { HoxDocument4D::HOX_MESH_FORMAT_POLYTOPE, HoxDocument4D::HOX_MESH_FORMAT_TETRAHEDRAL }) {
				CAPTURE(format);
				const Ref<MultiSurfaceMesh4D> mesh = doc->import_generate_multi_surface_mesh_4d(include_interior, format);
				REQUIRE(mesh.is_valid());
				REQUIRE(mesh->get_surface_meshes().size() == (different_materials ? 2 : 1));
				int outward = 0;
				int inward = 0;
				int interior = 0;
				for (const Ref<SingleSurfaceMesh4D> &surface : mesh->get_surface_meshes()) {
					const Ref<TetraMesh4D> tetra_mesh = surface;
					REQUIRE(tetra_mesh.is_valid());
					CHECK(tetra_mesh->is_mesh_data_valid());
					check_tetrahedra_outward(tetra_mesh, outward, inward, interior);
				}
				CHECK(outward > 0);
				CHECK(inward == 0);
				if (different_materials && include_interior) {
					CHECK(interior > 0);
				} else {
					CHECK(interior == 0);
				}
			}
		}
	}
}

TEST_CASE("[HoxDocument4D] Multiple grids survive packing and instantiating a scene") {
	const char *json = R"({"version":"test","materials":[],"scene":{"hoxelGrids":[{"data":[{"i":[0,0,0,0],"m":-1}],"dimensions":{"x":1,"y":1,"z":1,"w":1}},{"data":[{"i":[0,0,0,0],"m":-1}],"dimensions":{"x":1,"y":1,"z":1,"w":1},"translation":{"x":2,"y":0,"z":0,"w":0}}]}})";
	const Ref<HoxDocument4D> doc = HoxDocument4D::import_read_from_byte_array(String(json).to_utf8_buffer(), "grids.hox");
	REQUIRE(doc.is_valid());
	Node4D *root = doc->import_generate_scene();
	REQUIRE(root != nullptr);
	Ref<PackedScene> packed_scene;
	packed_scene.instantiate();
	const Error pack_error = packed_scene->pack(root);
	memdelete(root);
	REQUIRE(pack_error == OK);
	Node *restored = packed_scene->instantiate();
	REQUIRE(restored != nullptr);
	CHECK(restored->get_child_count() == 2);
	for (int i = 0; i < restored->get_child_count(); i++) {
		MeshInstance4D *child = Object::cast_to<MeshInstance4D>(restored->get_child(i));
		CHECK(child != nullptr);
		if (child != nullptr) {
			CHECK(child->get_owner() == restored);
			CHECK(child->get_mesh().is_valid());
			CHECK(child->get_transform().origin == Vector4(i * 2, 0, 0, 0));
		}
	}
	memdelete(restored);
}
} // namespace TestHoxDocument4D
