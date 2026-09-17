#pragma once

#include "../../math/rect4i.h"
#include "../generators/voxel_generator.h"

// Virtual base class, produces the voxel content of one edited region to
// write into a VoxelData volume. An edit works exactly like the VoxelGenerator
// it is, except that it only covers the region inside its bounds rather than
// a whole world. Subclasses represent particular shapes of edit.
class VoxelEdit : public VoxelGenerator {
	GDCLASS(VoxelEdit, VoxelGenerator);

protected:
	// The region of voxels the edit changes; voxels outside it are unaffected.
	// Set by each subclass to cover its shape.
	Rect4i _bounds;

	static void _bind_methods();

public:
	const Rect4i &get_bounds() const { return _bounds; }
};
