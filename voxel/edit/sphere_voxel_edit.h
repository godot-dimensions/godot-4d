#pragma once

#include "voxel_edit.h"

// An edit that fills a hypersphere with one material. Voxels are sampled at
// their centers.
class SphereVoxelEdit : public VoxelEdit {
	GDCLASS(SphereVoxelEdit, VoxelEdit);

	Vector4 _center = Vector4();
	real_t _radius = 1.0;
	VoxelMaterial _material = VoxelMaterial::SOLID;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	Vector4 get_center() const { return _center; }
	void set_center(const Vector4 &p_center);

	real_t get_radius() const { return _radius; }
	void set_radius(const real_t p_radius);

	int get_fill_material() const { return (int)_material; }
	void set_fill_material(const int p_fill_material);

	SphereVoxelEdit() { _update_bounds(); }
	SphereVoxelEdit(const Vector4 &p_center, const real_t p_radius, const VoxelMaterial p_material) :
			_center(p_center),
			_radius(p_radius),
			_material(p_material) {
		_update_bounds();
	}

private:
	void _update_bounds();
};
