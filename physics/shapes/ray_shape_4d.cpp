#include "ray_shape_4d.h"

#include "../../model/wire/array_wire_mesh_4d.h"

Rect4 RayShape4D::get_rect_bounds(const Transform4D &p_to_target) const {
	Rect4 rect_bounds = Rect4(p_to_target.origin, Vector4(0, 0, 0, 0));
	rect_bounds.expand_self_to_point(p_to_target.basis.y * _ray_target_y + p_to_target.origin);
	return rect_bounds;
}

Vector4 RayShape4D::get_nearest_point(const Vector4 &p_point) const {
	real_t clamped_y;
	if (likely(_ray_target_y < 0.0f)) {
		clamped_y = CLAMP(p_point.y, _ray_target_y, 0.0f);
	} else {
		clamped_y = CLAMP(p_point.y, 0.0f, _ray_target_y);
	}
	return Vector4(0.0f, clamped_y, 0.0f, 0.0f);
}

Vector4 RayShape4D::get_support_point(const Vector4 &p_direction) const {
	if (p_direction.y > 0.0f) {
		return Vector4(0, 0, 0, 0);
	}
	return Vector4(0, _ray_target_y, 0, 0);
}

bool RayShape4D::has_point(const Vector4 &p_point) const {
	if (!(Math::is_zero_approx(p_point.x) && Math::is_zero_approx(p_point.z) && Math::is_zero_approx(p_point.w))) {
		return false;
	}
	real_t clamped_y;
	if (likely(_ray_target_y < 0.0f)) {
		clamped_y = CLAMP(p_point.y, _ray_target_y, 0.0f);
	} else {
		clamped_y = CLAMP(p_point.y, 0.0f, _ray_target_y);
	}
	return Math::is_equal_approx(p_point.y, clamped_y);
}

Ref<WireMesh4D> RayShape4D::to_wire_mesh() const {
	Ref<ArrayWireMesh4D> ray_wire_mesh;
	ray_wire_mesh.instantiate();
	ray_wire_mesh->append_edge_points(Vector4(0, 0, 0, 0), Vector4(0, _ray_target_y, 0, 0));
	ray_wire_mesh->append_edge_points(Vector4(_ray_target_y * +0.25, _ray_target_y * 0.75, 0, 0), Vector4(0, _ray_target_y, 0, 0));
	ray_wire_mesh->append_edge_points(Vector4(_ray_target_y * -0.25, _ray_target_y * 0.75, 0, 0), Vector4(0, _ray_target_y, 0, 0));
	ray_wire_mesh->append_edge_points(Vector4(0, _ray_target_y * 0.75, _ray_target_y * +0.25, 0), Vector4(0, _ray_target_y, 0, 0));
	ray_wire_mesh->append_edge_points(Vector4(0, _ray_target_y * 0.75, _ray_target_y * -0.25, 0), Vector4(0, _ray_target_y, 0, 0));
	ray_wire_mesh->append_edge_points(Vector4(0, _ray_target_y * 0.75, 0, _ray_target_y * +0.25), Vector4(0, _ray_target_y, 0, 0));
	ray_wire_mesh->append_edge_points(Vector4(0, _ray_target_y * 0.75, 0, _ray_target_y * -0.25), Vector4(0, _ray_target_y, 0, 0));
	return ray_wire_mesh;
}

void RayShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ray_length"), &RayShape4D::get_ray_length);
	ClassDB::bind_method(D_METHOD("set_ray_length", "length"), &RayShape4D::set_ray_length);
	ClassDB::bind_method(D_METHOD("get_ray_target_y"), &RayShape4D::get_ray_target_y);
	ClassDB::bind_method(D_METHOD("set_ray_target_y", "target_y"), &RayShape4D::set_ray_target_y);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ray_length", PROPERTY_HINT_RANGE, "0.001,1000,or_greater"), "set_ray_length", "get_ray_length");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ray_target_y", PROPERTY_HINT_RANGE, "-1000,-0.001,or_less", PROPERTY_USAGE_NONE), "set_ray_target_y", "get_ray_target_y");
}
