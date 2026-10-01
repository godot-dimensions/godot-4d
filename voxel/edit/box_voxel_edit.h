#pragma once

#include "../../math/rect4.h"
#include "voxel_edit.h"

// An edit that fills an axis-aligned box with one material. Voxels are
// sampled at their centers.
class BoxVoxelEdit : public VoxelEdit {
	GDCLASS(BoxVoxelEdit, VoxelEdit);

	Rect4 _rect = Rect4(Vector4(), Vector4(1, 1, 1, 1));
	VoxelMaterial _material = VoxelMaterial::AIR;

	bool _contains_point(const Vector4 &p_point) const;
	void _update_bounds();

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	Rect4 get_rect() const { return _rect; }
	void set_rect(const Rect4 &p_rect);

	Vector4 get_position() const { return _rect.position; }
	void set_position(const Vector4 &p_position);
	Vector4 get_size() const { return _rect.size; }
	void set_size(const Vector4 &p_size);

	int get_fill_material() const { return (int)_material; }
	void set_fill_material(const int p_fill_material);

	BoxVoxelEdit() { _update_bounds(); }
	BoxVoxelEdit(const Rect4 &p_rect, const VoxelMaterial p_material) :
			_rect(p_rect),
			_material(p_material) {
		_update_bounds();
	}
};
