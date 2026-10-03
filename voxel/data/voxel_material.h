#pragma once

#include "../../godot_4d_defines.h"

// The material a voxel is made of. Only the reserved materials at the top of
// the range are named; every value below them is a distinct opaque custom
// material, with meanings to be assigned by the palette. UNDEFINED is never
// stored in generated voxel data; it is the null value returned when the
// value of an undefined voxel is requested.
enum class VoxelMaterial : uint8_t {
	AIR = 254,
	UNDEFINED = 255,
	// The exclusive upper limit of the custom materials, whose range starts
	// at 0, and so also their count.
	CUSTOM_COUNT = AIR,
};

inline bool is_material_opaque(const VoxelMaterial p_material) {
	return p_material < VoxelMaterial::CUSTOM_COUNT;
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
// UNDEFINED, in order to ensure meshes are watertight, manifold, and closed.
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
