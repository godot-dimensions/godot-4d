#pragma once

#include "voxel_data_tree.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#endif

// A 4D volume of voxel data that can be expanded indefinitely.
class VoxelData : public RefCounted {
	GDCLASS(VoxelData, RefCounted);

	VoxelDataTree *_tree = nullptr;
	Ref<VoxelGenerator> _generator;

protected:
	static void _bind_methods();

public:
	// bounds on the currently defined region
	const Rect4i &get_bounds() const { return _tree->get_bounds(); }

	bool is_voxel_defined(const Vector4i &p_voxel) const;
	VoxelValue get_value(const Vector4i &p_voxel) const;
	int get_density(const Vector4i &p_voxel) const;

	// The stored surface normal of the edge from the given voxel to its
	// neighbor one step along the given axis, or Vector4() if no normal is
	// stored for that edge.
	Vector4 get_edge_normal(const Vector4i &p_voxel, const int p_axis) const;

	VoxelData();
	~VoxelData();
};
