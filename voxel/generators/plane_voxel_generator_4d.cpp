#include "plane_voxel_generator_4d.h"

VoxelMaterial4D PlaneVoxelGenerator4D::get_material(const Vector4i &p_voxel) const {
	return _plane.is_point_over(get_voxel_center(p_voxel)) ? _material_over : _material_under;
}

VoxelEdgeData4D PlaneVoxelGenerator4D::get_edge_data(const Vector4i &p_voxel, const int p_axis) const {
	const Vector4 center = get_voxel_center(p_voxel);
	// The exact crossing of the edge running from the voxel's center one step
	// along the axis. The denominator cannot be zero on an active edge, whose
	// two ends the plane separates.
	const real_t position = -_plane.distance_to(center) / _plane.normal[p_axis];
	return VoxelEdgeData4D(_plane.normal.normalized(), position);
}

void PlaneVoxelGenerator4D::set_plane(const Plane4D &p_plane) {
	_plane = p_plane;
	emit_changed();
}

void PlaneVoxelGenerator4D::set_normal(const Vector4 &p_normal) {
	_plane.normal = p_normal;
	emit_changed();
}

void PlaneVoxelGenerator4D::set_distance(const real_t p_distance) {
	_plane.distance = p_distance;
	emit_changed();
}

void PlaneVoxelGenerator4D::set_material_over(const int p_material) {
	ERR_FAIL_COND_MSG((int)(VoxelMaterial4D)p_material != p_material, "PlaneVoxelGenerator4D materials must fit in the material range. Refusing to set.");
	_material_over = (VoxelMaterial4D)p_material;
	emit_changed();
}

void PlaneVoxelGenerator4D::set_material_under(const int p_material) {
	ERR_FAIL_COND_MSG((int)(VoxelMaterial4D)p_material != p_material, "PlaneVoxelGenerator4D materials must fit in the material range. Refusing to set.");
	_material_under = (VoxelMaterial4D)p_material;
	emit_changed();
}

void PlaneVoxelGenerator4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_normal"), &PlaneVoxelGenerator4D::get_normal);
	ClassDB::bind_method(D_METHOD("set_normal", "normal"), &PlaneVoxelGenerator4D::set_normal);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "normal"), "set_normal", "get_normal");

	ClassDB::bind_method(D_METHOD("get_distance"), &PlaneVoxelGenerator4D::get_distance);
	ClassDB::bind_method(D_METHOD("set_distance", "distance"), &PlaneVoxelGenerator4D::set_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance"), "set_distance", "get_distance");

	ClassDB::bind_method(D_METHOD("get_material_over"), &PlaneVoxelGenerator4D::get_material_over);
	ClassDB::bind_method(D_METHOD("set_material_over", "material"), &PlaneVoxelGenerator4D::set_material_over);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material_over", PROPERTY_HINT_RANGE, "0,255,1"), "set_material_over", "get_material_over");

	ClassDB::bind_method(D_METHOD("get_material_under"), &PlaneVoxelGenerator4D::get_material_under);
	ClassDB::bind_method(D_METHOD("set_material_under", "material"), &PlaneVoxelGenerator4D::set_material_under);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "material_under", PROPERTY_HINT_RANGE, "0,255,1"), "set_material_under", "get_material_under");
}
