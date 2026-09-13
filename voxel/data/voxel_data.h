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
	// Whether every voxel in the given region is defined. Regions reaching
	// outside the defined bounds are not fully defined.
	bool is_region_defined(const Rect4i &p_region) const;
	VoxelValue get_value(const Vector4i &p_voxel) const;
	int get_density(const Vector4i &p_voxel) const;

	// The stored surface normal of the edge from the given voxel to its
	// neighbor one step along the given axis, or Vector4() if no normal is
	// stored for that edge.
	Vector4 get_edge_normal(const Vector4i &p_voxel, const int p_axis) const;

	// The lowest voxel of the data chunk containing the given voxel. Chunks
	// are aligned to a chunk-sized grid anchored at the bounds.
	Vector4i get_chunk_position(const Vector4i &p_voxel) const;

	// Generates a detached chunk-sized tree with the content of the chunk
	// containing the given voxel. This only reads the generator, so it can
	// run on a worker thread. The caller owns the returned tree.
	VoxelDataTree *generate_chunk_content(const Vector4i &p_voxel) const;

	// Grafts a generated chunk into the data, taking ownership of it. The
	// chunk is discarded if that chunk of the data is already defined. Main
	// thread only.
	void apply_generated_chunk(VoxelDataTree *p_chunk);

	// Immediately generates every chunk inside the bounds.
	void load_all_chunks();

	VoxelData();
	~VoxelData();
};
