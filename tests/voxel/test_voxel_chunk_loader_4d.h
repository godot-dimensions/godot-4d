#pragma once

#include "../../voxel/generators/tiger_test_generator_4d.h"
#include "../../voxel/voxel_chunk_loader_4d.h"
#include "../../voxel/voxel_load_trigger_4d.h"
#include "../../voxel/voxel_mesh_handler_4d.h"
#include "../../voxel/voxel_world_4d.h"

#include "core/object/message_queue.h"
#include "core/os/os.h"
#include "scene/main/window.h"
#include "tests/test_macros.h"

namespace TestVoxelChunkLoader4D {

static void _queue_all_chunks(VoxelChunkLoader4D *p_loader, const Rect4i &p_bounds) {
	const Vector4i end = p_bounds.get_end();
	for (int32_t w = p_bounds.position.w; w < end.w; w += VOXEL_4D_DATA_CHUNK_SIZE) {
		for (int32_t z = p_bounds.position.z; z < end.z; z += VOXEL_4D_DATA_CHUNK_SIZE) {
			for (int32_t y = p_bounds.position.y; y < end.y; y += VOXEL_4D_DATA_CHUNK_SIZE) {
				for (int32_t x = p_bounds.position.x; x < end.x; x += VOXEL_4D_DATA_CHUNK_SIZE) {
					p_loader->queue_load(Vector4i(x, y, z, w));
				}
			}
		}
	}
}

// The SceneTree tag makes the test harness create the MessageQueue, which
// finished chunk loads are delivered through.
TEST_CASE("[SceneTree][VoxelChunkLoader4D] Dynamic loading") {
	VoxelWorld4D *world = memnew(VoxelWorld4D);
	world->set_generator(memnew(TigerTestGenerator4D));
	const Rect4i bounds = Rect4i(VOXEL_4D_DATA_CHUNK_SIZE_VECTOR * -2, VOXEL_4D_DATA_CHUNK_SIZE_VECTOR * 4);
	{
		// A loader destroyed while loads are pending must wait for its worker
		// tasks and free the content that was never stored.
		VoxelChunkLoader4D *abandoned_loader = memnew(VoxelChunkLoader4D(world));
		_queue_all_chunks(abandoned_loader, bounds);
		memdelete(abandoned_loader);
		// Its leftover finished-load messages must be dropped harmlessly.
		MessageQueue::get_singleton()->flush();
	}
	CHECK_MESSAGE(!world->get_voxel_data()->is_voxel_defined(bounds.position), "VoxelChunkLoader4D loads finishing after the loader's destruction should be discarded.");
	memdelete(world);

	world = memnew(VoxelWorld4D);
	world->set_generator(memnew(TigerTestGenerator4D));
	VoxelChunkLoader4D *loader = memnew(VoxelChunkLoader4D(world));
	world->get_mesh_handler().mark_region_dirty(world->get_voxel_data()->get_bounds());
	world->get_mesh_handler().update_dirty_meshes();
	CHECK_MESSAGE(world->get_child_count() == 0, "VoxelMeshHandler4D should create no meshes while no chunks are loaded.");

	_queue_all_chunks(loader, bounds);
	// Queueing the same chunk twice while it is pending is ignored.
	loader->queue_load(bounds.position);
	CHECK_MESSAGE(!world->get_voxel_data()->is_voxel_defined(bounds.position), "VoxelChunkLoader4D queue_load should not modify the voxel data synchronously.");

	int safety = 0;
	while (!world->get_voxel_data()->is_region_defined(bounds) && safety < 10000) {
		OS::get_singleton()->delay_usec(1000);
		MessageQueue::get_singleton()->flush();
		world->get_mesh_handler().update_dirty_meshes();
		safety++;
	}
	CHECK_MESSAGE(world->get_voxel_data()->is_region_defined(bounds), "VoxelChunkLoader4D should eventually load every queued chunk.");
	world->get_mesh_handler().update_dirty_meshes();
	CHECK_MESSAGE(world->get_child_count() > 0, "VoxelMeshHandler4D should create the meshes once the chunks around them are loaded.");
	memdelete(loader);
	memdelete(world);
}

TEST_CASE("[SceneTree][VoxelChunkLoader4D] World bounds") {
	Window *root = SceneTree::get_singleton()->get_root();
	VoxelWorld4D *world = memnew(VoxelWorld4D);
	world->set_generator(memnew(TigerTestGenerator4D));
	world->set_world_bounds_enabled(true);
	// Not aligned to the chunk grid, so some chunks are partially inside.
	world->set_world_bounds_position(Vector4i(-3, -3, -3, -3));
	world->set_world_bounds_size(Vector4i(14, 14, 14, 14));
	root->add_child(world);
	VoxelLoadTrigger4D *trigger = memnew(VoxelLoadTrigger4D);
	trigger->set_load_distance(2.0f * VOXEL_4D_DATA_CHUNK_SIZE);
	root->add_child(trigger);
	VoxelChunkLoader4D *loader = memnew(VoxelChunkLoader4D(world));
	loader->update_loaded_chunks();
	const Ref<VoxelData4D> voxel_data = world->get_voxel_data();
	int safety = 0;
	while (!(voxel_data->is_voxel_defined(Vector4i(1, 1, 1, 1)) && voxel_data->is_voxel_defined(Vector4i(-1, -1, -1, -1))) && safety < 10000) {
		OS::get_singleton()->delay_usec(1000);
		MessageQueue::get_singleton()->flush();
		safety++;
	}
	CHECK_MESSAGE(voxel_data->is_voxel_defined(Vector4i(1, 1, 1, 1)), "VoxelChunkLoader4D should load chunks inside the world bounds around the trigger.");
	CHECK_MESSAGE(voxel_data->is_voxel_defined(Vector4i(-1, -1, -1, -1)), "VoxelChunkLoader4D should load chunks partially inside the world bounds.");
	bool outside_loaded = false;
	for (int32_t w = -4; w <= 4 && !outside_loaded; w++) {
		for (int32_t z = -4; z <= 4 && !outside_loaded; z++) {
			for (int32_t y = -4; y <= 4 && !outside_loaded; y++) {
				for (int32_t x = -4; x <= 4 && !outside_loaded; x++) {
					const Rect4i chunk = Rect4i(Vector4i(x, y, z, w) * VOXEL_4D_DATA_CHUNK_SIZE, VOXEL_4D_DATA_CHUNK_SIZE_VECTOR);
					if (!chunk.intersects_exclusive(world->get_world_bounds())) {
						outside_loaded = outside_loaded || voxel_data->is_voxel_defined(chunk.position);
					}
				}
			}
		}
	}
	CHECK_MESSAGE(!outside_loaded, "VoxelChunkLoader4D should never load chunks entirely outside the world bounds.");

	// Shrinking the bounds unloads the chunks they no longer reach,
	// synchronously.
	CHECK_MESSAGE(voxel_data->is_voxel_defined(Vector4i(9, 0, 0, 0)), "VoxelChunkLoader4D should have loaded the chunk that the shrunk bounds will exclude.");
	world->set_world_bounds_size(Vector4i(6, 6, 6, 6));
	loader->update_loaded_chunks();
	CHECK_MESSAGE(!voxel_data->is_voxel_defined(Vector4i(9, 0, 0, 0)), "VoxelChunkLoader4D should unload chunks that leave the world bounds.");

	// Degenerate empty bounds with a distant trigger: everything unloads and,
	// as a regression check for the probe walk, the update must terminate.
	world->set_world_bounds_size(Vector4i(0, 0, 0, 0));
	trigger->set_position(Vector4(100, 0, 0, 0));
	loader->update_loaded_chunks();
	CHECK_MESSAGE(!voxel_data->is_voxel_defined(Vector4i(1, 1, 1, 1)), "VoxelChunkLoader4D should unload everything when the world bounds are empty.");

	memdelete(loader);
	root->remove_child(world);
	root->remove_child(trigger);
	memdelete(world);
	memdelete(trigger);
}
} // namespace TestVoxelChunkLoader4D
