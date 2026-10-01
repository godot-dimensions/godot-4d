#pragma once

#include "voxel_generator.h"

// Generates one material filling an axis-aligned box and another filling the
// rest of space. The inner material defaults to UNDEFINED and the outer to
// air, carving an open region out of a backdrop of air for other layers of a
// LayeredVoxelGenerator to fill.
class BoxVoxelGenerator : public VoxelGenerator {
	GDCLASS(BoxVoxelGenerator, VoxelGenerator);

	Vector4 _center;
	Vector4 _size = Vector4(20.0f, 20.0f, 20.0f, 20.0f);
	VoxelMaterial _material_inner = VoxelMaterial::UNDEFINED;
	VoxelMaterial _material_outer = VoxelMaterial::AIR;

	bool _contains_point(const Vector4 &p_point) const;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	Vector4 get_center() const { return _center; }
	void set_center(const Vector4 &p_center);
	Vector4 get_size() const { return _size; }
	void set_size(const Vector4 &p_size);

	int get_material_inner() const { return (int)_material_inner; }
	void set_material_inner(const int p_material);
	int get_material_outer() const { return (int)_material_outer; }
	void set_material_outer(const int p_material);
};
