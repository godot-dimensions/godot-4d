#pragma once

#include "voxel_edit.h"

// An edit that fills a hypersphere with one material. Voxels are sampled at
// their centers.
class SphereVoxelEdit : public VoxelEdit {
	GDCLASS(SphereVoxelEdit, VoxelEdit);

	Vector4 _center = Vector4();
	double _radius = 1.0;
	VoxelMaterial _material = VoxelMaterial::SOLID;

protected:
	static void _bind_methods();

public:
	virtual VoxelValue get_value(const Vector4i &p_voxel) const override;
	virtual Vector4 get_normal(const Vector4i &p_voxel, const int p_axis, const VoxelValue &p_value_1, const VoxelValue &p_value_2) const override;

	Vector4 get_center() const { return _center; }
	void set_center(const Vector4 &p_center);

	double get_radius() const { return _radius; }
	void set_radius(const double p_radius);

	int get_material() const { return (int)_material; }
	void set_material(const int p_material);

	SphereVoxelEdit() { _update_bounds(); }
	SphereVoxelEdit(const Vector4 &p_center, const double p_radius, const VoxelMaterial p_material) :
			_center(p_center),
			_radius(p_radius),
			_material(p_material) {
		_update_bounds();
	}

private:
	void _update_bounds();
};
