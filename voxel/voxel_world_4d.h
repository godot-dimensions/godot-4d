#pragma once

#include "../nodes/node_4d.h"
#include "data/voxel_data.h"
#include "voxel_chunk_loader.h"
#include "voxel_mesh_handler.h"

// Places a volume of 4D voxel data (VoxelData) into the scene tree.
// May be used to represent smaller voxel-based objects, not just whole worlds.
class VoxelWorld4D : public Node4D {
	GDCLASS(VoxelWorld4D, Node4D);

	Ref<VoxelData> _voxel_data;
	VoxelMeshHandler _mesh_handler;
	// Owned; an Object so it can receive messages, so it cannot be a value member.
	VoxelChunkLoader *_chunk_loader = nullptr;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	Ref<VoxelData> get_voxel_data() const { return _voxel_data; }
	VoxelMeshHandler &get_mesh_handler() { return _mesh_handler; }

	VoxelWorld4D();
	~VoxelWorld4D();
};
