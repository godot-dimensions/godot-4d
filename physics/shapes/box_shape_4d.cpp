#include "box_shape_4d.h"

#include "../../model/mesh/poly/box_poly_mesh_4d.h"
#include "../../model/mesh/tetra/box_tetra_mesh_4d.h"
#include "../../model/mesh/wire/box_wire_mesh_4d.h"

Vector4 BoxShape4D::get_half_extents() const {
	return _size * 0.5f;
}

void BoxShape4D::set_half_extents(const Vector4 &p_half_extents) {
	_size = p_half_extents * 2.0f;
}

Vector4 BoxShape4D::get_size() const {
	return _size;
}

void BoxShape4D::set_size(const Vector4 &p_size) {
	_size = p_size;
}

real_t BoxShape4D::get_hypervolume() const {
	return _size.x * _size.y * _size.z * _size.w;
}

real_t BoxShape4D::get_surface_volume() const {
	return 2.0f * (_size.x * _size.y * _size.z + _size.x * _size.y * _size.w + _size.x * _size.z * _size.w + _size.y * _size.z * _size.w);
}

Rect4 BoxShape4D::get_rect_bounds(const Transform4D &p_to_target) const {
	Rect4 bounds = Rect4(p_to_target.origin, Vector4());
	const Vector4 half_extents = get_half_extents();
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 2; z++) {
				for (int w = 0; w < 2; w++) {
					const Vector4 point = Vector4(
							x == 0 ? -half_extents.x : half_extents.x,
							y == 0 ? -half_extents.y : half_extents.y,
							z == 0 ? -half_extents.z : half_extents.z,
							w == 0 ? -half_extents.w : half_extents.w);
					bounds = bounds.expand_to_point(p_to_target * point);
				}
			}
		}
	}
	return bounds;
}

Rect4 BoxShape4D::get_rect_bounds_fast() const {
	const Vector4 half_extents = get_half_extents();
	return Rect4(-half_extents, _size);
}

Dictionary BoxShape4D::raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance, const bool p_inside_is_zero) const {
	const Rect4 bounds = get_rect_bounds_fast();
	if (p_inside_is_zero && bounds.has_point(p_local_from)) {
		Dictionary inside_result;
		inside_result["hit"] = true;
		inside_result["distance"] = 0.0f;
		inside_result["normal"] = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		inside_result["point"] = p_local_from;
		return inside_result;
	}
	// Pass false here because we handle the inside case above (avoid unnecessary calls and checks).
	return bounds.raycast_intersects_dict(p_local_from, p_local_direction, p_max_distance, false);
}

real_t BoxShape4D::get_signed_distance_to_surface(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_surface) const {
	const Vector4 half_extents = get_half_extents();
	const Vector4 abs_point = p_local_point.abs();
	const Vector4 abs_to_surface = abs_point - half_extents;
	const Vector4 abs_to_surface_abs = abs_to_surface.abs();
	real_t nearest_distance_signed = Math_INF;
	real_t nearest_distance_abs = Math_INF;
	int8_t nearest_axis = 0;
	for (int8_t axis = 0; axis < 4; axis++) {
		if (abs_to_surface_abs[axis] < nearest_distance_abs) {
			nearest_distance_signed = abs_to_surface[axis];
			nearest_distance_abs = abs_to_surface_abs[axis];
			nearest_axis = axis;
		}
	}
	if (r_nearest_point_on_surface != nullptr) {
		Vector4 nearest_point = Vector4(
				CLAMP(p_local_point.x, -half_extents.x, half_extents.x),
				CLAMP(p_local_point.y, -half_extents.y, half_extents.y),
				CLAMP(p_local_point.z, -half_extents.z, half_extents.z),
				CLAMP(p_local_point.w, -half_extents.w, half_extents.w));
		nearest_point[nearest_axis] = (p_local_point[nearest_axis] > 0.0f) ? half_extents[nearest_axis] : -half_extents[nearest_axis];
		*r_nearest_point_on_surface = nearest_point;
	}
	return nearest_distance_signed;
}

Vector4 BoxShape4D::get_nearest_point(const Vector4 &p_local_point) const {
	const Vector4 half_extents = get_half_extents();
	return Vector4(
			CLAMP(p_local_point.x, -half_extents.x, half_extents.x),
			CLAMP(p_local_point.y, -half_extents.y, half_extents.y),
			CLAMP(p_local_point.z, -half_extents.z, half_extents.z),
			CLAMP(p_local_point.w, -half_extents.w, half_extents.w));
}

Vector4 BoxShape4D::get_support_point(const Vector4 &p_local_direction) const {
	const Vector4 half_extents = get_half_extents();
	return Vector4(
			(p_local_direction.x > 0.0f) ? half_extents.x : -half_extents.x,
			(p_local_direction.y > 0.0f) ? half_extents.y : -half_extents.y,
			(p_local_direction.z > 0.0f) ? half_extents.z : -half_extents.z,
			(p_local_direction.w > 0.0f) ? half_extents.w : -half_extents.w);
}

bool BoxShape4D::has_point(const Vector4 &p_local_point) const {
	const Vector4 abs_point = p_local_point.abs();
	const Vector4 half_extents = get_half_extents();
	return abs_point.x <= half_extents.x && abs_point.y <= half_extents.y && abs_point.z <= half_extents.z && abs_point.w <= half_extents.w;
}

bool BoxShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<BoxShape4D> box_shape = p_shape;
	if (box_shape.is_null()) {
		return false;
	}
	return _size == box_shape->get_size();
}

Ref<PolyMesh4D> BoxShape4D::to_poly_mesh(const Dictionary &p_options) const {
	Ref<BoxPolyMesh4D> poly_mesh;
	poly_mesh.instantiate();
	poly_mesh->set_size(_size);
	return poly_mesh;
}

Ref<TetraMesh4D> BoxShape4D::to_tetra_mesh(const Dictionary &p_options) const {
	Ref<BoxTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(_size);
	return tetra_mesh;
}

Ref<WireMesh4D> BoxShape4D::to_wire_mesh(const Dictionary &p_options) const {
	Ref<BoxWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	return wire_mesh;
}

void BoxShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &BoxShape4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &BoxShape4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &BoxShape4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxShape4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");
}
