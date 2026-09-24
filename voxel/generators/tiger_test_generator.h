#pragma once

#include "voxel_generator.h"

// Temporary test generator producing a tiger: the product of circles of the
// major radius in the XY and ZW planes, thickened into a tube of the minor
// radius. Voxels are sampled at their centers.
class TigerTestGenerator : public VoxelGenerator {
	GDCLASS(TigerTestGenerator, VoxelGenerator);

	real_t _major_radius = 10.0;
	real_t _minor_radius = 4.5;

	real_t _signed_distance(const Vector4 &p_point) const;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	TigerTestGenerator() {}
	TigerTestGenerator(const real_t p_major_radius, const real_t p_minor_radius) :
			_major_radius(p_major_radius),
			_minor_radius(p_minor_radius) {}
};
