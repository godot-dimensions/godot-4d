#pragma once

#include "voxel_generator.h"

// Clips a modifier generator to the shape of a base generator: where the
// base is undefined, the result is always undefined, and where it's defined,
// the modifier's material may overwrite it.
class ClippedVoxelGenerator : public VoxelGenerator {
	GDCLASS(ClippedVoxelGenerator, VoxelGenerator);

	Ref<VoxelGenerator> _base;
	Ref<VoxelGenerator> _modifier;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	Ref<VoxelGenerator> get_base() const { return _base; }
	void set_base(const Ref<VoxelGenerator> &p_base);
	Ref<VoxelGenerator> get_modifier() const { return _modifier; }
	void set_modifier(const Ref<VoxelGenerator> &p_modifier);
};
