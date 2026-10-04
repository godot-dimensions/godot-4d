#pragma once

#include "voxel_generator_4d.h"

// Clips a modifier generator to the shape of a base generator: where the
// base is undefined, the result is always undefined, and where it's defined,
// the modifier's material may overwrite it.
class ClippedVoxelGenerator4D : public VoxelGenerator4D {
	GDCLASS(ClippedVoxelGenerator4D, VoxelGenerator4D);

	Ref<VoxelGenerator4D> _base;
	Ref<VoxelGenerator4D> _modifier;

	static VoxelMaterial4D _clipped_material(const VoxelMaterial4D p_base, const VoxelMaterial4D p_modifier);
	static int _transition_significance(const VoxelMaterial4D p_before, const VoxelMaterial4D p_after);

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial4D get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData4D get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	Ref<VoxelGenerator4D> get_base() const { return _base; }
	void set_base(const Ref<VoxelGenerator4D> &p_base);
	Ref<VoxelGenerator4D> get_modifier() const { return _modifier; }
	void set_modifier(const Ref<VoxelGenerator4D> &p_modifier);
};
