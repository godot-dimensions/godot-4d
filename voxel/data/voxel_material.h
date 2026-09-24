#pragma once

#include "../../godot_4d_defines.h"

// The material a voxel is made of. Only the reserved materials are named;
// every other value is a distinct opaque material, with meanings to be
// assigned later. UNDEFINED is never stored in generated voxel data; it is
// the null value returned when the value of an undefined voxel is requested.
enum class VoxelMaterial : uint8_t {
	UNDEFINED = 0,
	AIR = 1,
	// The number of reserved materials above, and so also the first
	// unreserved material.
	RESERVED_COUNT = 2,
};

inline bool is_material_opaque(const VoxelMaterial p_material) {
	return p_material >= VoxelMaterial::RESERVED_COUNT;
}

inline VoxelMaterial overlay_material(const VoxelMaterial p_over, const VoxelMaterial p_under) {
	return p_over == VoxelMaterial::UNDEFINED ? p_under : p_over;
}

// Whether a face of the surface mesh separates two adjacent voxels, and which
// of them its normal points toward.
enum class VoxelFace {
	NONE,
	TOWARD_FIRST,
	TOWARD_SECOND,
};

// The face needed between adjacent voxels of the two materials, depending on
// their opacity.
// While this may be extended to other cases later, the relation of having no
// face should always remain an equivalence relation among materials other than
// UNDEFINED, in order to ensure mesh closedness.
inline VoxelFace get_face_between(const VoxelMaterial p_first, const VoxelMaterial p_second) {
	if (p_first == VoxelMaterial::UNDEFINED || p_second == VoxelMaterial::UNDEFINED) {
		return VoxelFace::NONE;
	}
	const bool first_opaque = is_material_opaque(p_first);
	if (first_opaque == is_material_opaque(p_second)) {
		return VoxelFace::NONE;
	}
	return first_opaque ? VoxelFace::TOWARD_SECOND : VoxelFace::TOWARD_FIRST;
}
