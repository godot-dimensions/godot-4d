#pragma once

#include "../../../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../../../model/mesh/tetra/box_tetra_mesh_4d.h"
#include "../mesh_attribute_test_helpers_4d.h"

#include "scene/resources/mesh.h"
#include "tests/test_macros.h"

namespace TestArrayTetraMesh4D {
using namespace TestMeshAttributes4D;

static Ref<ArrayTetraMesh4D> make_single_tetra_mesh(const bool p_with_attributes) {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	mesh->set_vertex_positions(PackedVector4Array{ Vector4(), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0) });
	mesh->set_simplex_cell_vertex_indices(PackedInt32Array{ 0, 1, 2, 3 });
	if (p_with_attributes) {
		mesh->set_simplex_cell_boundary_normals(PackedVector4Array{ Vector4(0, 0, 0, -1) });
		set_simplex_normal_values(mesh, PackedVector4Array{ Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1) });
		set_simplex_texture_map_values(mesh, PackedVector3Array{ Vector3(), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) });
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
	set_simplex_normal_values(mesh, PackedVector4Array{ Vector4(1, 0, 0, 0) });
	ERR_PRINT_OFF;
	CHECK_FALSE(mesh->is_mesh_data_valid());
	ERR_PRINT_ON;
	mesh->get_proxy_mesh_3d();
	mesh->get_proxy_mesh_3d();
	CHECK(mesh->proxy_updates == 1);
	mesh->set_flat_shading_normals();
	const PackedVector4Array normals = sample_normal_values(mesh);
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
			const PackedVector4Array source_normals = sample_normal_values(source);
			const PackedVector3Array source_textures = sample_texture_map_values(source);
			const PackedVector4Array original_normals = sample_normal_values(mesh);
			const PackedVector3Array original_textures = sample_texture_map_values(mesh);
			REQUIRE(mesh->get_simplex_cell_positions().size() == 4);
			mesh->merge_with(source, transform);
			REQUIRE(mesh->is_mesh_data_valid());
			CHECK(mesh->get_simplex_cell_vertex_indices() == PackedInt32Array({ 0, 1, 2, 3, 4, 5, 6, 7 }));
			const PackedVector4Array positions = mesh->get_simplex_cell_positions();
			REQUIRE(positions.size() == 8);
			for (int i = 0; i < 4; i++) {
				CHECK(positions[i + 4] == transform * source->get_vertex_positions()[i]);
			}
			const PackedVector4Array normals = sample_normal_values(mesh);
			const PackedVector3Array textures = sample_texture_map_values(mesh);
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
			CHECK(sample_normal_values(source) == source_normals);
			CHECK(sample_texture_map_values(source) == source_textures);
		}
	}
}

TEST_CASE("[ArrayTetraMesh4D] Empty and invalid merge inputs") {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	const Ref<ArrayTetraMesh4D> source = make_single_tetra_mesh(true);
	mesh->merge_with(source);
	CHECK(sample_texture_map_values(mesh) == sample_texture_map_values(source));
	CHECK(mesh->get_simplex_cell_boundary_normals() == source->get_simplex_cell_boundary_normals());
	Ref<ArrayTetraMesh4D> empty;
	empty.instantiate();
	const PackedVector4Array normal_values = mesh->get_normal_values();
	const PackedVector3Array texture_values = mesh->get_texture_map_values();
	mesh->merge_with(empty);
	CHECK(mesh->get_normal_values() == normal_values);
	CHECK(mesh->get_texture_map_values() == texture_values);
	CHECK(sample_texture_map_values(mesh) == sample_texture_map_values(source));
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
	CHECK(sample_texture_map_values(mesh) == sample_texture_map_values(source));
	CHECK(mesh->is_mesh_data_valid());
}

TEST_CASE("[SceneTree][ArrayTetraMesh4D] Direct proxy and texture export reject invalid data and recover") {
	const Ref<ArrayTetraMesh4D> source = make_single_tetra_mesh(true);
	for (const bool warm_proxy : { false, true }) {
		for (int invalid_data = 0; invalid_data < 12; invalid_data++) {
			CAPTURE(warm_proxy);
			CAPTURE(invalid_data);
			Ref<ArrayTetraMesh4D> mesh = make_single_tetra_mesh(true);
			Ref<ArrayMesh> proxy;
			if (warm_proxy) {
				proxy = mesh->get_proxy_mesh_3d();
				REQUIRE(proxy.is_valid());
				REQUIRE(proxy->get_surface_count() == 1);
				CHECK(proxy->surface_get_array_len(0) == 12);
			}
			switch (invalid_data) {
				case 0:
					mesh->set_simplex_cell_vertex_indices(PackedInt32Array{ 0, 1, 2 });
					break;
				case 1:
					mesh->set_simplex_cell_vertex_indices(PackedInt32Array{ 0, 1, 2, -1 });
					break;
				case 2:
					mesh->set_simplex_cell_vertex_indices(PackedInt32Array{ 0, 1, 2, 4 });
					break;
				case 3:
					set_simplex_texture_map_values(mesh, PackedVector3Array{ Vector3() });
					break;
				case 4: {
					PackedVector3Array texture_map = sample_texture_map_values(mesh);
					texture_map.append(Vector3());
					set_simplex_texture_map_values(mesh, texture_map);
				} break;
				case 5:
					set_simplex_normal_values(mesh, PackedVector4Array{ Vector4() });
					break;
				case 6:
					mesh->set_simplex_cell_boundary_normals(PackedVector4Array{ Vector4(), Vector4() });
					break;
				case 7:
					mesh->set_vertex_positions(PackedVector4Array());
					break;
				case 8:
				case 9: {
					PackedInt32Array indices = mesh->get_simplex_cell_texture_map_indices();
					indices.set(0, invalid_data == 8 ? -1 : mesh->get_texture_map_values().size());
					mesh->set_simplex_cell_texture_map_indices(indices);
				} break;
				case 10:
				case 11: {
					PackedInt32Array indices = mesh->get_simplex_cell_normal_indices();
					indices.set(0, invalid_data == 10 ? -1 : mesh->get_normal_values().size());
					mesh->set_simplex_cell_normal_indices(indices);
				} break;
			}
			ERR_PRINT_OFF;
			const Ref<ArrayMesh> invalid_proxy = mesh->get_proxy_mesh_3d();
			ERR_PRINT_ON;
			REQUIRE(invalid_proxy.is_valid());
			CHECK(invalid_proxy->get_surface_count() == 0);
			if (warm_proxy) {
				CHECK(invalid_proxy == proxy);
				CHECK(proxy->get_surface_count() == 0);
			}
			ERR_PRINT_OFF;
			const Ref<ArrayMesh> invalid_export = mesh->export_texture_map_mesh();
			ERR_PRINT_ON;
			CHECK(invalid_export.is_null());

			mesh->set_vertex_positions(source->get_vertex_positions());
			mesh->set_simplex_cell_vertex_indices(source->get_simplex_cell_vertex_indices());
			set_simplex_normal_values(mesh, sample_normal_values(source));
			mesh->set_simplex_cell_boundary_normals(source->get_simplex_cell_boundary_normals());
			set_simplex_texture_map_values(mesh, sample_texture_map_values(source));
			proxy = mesh->get_proxy_mesh_3d();
			REQUIRE(proxy.is_valid());
			CHECK(proxy == invalid_proxy);
			REQUIRE(proxy->get_surface_count() == 1);
			CHECK(proxy->surface_get_array_len(0) == 12);
			const Ref<ArrayMesh> exported = mesh->export_texture_map_mesh();
			REQUIRE(exported.is_valid());
			REQUIRE(exported->get_surface_count() == 1);
			CHECK(exported->surface_get_array_len(0) == 12);
		}
	}
}

TEST_CASE("[SceneTree][ArrayTetraMesh4D] Appending a missing vertex rebuilds an invalid proxy") {
	for (const bool cache_partial_positions : { false, true }) {
		CAPTURE(cache_partial_positions);
		Ref<ArrayTetraMesh4D> mesh = make_single_tetra_mesh(false);
		mesh->set_simplex_cell_vertex_indices(PackedInt32Array{ 0, 1, 2, 4 });
		if (cache_partial_positions) {
			ERR_PRINT_OFF;
			const PackedVector4Array partial_positions = mesh->get_simplex_cell_positions();
			ERR_PRINT_ON;
			CHECK(partial_positions.size() == 3);
		}
		ERR_PRINT_OFF;
		const Ref<ArrayMesh> proxy = mesh->get_proxy_mesh_3d();
		ERR_PRINT_ON;
		REQUIRE(proxy.is_valid());
		CHECK(proxy->get_surface_count() == 0);
		const Vector4 missing_position(0, 0, 1, 1);
		CHECK(mesh->append_vertex(missing_position) == 4);
		const Ref<ArrayMesh> repaired_proxy = mesh->get_proxy_mesh_3d();
		CHECK(repaired_proxy == proxy);
		REQUIRE(repaired_proxy->get_surface_count() == 1);
		CHECK(repaired_proxy->surface_get_array_len(0) == 12);
		CHECK(mesh->get_simplex_cell_positions() == PackedVector4Array({ Vector4(), Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0), missing_position }));
	}
}

TEST_CASE("[SceneTree][ArrayTetraMesh4D] Empty and degenerate texture exports") {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	CHECK(mesh->get_proxy_mesh_3d()->get_surface_count() == 0);
	const Ref<ArrayMesh> empty_export = mesh->export_texture_map_mesh();
	REQUIRE(empty_export.is_valid());
	CHECK(empty_export->get_surface_count() == 0);

	mesh = make_single_tetra_mesh(false);
	CHECK(mesh->get_proxy_mesh_3d()->get_surface_count() == 1);
	const Ref<ArrayMesh> unmapped_export = mesh->export_texture_map_mesh();
	REQUIRE(unmapped_export.is_valid());
	CHECK(unmapped_export->get_surface_count() == 0);

	// Duplicate points and distinct coplanar points both describe degenerate texture cells.
	for (const PackedVector3Array &texture_map : {
				 PackedVector3Array{ Vector3(), Vector3(), Vector3(0, 1, 0), Vector3(0, 0, 1) },
				 PackedVector3Array{ Vector3(), Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(1, 1, 0) } }) {
		set_simplex_texture_map_values(mesh, texture_map);
		const Ref<ArrayMesh> degenerate_export = mesh->export_texture_map_mesh();
		REQUIRE(degenerate_export.is_valid());
		CHECK(degenerate_export->get_surface_count() == 0);

		const Ref<ArrayTetraMesh4D> mapped = make_single_tetra_mesh(true);
		mesh->merge_with(mapped);
		const Ref<ArrayMesh> mixed_export = mesh->export_texture_map_mesh();
		REQUIRE(mixed_export.is_valid());
		REQUIRE(mixed_export->get_surface_count() == 1);
		CHECK(mixed_export->surface_get_array_len(0) == 12);
		mesh = make_single_tetra_mesh(false);
	}
}

TEST_CASE("[SceneTree][BoxTetraMesh4D] Texture export validates and preserves all mapping modes") {
	class InvalidBoxMesh : public BoxTetraMesh4D {
	public:
		bool validate_mesh_data() override { return false; }
	};
	Ref<InvalidBoxMesh> invalid;
	invalid.instantiate();
	ERR_PRINT_OFF;
	const Ref<ArrayMesh> invalid_export = invalid->export_texture_map_mesh();
	ERR_PRINT_ON;
	CHECK(invalid_export.is_null());

	for (int decomposition = 0; decomposition < 3; decomposition++) {
		for (int mapping = 0; mapping < 4; mapping++) {
			CAPTURE(decomposition);
			CAPTURE(mapping);
			Ref<BoxTetraMesh4D> mesh;
			mesh.instantiate();
			mesh->set_tetra_decomp((BoxTetraMesh4D::BoxTetraDecomp)decomposition);
			mesh->set_cell_texture_map((BoxTetraMesh4D::BoxCellTextureMap)mapping);
			const Ref<ArrayMesh> exported = mesh->export_texture_map_mesh();
			REQUIRE(exported.is_valid());
			REQUIRE(exported->get_surface_count() == 1);
			CHECK(exported->surface_get_array_len(0) == mesh->get_simplex_cell_vertex_indices().size() * 3);
		}
	}
}

TEST_CASE("[ArrayTetraMesh4D] Explicit compaction removes unreferenced data") {
	Ref<ArrayTetraMesh4D> mesh = make_single_tetra_mesh(true);
	// Point every attribute index at one value, orphaning the rest until compaction.
	mesh->set_simplex_cell_normal_indices(PackedInt32Array{ 0, 0, 0, 0 });
	mesh->set_simplex_cell_texture_map_indices(PackedInt32Array{ 3, 3, 3, 3 });
	CHECK(mesh->get_normal_values().size() == 4);
	CHECK(mesh->get_texture_map_values().size() == 4);
	mesh->compact_normal_values();
	mesh->compact_texture_map_values();
	CHECK(mesh->get_normal_values() == PackedVector4Array{ Vector4(1, 0, 0, 0) });
	CHECK(mesh->get_texture_map_values() == PackedVector3Array{ Vector3(0, 0, 1) });
	CHECK(mesh->get_simplex_cell_texture_map_indices() == PackedInt32Array({ 0, 0, 0, 0 }));
	// Compaction merges duplicated values in the pool, remapping the indices.
	mesh->set_normal_values(PackedVector4Array{ Vector4(0, 0, 0, 1), Vector4(0, 0, 0, 1) });
	mesh->set_simplex_cell_normal_indices(PackedInt32Array{ 0, 1, 0, 1 });
	mesh->compact_normal_values();
	CHECK(mesh->get_normal_values() == PackedVector4Array{ Vector4(0, 0, 0, 1) });
	CHECK(mesh->get_simplex_cell_normal_indices() == PackedInt32Array({ 0, 0, 0, 0 }));
}
} // namespace TestArrayTetraMesh4D
