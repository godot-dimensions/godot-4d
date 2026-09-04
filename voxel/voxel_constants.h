#pragma once

#include "../godot_4d_defines.h"

// Miscellaneous constants used throughout the voxel system.

// The size of a chunk of voxel data on each axis, so each chunk
// holds VOXEL_DATA_CHUNK_SIZE^4 voxels. Kept small while prototyping.
constexpr int32_t VOXEL_DATA_CHUNK_SIZE = 4;

// The total number of voxels in a chunk.
constexpr int32_t VOXEL_DATA_CHUNK_HYPERVOLUME = VOXEL_DATA_CHUNK_SIZE * VOXEL_DATA_CHUNK_SIZE * VOXEL_DATA_CHUNK_SIZE * VOXEL_DATA_CHUNK_SIZE;

// The size on each axis of the region covered by one generated chunk mesh.
constexpr int32_t VOXEL_MESH_CHUNK_SIZE = 8;
