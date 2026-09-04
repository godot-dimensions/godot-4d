#pragma once

#include "voxel_generator.h"

// Temporary test generator producing a tiger: the product of circles of the
// major radius in the XY and ZW planes, thickened into a tube of the minor
// radius. Voxels are sampled at their centers.
class TigerTestGenerator : public VoxelGenerator {
	GDCLASS(TigerTestGenerator, VoxelGenerator);

	double _major_radius = 10.0;
	double _minor_radius = 4.5;

protected:
	static void _bind_methods();

public:
	virtual VoxelValue get_value(const Vector4i &p_voxel) const override;
	virtual Vector4 get_normal(const Vector4i &p_voxel, const int p_axis, const VoxelValue &p_value_1, const VoxelValue &p_value_2) const override;

	TigerTestGenerator() {}
	TigerTestGenerator(const double p_major_radius, const double p_minor_radius) :
			_major_radius(p_major_radius),
			_minor_radius(p_minor_radius) {}
};
