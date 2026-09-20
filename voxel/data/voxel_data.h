#pragma once

#include "voxel_data_tree.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#endif

class VoxelEdit;

// A 4D volume of voxel data that can be expanded indefinitely.
class VoxelData : public RefCounted {
	GDCLASS(VoxelData, RefCounted);

	// Null indicates no voxels are defined at all.
	VoxelDataTree *_tree = nullptr;
	Ref<VoxelGenerator> _generator;

protected:
	static void _bind_methods();

public:
	// bounds on the currently defined region
	Rect4i get_bounds() const { return _tree == nullptr ? Rect4i() : _tree->get_bounds(); }

	bool is_voxel_defined(const Vector4i &p_voxel) const;
	// Whether every voxel in the given region is defined. Regions reaching
	// outside the defined bounds are not fully defined.
	bool is_region_defined(const Rect4i &p_region) const;
	VoxelMaterial get_material(const Vector4i &p_voxel) const;

	// The stored surface data of the edge from the given voxel to its
	// neighbor one step along the given axis, still encoded, or arbitrary data
	// if none is stored for that edge.
	VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const;

	// The lowest voxel of the data chunk containing the given voxel. Chunks
	// are aligned to the global chunk-sized grid anchored at the origin.
	Vector4i get_chunk_position(const Vector4i &p_voxel) const;

	// The tree node whose bounds are exactly the given region, or null if no
	// node has those bounds. The node stays owned by the data, and is only
	// valid until the data is next modified.
	const VoxelDataTree *find_region_node(const Rect4i &p_region) const;

	// Generates a detached chunk-sized tree with the content of the chunk
	// containing the given voxel. This only reads the generator, so it can
	// run on a worker thread. The caller owns the returned tree.
	VoxelDataTree *generate_chunk_content(const Vector4i &p_voxel) const;

	// Grafts a generated chunk into the data, taking ownership of it. The
	// chunk is discarded if that chunk of the data is already defined. Main
	// thread only.
	void apply_generated_chunk(VoxelDataTree *p_chunk);

	// Overlays the edit onto the defined parts of the data; parts of the edit
	// over undefined chunks are discarded. Main thread only.
	void apply_edit(const Ref<VoxelEdit> &p_edit);

	// Makes the data chunk containing the given voxel undefined again.
	// Returns whether any data was unloaded. Main thread only.
	bool unload_chunk(const Vector4i &p_voxel);

	// Immediately generates a fixed 8x8x8x8-chunk region centered
	// on the origin, for testing purposes.
	void load_all_chunks();

private:
	// Shrinks the tree to the minimum size it needs to fit the defined data
	// (while properly aligned).
	void trim_bounds();

	// Doubles the tree's bounds, moving the content into the new root. Extends
	// the tree towards the given location, to the extent permitted by the
	// alignment invariant.
	void expand_bounds(const Vector4i &p_toward);

public:
	VoxelData();
	~VoxelData();
};
