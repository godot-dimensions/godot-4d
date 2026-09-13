#pragma once

#include "../../voxel/voxel_mesh_handler.h"
#include "../../voxel/voxel_world_4d.h"

#include "tests/test_macros.h"

namespace TestVoxelMeshHandler {

// Drains the dirty set despite update_dirty_meshes's per-call cap; enough
// calls for every mesh chunk in the test world's bounds.
static void _update_all_dirty_meshes(VoxelMeshHandler &p_handler) {
	for (int i = 0; i < 256; i++) {
		p_handler.update_dirty_meshes();
	}
}

static Ref<Mesh4D> _chunk_mesh_at(const VoxelWorld4D *p_world, const Vector4i &p_chunk_position) {
	for (int i = 0; i < p_world->get_child_count(); i++) {
		MeshInstance4D *mesh_instance = Object::cast_to<MeshInstance4D>(p_world->get_child(i));
		if (mesh_instance != nullptr && mesh_instance->get_position() == Vector4(p_chunk_position)) {
			return mesh_instance->get_mesh();
		}
	}
	return Ref<Mesh4D>();
}

TEST_CASE("[VoxelMeshHandler] Mesh generation") {
	VoxelWorld4D *world = memnew(VoxelWorld4D);
	world->get_voxel_data()->load_all_chunks();
	VoxelMeshHandler handler = VoxelMeshHandler(world);
	handler.mark_region_dirty(world->get_voxel_data()->get_bounds());
	_update_all_dirty_meshes(handler);
	CHECK_MESSAGE(world->get_child_count() > 0, "VoxelMeshHandler should add mesh instances for chunks on the test shape's surface.");
	CHECK_MESSAGE(world->get_child_count() < 8 * 8 * 8 * 8, "VoxelMeshHandler should not add mesh instances for chunks with empty meshes.");
	memdelete(world);
}

TEST_CASE("[VoxelMeshHandler] Dirty region updates") {
	VoxelWorld4D *world = memnew(VoxelWorld4D);
	world->get_voxel_data()->load_all_chunks();
	VoxelMeshHandler handler = VoxelMeshHandler(world);
	handler.mark_region_dirty(world->get_voxel_data()->get_bounds());
	_update_all_dirty_meshes(handler);
	const Vector4i target_chunk = Vector4i(8, 0, 8, 0);
	const Vector4i lower_neighbor_chunk = Vector4i(0, 0, 8, 0);
	const Vector4i far_chunk = Vector4i(8, 0, 8, -8);
	Ref<Mesh4D> target_before = _chunk_mesh_at(world, target_chunk);
	const Ref<Mesh4D> neighbor_before = _chunk_mesh_at(world, lower_neighbor_chunk);
	const Ref<Mesh4D> far_before = _chunk_mesh_at(world, far_chunk);
	REQUIRE(target_before.is_valid());
	REQUIRE(neighbor_before.is_valid());
	REQUIRE(far_before.is_valid());
	const int child_count = world->get_child_count();

	handler.update_dirty_meshes();
	CHECK_MESSAGE(_chunk_mesh_at(world, target_chunk) == target_before, "VoxelMeshHandler update_dirty_meshes should do nothing when no region was marked dirty.");

	handler.mark_region_dirty(Rect4i(10, 2, 10, 2, 1, 1, 1, 1));
	handler.update_dirty_meshes();
	CHECK_MESSAGE(_chunk_mesh_at(world, target_chunk) != target_before, "VoxelMeshHandler should regenerate the mesh of the chunk containing the dirty region.");
	CHECK_MESSAGE(_chunk_mesh_at(world, far_chunk) == far_before, "VoxelMeshHandler should not regenerate the meshes of chunks that do not read the dirty region.");
	CHECK_MESSAGE(world->get_child_count() == child_count, "VoxelMeshHandler should reuse the mesh instance when regenerating a chunk's mesh.");

	// A change on a chunk's lower border also invalidates the neighboring
	// chunk's mesh, which reads values and normals one step past its own
	// upper border.
	target_before = _chunk_mesh_at(world, target_chunk);
	handler.mark_region_dirty(Rect4i(8, 2, 10, 2, 1, 1, 1, 1));
	handler.update_dirty_meshes();
	CHECK_MESSAGE(_chunk_mesh_at(world, target_chunk) != target_before, "VoxelMeshHandler should regenerate the chunk containing the dirty region.");
	CHECK_MESSAGE(_chunk_mesh_at(world, lower_neighbor_chunk) != neighbor_before, "VoxelMeshHandler should regenerate a neighboring chunk whose mesh reads data in the dirty region.");
	memdelete(world);
}
} // namespace TestVoxelMeshHandler
