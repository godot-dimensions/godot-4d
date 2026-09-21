#include "tiger_test_generator.h"

void TigerTestGenerator::_bind_methods() {
}

real_t TigerTestGenerator::_signed_distance(const Vector4 &p_point) const {
	const real_t xy = Math::sqrt(p_point.x * p_point.x + p_point.y * p_point.y) - _major_radius;
	const real_t zw = Math::sqrt(p_point.z * p_point.z + p_point.w * p_point.w) - _major_radius;
	return _minor_radius - Math::sqrt(xy * xy + zw * zw);
}

VoxelMaterial TigerTestGenerator::get_material(const Vector4i &p_voxel) const {
	const Vector4 point = Vector4(p_voxel.x + 0.5, p_voxel.y + 0.5, p_voxel.z + 0.5, p_voxel.w + 0.5);
	return _signed_distance(point) > 0.0 ? VoxelMaterial::SOLID : VoxelMaterial::AIR;
}

VoxelEdgeData TigerTestGenerator::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	Vector4 point = Vector4(p_voxel.x + 0.5, p_voxel.y + 0.5, p_voxel.z + 0.5, p_voxel.w + 0.5);
	Vector4 neighbor_point = point;
	neighbor_point[p_axis] += 1.0;
	const real_t distance_1 = _signed_distance(point);
	const real_t distance_2 = _signed_distance(neighbor_point);
	const real_t crossing = distance_1 / (distance_1 - distance_2);
	point[p_axis] += crossing;
	// The analytic gradient of the distance from the tiger's core circles.
	const real_t r_xy = Math::sqrt(point.x * point.x + point.y * point.y);
	const real_t r_zw = Math::sqrt(point.z * point.z + point.w * point.w);
	Vector4 gradient = Vector4();
	if (r_xy > 0.0) {
		const real_t from_circle = r_xy - _major_radius;
		gradient.x = from_circle * point.x / r_xy;
		gradient.y = from_circle * point.y / r_xy;
	}
	if (r_zw > 0.0) {
		const real_t from_circle = r_zw - _major_radius;
		gradient.z = from_circle * point.z / r_zw;
		gradient.w = from_circle * point.w / r_zw;
	}
	return VoxelEdgeData::encode(gradient, crossing);
}
