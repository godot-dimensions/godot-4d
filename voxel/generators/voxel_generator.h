#pragma once

#include "../data/voxel_edge_data.h"
#include "../data/voxel_material.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#endif

// Virtual base class, produces the voxel content that fills a VoxelData volume.
// Chunks are generated on worker threads, so implementations must be safe to
// call from several threads at once.
class VoxelGenerator : public RefCounted {
	GDCLASS(VoxelGenerator, RefCounted);

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const = 0;

	// The surface data of the edge from the given voxel to its neighbor one
	// step along the given axis: the position along the edge where the
	// surface crosses it, and the surface normal there. Only called for
	// active edges, whose two voxels have different materials. The encoding
	// discards the normal's sign, so either orientation may be produced.
	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const = 0;
};
