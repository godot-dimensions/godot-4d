#pragma once

#include "voxel_generator.h"

// Temporary test generator producing a tiger: the product of circles of the
// major radius in the XY and ZW planes, thickened into a tube of the minor
// radius. Voxels are sampled at their centers.
class TigerTestGenerator : public VoxelGenerator {
	GDCLASS(TigerTestGenerator, VoxelGenerator);

	double _major_radius = 10.0;
	double _minor_radius = 4.5;

	double _signed_distance(const Vector4 &p_point) const;

protected:
	static void _bind_methods();

public:
	virtual VoxelMaterial get_material(const Vector4i &p_voxel) const override;
	virtual VoxelEdgeData get_edge_data(const Vector4i &p_voxel, const int p_axis) const override;

	TigerTestGenerator() {}
	TigerTestGenerator(const double p_major_radius, const double p_minor_radius) :
			_major_radius(p_major_radius),
			_minor_radius(p_minor_radius) {}
};
