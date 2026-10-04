#pragma once

#include "../../godot_4d_defines.h"

// The material a voxel is made of. Only the reserved materials at the top of
// the range are named; every value below them is a distinct opaque custom
// material, with meanings to be assigned by the palette. UNDEFINED is never
// stored in generated voxel data; it is the null value returned when the
// value of an undefined voxel is requested.
enum class VoxelMaterial4D : uint8_t {
	AIR = 254,
	UNDEFINED = 255,
	// The exclusive upper limit of the custom materials, whose range starts
	// at 0, and so also their count.
	CUSTOM_COUNT = AIR,
};

inline bool is_material_opaque(const VoxelMaterial4D p_material) {
	return p_material < VoxelMaterial4D::CUSTOM_COUNT;
}

inline VoxelMaterial4D overlay_material(const VoxelMaterial4D p_over, const VoxelMaterial4D p_under) {
	return p_over == VoxelMaterial4D::UNDEFINED ? p_under : p_over;
}

// Whether a face of the surface mesh separates two adjacent voxels, and which
// of them its normal points toward.
enum class VoxelFace4D {
	NONE,
	TOWARD_FIRST,
	TOWARD_SECOND,
};

// The face needed between adjacent voxels of the two materials, depending on
// their opacity.
// While this may be extended to other cases later, the relation of having no
// face should always remain an equivalence relation among materials other than
// UNDEFINED, in order to ensure meshes are watertight, manifold, and closed.
inline VoxelFace4D get_face_between(const VoxelMaterial4D p_first, const VoxelMaterial4D p_second) {
	if (p_first == VoxelMaterial4D::UNDEFINED || p_second == VoxelMaterial4D::UNDEFINED) {
		return VoxelFace4D::NONE;
	}
	const bool first_opaque = is_material_opaque(p_first);
	if (first_opaque == is_material_opaque(p_second)) {
		return VoxelFace4D::NONE;
	}
	return first_opaque ? VoxelFace4D::TOWARD_SECOND : VoxelFace4D::TOWARD_FIRST;
}
