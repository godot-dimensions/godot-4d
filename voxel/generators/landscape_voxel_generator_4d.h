#pragma once

#include "layered_voxel_noise_4d.h"
#include "voxel_generator_4d.h"

// Generates noise-based landscape terrain from a height field: a point's
// relative height is its Y coordinate minus layered 4D value noise, so the
// noise displaces the terrain vertically by up to about the intensity. The
// ascending cutoffs are heights in voxels: band i, below the i-th cutoff,
// is filled with the i-th material, from the deepest band up, and the last
// material fills the band above every cutoff. Bands beyond the materials
// list, or everywhere while the list is empty, are UNDEFINED.
class LandscapeVoxelGenerator4D : public VoxelGenerator4D {
	GDCLASS(LandscapeVoxelGenerator4D, VoxelGenerator4D);

	LayeredVoxelNoise4D _noise = { 0, 5, 0.65f, Vector4(3.5f, 8, 3.5f, 3.5f), 12.0f };
	PackedInt32Array _materials;
	PackedFloat32Array _material_cutoffs;

	real_t _relative_height(const Vector4 &p_point, Vector4 *r_gradient) const;
	int _band_for_height(const real_t p_height) const;
	VoxelMaterial4D _material_for_height(const real_t p_height, real_t *r_cutoff) const;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial4D get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData4D get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	int get_seed() const { return (int)_noise.seed; }
	void set_seed(const int p_seed);
	int get_octaves() const { return _noise.octaves; }
	void set_octaves(const int p_octaves);
	real_t get_persistence() const { return _noise.persistence; }
	void set_persistence(const real_t p_persistence);
	real_t get_vertical_scale() const { return _noise.scale.y; }
	void set_vertical_scale(const real_t p_vertical_scale);
	real_t get_horizontal_scale() const { return _noise.scale.x; }
	void set_horizontal_scale(const real_t p_horizontal_scale);
	real_t get_intensity() const { return _noise.intensity; }
	void set_intensity(const real_t p_intensity);
	PackedInt32Array get_materials() const { return _materials; }
	void set_materials(const PackedInt32Array &p_materials);
	PackedFloat32Array get_material_cutoffs() const { return _material_cutoffs; }
	void set_material_cutoffs(const PackedFloat32Array &p_material_cutoffs);
};
