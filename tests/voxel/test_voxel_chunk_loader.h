#pragma once

#include "../../voxel/voxel_chunk_loader.h"
#include "../../voxel/voxel_mesh_handler.h"
#include "../../voxel/voxel_world_4d.h"

#include "core/object/message_queue.h"
#include "core/os/os.h"
#include "tests/test_macros.h"

namespace TestVoxelChunkLoader {

static void _queue_all_chunks(VoxelChunkLoader *p_loader, const Rect4i &p_bounds) {
	const Vector4i end = p_bounds.get_end();
	for (int32_t w = p_bounds.position.w; w < end.w; w += VOXEL_DATA_CHUNK_SIZE) {
		for (int32_t z = p_bounds.position.z; z < end.z; z += VOXEL_DATA_CHUNK_SIZE) {
			for (int32_t y = p_bounds.position.y; y < end.y; y += VOXEL_DATA_CHUNK_SIZE) {
				for (int32_t x = p_bounds.position.x; x < end.x; x += VOXEL_DATA_CHUNK_SIZE) {
					p_loader->queue_load(Vector4i(x, y, z, w));
				}
			}
		}
	}
}

// The SceneTree tag makes the test harness create the MessageQueue, which
// finished chunk loads are delivered through.
TEST_CASE("[VoxelChunkLoader][SceneTree] Dynamic loading") {
	VoxelWorld4D *world = memnew(VoxelWorld4D);
	const Rect4i bounds = Rect4i(VOXEL_DATA_CHUNK_SIZE_VECTOR * -2, VOXEL_DATA_CHUNK_SIZE_VECTOR * 4);
	{
		// A loader destroyed while loads are pending must wait for its worker
		// tasks and free the content that was never stored.
		VoxelChunkLoader *abandoned_loader = memnew(VoxelChunkLoader(world));
		_queue_all_chunks(abandoned_loader, bounds);
		memdelete(abandoned_loader);
		// Its leftover finished-load messages must be dropped harmlessly.
		MessageQueue::get_singleton()->flush();
	}
	CHECK_MESSAGE(!world->get_voxel_data()->is_voxel_defined(bounds.position), "VoxelChunkLoader loads finishing after the loader's destruction should be discarded.");
	memdelete(world);

	world = memnew(VoxelWorld4D);
	VoxelChunkLoader *loader = memnew(VoxelChunkLoader(world));
	world->get_mesh_handler().mark_region_dirty(world->get_voxel_data()->get_bounds());
	world->get_mesh_handler().update_dirty_meshes();
	CHECK_MESSAGE(world->get_child_count() == 0, "VoxelMeshHandler should create no meshes while no chunks are loaded.");

	_queue_all_chunks(loader, bounds);
	// Queueing the same chunk twice while it is pending is ignored.
	loader->queue_load(bounds.position);
	CHECK_MESSAGE(!world->get_voxel_data()->is_voxel_defined(bounds.position), "VoxelChunkLoader queue_load should not modify the voxel data synchronously.");

	int safety = 0;
	while (!world->get_voxel_data()->is_region_defined(bounds) && safety < 10000) {
		OS::get_singleton()->delay_usec(1000);
		MessageQueue::get_singleton()->flush();
		world->get_mesh_handler().update_dirty_meshes();
		safety++;
	}
	CHECK_MESSAGE(world->get_voxel_data()->is_region_defined(bounds), "VoxelChunkLoader should eventually load every queued chunk.");
	world->get_mesh_handler().update_dirty_meshes();
	CHECK_MESSAGE(world->get_child_count() > 0, "VoxelMeshHandler should create the meshes once the chunks around them are loaded.");
	memdelete(loader);
	memdelete(world);
}
} // namespace TestVoxelChunkLoader
