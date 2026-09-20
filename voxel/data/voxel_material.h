#pragma once

#include "../../godot_4d_defines.h"

// The material a voxel is made of. Just air and solid while prototyping.
// UNDEFINED is never stored in generated voxel data; it is the null value
// returned when the value of an undefined voxel is requested.
enum class VoxelMaterial : uint8_t {
	UNDEFINED = 0,
	AIR = 1,
	SOLID = 2,
};

inline bool is_material_opaque(const VoxelMaterial p_material) {
	return p_material == VoxelMaterial::SOLID;
}

inline VoxelMaterial overlay_material(const VoxelMaterial p_over, const VoxelMaterial p_under) {
	return p_over == VoxelMaterial::UNDEFINED ? p_under : p_over;
}
