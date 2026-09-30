#pragma once

#include "../../math/plane_4d.h"
#include "voxel_generator.h"

// Generates one material filling the half-space over the plane and another
// filling the half-space under it.
class PlaneVoxelGenerator : public VoxelGenerator {
	GDCLASS(PlaneVoxelGenerator, VoxelGenerator);

	Plane4D _plane;
	VoxelMaterial _material_over = VoxelMaterial::UNDEFINED;
	VoxelMaterial _material_under = VoxelMaterial::UNDEFINED;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	Plane4D get_plane() const { return _plane; }
	void set_plane(const Plane4D &p_plane);

	Vector4 get_normal() const { return _plane.normal; }
	void set_normal(const Vector4 &p_normal);
	real_t get_distance() const { return _plane.distance; }
	void set_distance(const real_t p_distance);

	int get_material_over() const { return (int)_material_over; }
	void set_material_over(const int p_material);
	int get_material_under() const { return (int)_material_under; }
	void set_material_under(const int p_material);
};
