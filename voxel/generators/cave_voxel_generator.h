#pragma once

#include "voxel_generator.h"
#include "voxel_noise.h"

// Generates tunnel-like caves as the near-zero set of three independent
// noise fields: a continuous function from R^4 to R^3 is typically zero on
// a connected set of dimension 1, which thickens into tunnels where the
// fields' combined magnitude is below a threshold determined by the desired
// typical diameter. Inside the tunnels is the cave material (air by default)
// and outside the outer material (UNDEFINED by default, so that lower layers
// of a LayeredVoxelGenerator show through there).
class CaveVoxelGenerator : public VoxelGenerator {
	GDCLASS(CaveVoxelGenerator, VoxelGenerator);

	LayeredVoxelNoise _noise = { 0, 4, 0.4f, Vector4(8, 6, 8, 8), 1.0f };
	real_t _diameter = 6.0f;
	VoxelMaterial _cave_material = VoxelMaterial::AIR;
	VoxelMaterial _outer_material = VoxelMaterial::UNDEFINED;
	// The squared field magnitude below which a point is inside a tunnel,
	// derived from the diameter and the noise parameters.
	real_t _threshold_squared = 0.0f;

	void _update_threshold();
	real_t _field_squared(const Vector4 &p_point, const bool p_early_out) const;
	Vector4 _field_gradient(const Vector4 &p_point) const;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

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
	real_t get_diameter() const { return _diameter; }
	void set_diameter(const real_t p_diameter);
	int get_cave_material() const { return (int)_cave_material; }
	void set_cave_material(const int p_material);
	int get_outer_material() const { return (int)_outer_material; }
	void set_outer_material(const int p_material);

	CaveVoxelGenerator() { _update_threshold(); }
};
