#pragma once

#include "../model/mesh/mesh_instance_4d.h"
#include "voxel_constants.h"

class VoxelWorld4D;

// Keeps track of the chunk meshes of one VoxelWorld4D, which have been generated,
// and (eventually) which need to be generated. This is conceptually part of
// VoxelWorld4D, split into its own class for readability.
class VoxelMeshHandler {
	VoxelWorld4D *const _world;
	// All existing chunk meshes, keyed by the lowest voxel coordinate of the
	// VOXEL_MESH_CHUNK_SIZE hypercube of voxel space each mesh covers.
	HashMap<Vector4i, MeshInstance4D *> _chunk_meshes;

public:
	// Generates a mesh for every mesh chunk inside the world's bounds and
	// adds them to the world as MeshInstance4D children, skipping chunks
	// whose mesh would be empty. Does nothing if the meshes have already
	// been generated.
	void initialize();

	explicit VoxelMeshHandler(VoxelWorld4D *p_world) :
			_world(p_world) {}
};
