#pragma once

#include "../nodes/node_4d.h"
#include "data/voxel_data.h"
#include "voxel_mesh_handler.h"

// Places a volume of 4D voxel data (VoxelData) into the scene tree.
// May be used to represent smaller voxel-based objects, not just whole worlds.
class VoxelWorld4D : public Node4D {
	GDCLASS(VoxelWorld4D, Node4D);

	Ref<VoxelData> _voxel_data;
	VoxelMeshHandler _mesh_handler;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	Ref<VoxelData> get_voxel_data() const { return _voxel_data; }

	VoxelWorld4D();
};
