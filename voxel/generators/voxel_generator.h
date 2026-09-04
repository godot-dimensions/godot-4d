#pragma once

#include "../data/voxel_value.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#endif

// Virtual base class, produces the voxel content that fills a VoxelData volume.
class VoxelGenerator : public RefCounted {
	GDCLASS(VoxelGenerator, RefCounted);

protected:
	static void _bind_methods();

public:
	virtual VoxelValue get_value(const Vector4i &p_voxel) const = 0;

	// The surface normal, pointing out of the solid material, at the point
	// where the surface crosses the edge from the given voxel to its neighbor
	// one step along the given axis. The values of those two voxels are
	// passed in as p_value_1 and p_value_2.
	virtual Vector4 get_normal(const Vector4i &p_voxel, const int p_axis, const VoxelValue &p_value_1, const VoxelValue &p_value_2) const = 0;
};
