#pragma once

#include "../../math/transform_4d.h"
#include "voxel_edit_4d.h"

// An edit that fills the image of the unit hypercube under a 4D transform
// with one material. Voxels are sampled at their centers. A transform with a
// singular basis encloses no volume and fills nothing.
class ParallelogramVoxelEdit4D : public VoxelEdit4D {
	GDCLASS(ParallelogramVoxelEdit4D, VoxelEdit4D);

	Transform4D _transform;
	VoxelMaterial4D _material = VoxelMaterial4D::AIR;
	// Cached from the transform; only valid while not degenerate.
	Transform4D _inverse_transform;
	bool _degenerate = false;

	bool _contains_local_point(const Vector4 &p_local) const;
	void _update_cache();

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial4D get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData4D get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	Transform4D get_transform() const { return _transform; }
	void set_transform(const Transform4D &p_transform);

	Projection get_basis_bind() const { return _transform.basis; }
	void set_basis_bind(const Projection &p_basis);
	Vector4 get_position() const { return _transform.origin; }
	void set_position(const Vector4 &p_position);

	int get_fill_material() const { return (int)_material; }
	void set_fill_material(const int p_fill_material);

	ParallelogramVoxelEdit4D() { _update_cache(); }
	ParallelogramVoxelEdit4D(const Transform4D &p_transform, const VoxelMaterial4D p_material) :
			_transform(p_transform),
			_material(p_material) {
		ERR_FAIL_COND_MSG(_material == VoxelMaterial4D::UNDEFINED, "ParallelogramVoxelEdit4D cannot apply the UNDEFINED material.");
		_update_cache();
	}
};
