#pragma once

#include "voxel_generator_4d.h"

// Generates one material filling a vertically aligned cylinder (a spherinder:
// a sphere over the three horizontal axes times a vertical interval) and
// another filling the rest of space. The inner material defaults to UNDEFINED
// and the outer to air, carving an open region out of a backdrop of air for
// other layers of a LayeredVoxelGenerator4D to fill.
class CylinderVoxelGenerator4D : public VoxelGenerator4D {
	GDCLASS(CylinderVoxelGenerator4D, VoxelGenerator4D);

	Vector4 _center;
	real_t _height = 40.0f;
	real_t _radius = 20.0f;
	VoxelMaterial4D _material_inner = VoxelMaterial4D::UNDEFINED;
	VoxelMaterial4D _material_outer = VoxelMaterial4D::AIR;

	bool _contains_point(const Vector4 &p_point) const;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial4D get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData4D get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	Vector4 get_center() const { return _center; }
	void set_center(const Vector4 &p_center);
	real_t get_height() const { return _height; }
	void set_height(const real_t p_height);
	real_t get_radius() const { return _radius; }
	void set_radius(const real_t p_radius);

	int get_material_inner() const { return (int)_material_inner; }
	void set_material_inner(const int p_material);
	int get_material_outer() const { return (int)_material_outer; }
	void set_material_outer(const int p_material);
};
