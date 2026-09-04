#pragma once

#include "../../voxel/voxel_mesh_handler.h"
#include "../../voxel/voxel_world_4d.h"

#include "tests/test_macros.h"

namespace TestVoxelMeshHandler {
TEST_CASE("[VoxelMeshHandler] Initialize") {
	VoxelWorld4D *world = memnew(VoxelWorld4D);
	VoxelMeshHandler handler = VoxelMeshHandler(world);
	handler.initialize();
	CHECK_MESSAGE(world->get_child_count() > 0, "VoxelMeshHandler initialize should add mesh instances for chunks on the test shape's surface.");
	CHECK_MESSAGE(world->get_child_count() < 8 * 8 * 8 * 8, "VoxelMeshHandler initialize should not add mesh instances for chunks with empty meshes.");
	const int initialized_child_count = world->get_child_count();
	handler.initialize();
	CHECK_MESSAGE(world->get_child_count() == initialized_child_count, "VoxelMeshHandler initialize should do nothing when the meshes were already generated.");
	memdelete(world);
}
} // namespace TestVoxelMeshHandler
