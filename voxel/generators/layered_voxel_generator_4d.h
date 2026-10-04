#pragma once

#include "voxel_generator_4d.h"

#if GDEXTENSION
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

// Composes a stack of generators, equivalent to applying each layer as an
// edit with the higher-index layers on top: each voxel takes the material of
// the topmost layer that defines it, so lower layers show through only where
// every layer above leaves the voxel UNDEFINED. With no layers, everything
// is UNDEFINED.
class LayeredVoxelGenerator4D : public VoxelGenerator4D {
	GDCLASS(LayeredVoxelGenerator4D, VoxelGenerator4D);

	Vector<Ref<VoxelGenerator4D>> _layers;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial4D get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData4D get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	Vector<Ref<VoxelGenerator4D>> get_layers() const { return _layers; }
	void set_layers(const Vector<Ref<VoxelGenerator4D>> &p_layers);

	TypedArray<VoxelGenerator4D> get_layers_bind() const;
	void set_layers_bind(const TypedArray<VoxelGenerator4D> &p_layers);
};
