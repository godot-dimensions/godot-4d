#pragma once

#include "voxel_edit_4d.h"

// An edit that fills a hypersphere with one material. Voxels are sampled at
// their centers.
class SphereVoxelEdit4D : public VoxelEdit4D {
	GDCLASS(SphereVoxelEdit4D, VoxelEdit4D);

	Vector4 _center = Vector4();
	real_t _radius = 1.0;
	VoxelMaterial4D _material = VoxelMaterial4D::AIR;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial4D get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData4D get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	Vector4 get_center() const { return _center; }
	void set_center(const Vector4 &p_center);

	real_t get_radius() const { return _radius; }
	void set_radius(const real_t p_radius);

	int get_fill_material() const { return (int)_material; }
	void set_fill_material(const int p_fill_material);

	SphereVoxelEdit4D() { _update_bounds(); }
	SphereVoxelEdit4D(const Vector4 &p_center, const real_t p_radius, const VoxelMaterial4D p_material) :
			_center(p_center),
			_radius(p_radius),
			_material(p_material) {
		_update_bounds();
	}

private:
	void _update_bounds();
};
