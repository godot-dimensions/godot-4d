#pragma once

#include "../../../../model/mesh/tetra/array_tetra_mesh_4d.h"

#include "tests/test_macros.h"

namespace TestArrayTetraMesh4D {
static Ref<ArrayTetraMesh4D> make_single_tetra_mesh(const bool p_with_attributes) {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertex_positions(PackedVector4Array{ Vector4(), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0) });
	mesh->set_simplex_cell_vertex_indices(PackedInt32Array{ 0, 1, 2, 3 });
	if (p_with_attributes) {
		mesh->set_simplex_cell_boundary_normals(PackedVector4Array{ Vector4(0, 0, 0, -1) });
		mesh->set_simplex_cell_vertex_normals(PackedVector4Array{ Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1) });
		mesh->set_simplex_cell_texture_map(PackedVector3Array{ Vector3(), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) });
	}
	return mesh;
}

TEST_CASE("[ArrayTetraMesh4D] Calculate Normals") {
	class ProxyUpdateProbe : public ArrayTetraMesh4D {
	public:
		int proxy_updates = 0;
		void update_proxy_mesh_3d() override { proxy_updates++; }
	};
	Ref<ProxyUpdateProbe> mesh;
	mesh.instantiate();
	const Ref<ArrayTetraMesh4D> source = make_single_tetra_mesh(false);
	mesh->set_vertex_positions(source->get_vertex_positions());
	mesh->set_simplex_cell_vertex_indices(source->get_simplex_cell_vertex_indices());
	mesh->set_simplex_cell_vertex_normals(PackedVector4Array{ Vector4(1, 0, 0, 0) });
	ERR_PRINT_OFF;
	CHECK_FALSE(mesh->is_mesh_data_valid());
	ERR_PRINT_ON;
	mesh->get_proxy_mesh_3d();
	mesh->get_proxy_mesh_3d();
	CHECK(mesh->proxy_updates == 1);
	mesh->set_flat_shading_normals();
	const PackedVector4Array normals = mesh->get_simplex_cell_vertex_normals();
	REQUIRE(normals.size() == 4);
	for (const Vector4 &normal : normals) {
		CHECK(normal == Vector4(0, 0, 0, 1));
	}
	mesh->get_proxy_mesh_3d();
	CHECK(mesh->proxy_updates == 2);
	CHECK(mesh->is_mesh_data_valid());
}

TEST_CASE("[ArrayTetraMesh4D] Merge Meshes") {
	const Transform4D transform(Basis4D::from_scale_uniform(-1), Vector4(10, 0, 0, 0));
	for (const bool destination_has_attributes : { false, true }) {
		for (const bool source_has_attributes : { false, true }) {
			CAPTURE(destination_has_attributes);
			CAPTURE(source_has_attributes);
			Ref<ArrayTetraMesh4D> mesh = make_single_tetra_mesh(destination_has_attributes);
			const Ref<ArrayTetraMesh4D> source = make_single_tetra_mesh(source_has_attributes);
			const PackedVector4Array source_normals = source->get_simplex_cell_vertex_normals();
			const PackedVector3Array source_textures = source->get_simplex_cell_texture_map();
			const PackedVector4Array original_normals = mesh->get_simplex_cell_vertex_normals();
			const PackedVector3Array original_textures = mesh->get_simplex_cell_texture_map();
			REQUIRE(mesh->get_simplex_cell_positions().size() == 4);
			mesh->merge_with(source, transform);
			REQUIRE(mesh->is_mesh_data_valid());
			CHECK(mesh->get_simplex_cell_vertex_indices() == PackedInt32Array({ 0, 1, 2, 3, 4, 5, 6, 7 }));
			const PackedVector4Array positions = mesh->get_simplex_cell_positions();
			REQUIRE(positions.size() == 8);
			for (int i = 0; i < 4; i++) {
				CHECK(positions[i + 4] == transform * source->get_vertex_positions()[i]);
			}
			const PackedVector4Array normals = mesh->get_simplex_cell_vertex_normals();
			const PackedVector3Array textures = mesh->get_simplex_cell_texture_map();
			if (destination_has_attributes || source_has_attributes) {
				REQUIRE(normals.size() == 8);
				REQUIRE(textures.size() == 8);
				for (int i = 0; i < 4; i++) {
					CHECK(normals[i] == (destination_has_attributes ? original_normals[i] : Vector4()));
					CHECK(textures[i] == (destination_has_attributes ? original_textures[i] : Vector3()));
					CHECK(normals[i + 4] == (source_has_attributes ? transform.basis * source_normals[i] : Vector4()));
					CHECK(textures[i + 4] == (source_has_attributes ? source_textures[i] : Vector3()));
				}
				const PackedVector4Array boundary_normals = mesh->get_simplex_cell_boundary_normals();
				REQUIRE(boundary_normals.size() == 2);
				CHECK(boundary_normals[0] == (destination_has_attributes ? Vector4(0, 0, 0, -1) : Vector4()));
				CHECK(boundary_normals[1] == (source_has_attributes ? Vector4(0, 0, 0, 1) : Vector4()));
			} else {
				CHECK(normals.is_empty());
				CHECK(textures.is_empty());
			}
			CHECK(source->get_simplex_cell_vertex_normals() == source_normals);
			CHECK(source->get_simplex_cell_texture_map() == source_textures);
		}
	}
}

TEST_CASE("[ArrayTetraMesh4D] Empty and invalid merge inputs") {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	const Ref<ArrayTetraMesh4D> source = make_single_tetra_mesh(true);
	mesh->merge_with(source);
	CHECK(mesh->get_simplex_cell_texture_map() == source->get_simplex_cell_texture_map());
	CHECK(mesh->get_simplex_cell_boundary_normals() == source->get_simplex_cell_boundary_normals());
	Ref<ArrayTetraMesh4D> empty;
	empty.instantiate();
	mesh->merge_with(empty);
	CHECK(mesh->get_simplex_cell_texture_map() == source->get_simplex_cell_texture_map());
	const PackedVector4Array positions = mesh->get_vertex_positions();
	const PackedInt32Array indices = mesh->get_simplex_cell_vertex_indices();
	Ref<ArrayTetraMesh4D> invalid = make_single_tetra_mesh(false);
	invalid->set_simplex_cell_vertex_indices(PackedInt32Array{ 0, 1, 2, 4 });
	ERR_PRINT_OFF; // Null and invalid sources must be rejected without modifying the destination.
	mesh->merge_with(Ref<ArrayTetraMesh4D>());
	mesh->merge_with(invalid);
	ERR_PRINT_ON;
	CHECK(mesh->get_vertex_positions() == positions);
	CHECK(mesh->get_simplex_cell_vertex_indices() == indices);
	CHECK(mesh->get_simplex_cell_texture_map() == source->get_simplex_cell_texture_map());
	CHECK(mesh->is_mesh_data_valid());
}
} // namespace TestArrayTetraMesh4D
