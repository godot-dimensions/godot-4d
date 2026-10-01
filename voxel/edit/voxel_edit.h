#pragma once

#include "../../math/rect4i.h"
#include "../data/voxel_edge_data.h"
#include "../data/voxel_material.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#endif

// Virtual base class, produces the voxel content of one edited region to
// write into a VoxelData volume. The interface mirrors VoxelGenerator, but
// an edit only covers the region inside its bounds, and is applied on the
// main thread. Subclasses represent particular shapes of edit.
class VoxelEdit : public RefCounted {
	GDCLASS(VoxelEdit, RefCounted);

protected:
	// The region of voxels the edit changes; voxels outside it are unaffected.
	// Set by each subclass to cover its shape.
	Rect4i _bounds;

	static void _bind_methods();

public:
	const Rect4i &get_bounds() const { return _bounds; }

	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const = 0;

	// The surface data of the edge from the given voxel to its neighbor one
	// step along the given axis: the position along the edge where the
	// surface crosses it, and the surface normal there. Only called for
	// active edges, whose two voxels have different materials. The encoding
	// discards the normal's sign, so either orientation may be produced.
	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const = 0;
};
