#include "tiger_test_generator.h"

// How quickly density grows with distance from the surface, in density units
// per voxel. Densities saturate at 255 / 64 = ~4 voxels from the surface.
static constexpr double DENSITY_PER_DISTANCE = 64.0;

void TigerTestGenerator::_bind_methods() {
}

VoxelValue TigerTestGenerator::get_value(const Vector4i &p_voxel) const {
	const double xy = Math::sqrt((p_voxel.x + 0.5) * (p_voxel.x + 0.5) + (p_voxel.y + 0.5) * (p_voxel.y + 0.5)) - _major_radius;
	const double zw = Math::sqrt((p_voxel.z + 0.5) * (p_voxel.z + 0.5) + (p_voxel.w + 0.5) * (p_voxel.w + 0.5)) - _major_radius;
	const double signed_distance = _minor_radius - Math::sqrt(xy * xy + zw * zw);
	VoxelValue value;
	value.material = signed_distance > 0.0 ? VoxelMaterial::SOLID : VoxelMaterial::AIR;
	value.density = (uint8_t)MIN(Math::abs(signed_distance) * DENSITY_PER_DISTANCE, 255.0);
	return value;
}

Vector4 TigerTestGenerator::get_normal(const Vector4i &p_voxel, const int p_axis, const VoxelValue &p_value_1, const VoxelValue &p_value_2) const {
	// The surface crosses the edge a / (a + b) of the way along it.
	const double a = p_value_1.density;
	const double b = p_value_2.density;
	const double crossing = a + b > 0.0 ? a / (a + b) : 0.5;
	Vector4 point = Vector4(p_voxel.x + 0.5, p_voxel.y + 0.5, p_voxel.z + 0.5, p_voxel.w + 0.5);
	point[p_axis] += crossing;
	// The analytic gradient of the distance from the tiger's core circles,
	// which points out of the solid tube around them.
	const double r_xy = Math::sqrt(point.x * point.x + point.y * point.y);
	const double r_zw = Math::sqrt(point.z * point.z + point.w * point.w);
	Vector4 gradient = Vector4();
	if (r_xy > 0.0) {
		const double from_circle = r_xy - _major_radius;
		gradient.x = from_circle * point.x / r_xy;
		gradient.y = from_circle * point.y / r_xy;
	}
	if (r_zw > 0.0) {
		const double from_circle = r_zw - _major_radius;
		gradient.z = from_circle * point.z / r_zw;
		gradient.w = from_circle * point.w / r_zw;
	}
	return gradient.normalized();
}
