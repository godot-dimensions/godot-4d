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

// The data stored for each voxel.
struct VoxelValue {
	VoxelMaterial material = VoxelMaterial::UNDEFINED;
	// How far the voxel's center is from the nearest surface: 0 at the
	// surface, increasing the deeper the voxel is inside its material,
	// whether that material is solid or air. On an edge between materials,
	// the surface crosses at the fraction a / (a + b) of the way from the
	// voxel with density a to the voxel with density b.
	uint8_t density = 0;

	bool is_opaque() const { return material == VoxelMaterial::SOLID; }

	bool operator==(const VoxelValue &p_other) const { return material == p_other.material && density == p_other.density; }
	bool operator!=(const VoxelValue &p_other) const { return !(*this == p_other); }
};

static_assert(sizeof(VoxelValue) == 2);
