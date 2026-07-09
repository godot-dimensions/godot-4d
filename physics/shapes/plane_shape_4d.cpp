#include "plane_shape_4d.h"

#include "../../model/mesh/wire/wire_mesh_builder_4d.h"

Rect4 PlaneShape4D::get_rect_bounds(const Transform4D &p_to_target) const {
	const Vector4 normal = p_to_target.basis.y.normalized();
	Vector4 rect_start = Vector4(PLANE_START, PLANE_START, PLANE_START, PLANE_START);
	Vector4 rect_size = Vector4(PLANE_WIDTH, PLANE_WIDTH, PLANE_WIDTH, PLANE_WIDTH);
	if (Math::is_zero_approx(normal.x)) {
		if (Math::is_zero_approx(normal.y)) {
			if (Math::is_zero_approx(normal.z)) {
				if (Math::is_zero_approx(normal.w)) {
					// Do nothing, return the fallback rect.
				} else {
					rect_size.w = PLANE_THICKNESS;
					if (normal.w > 0.0f) {
						rect_start.w = p_to_target.origin.w - PLANE_THICKNESS;
					} else {
						rect_start.w = p_to_target.origin.w;
					}
				}
			} else if (Math::is_zero_approx(normal.w)) {
				rect_size.z = PLANE_THICKNESS;
				if (normal.z > 0.0f) {
					rect_start.z = p_to_target.origin.z - PLANE_THICKNESS;
				} else {
					rect_start.z = p_to_target.origin.z;
				}
			}
		} else if (Math::is_zero_approx(normal.z) && Math::is_zero_approx(normal.w)) {
			rect_size.y = PLANE_THICKNESS;
			if (normal.y > 0.0f) {
				rect_start.y = p_to_target.origin.y - PLANE_THICKNESS;
			} else {
				rect_start.y = p_to_target.origin.y;
			}
		}
	} else if (Math::is_zero_approx(normal.y) && Math::is_zero_approx(normal.z) && Math::is_zero_approx(normal.w)) {
		rect_size.x = PLANE_THICKNESS;
		if (normal.x > 0.0f) {
			rect_start.x = p_to_target.origin.x - PLANE_THICKNESS;
		} else {
			rect_start.x = p_to_target.origin.x;
		}
	}
	return Rect4(rect_start, rect_size);
}

Dictionary PlaneShape4D::raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction) const {
	Dictionary result;
	result["hit"] = false;
	ERR_FAIL_COND_V_MSG(!p_local_direction.is_normalized(), result, "PlaneShape4D::raycast_intersects: Ray direction must be normalized.");
	const Plane4D plane = get_plane_4d();
	const real_t factor = plane.intersect_ray_factor(p_local_from, p_local_direction);
	const bool hit = factor >= 0.0f;
	result["hit"] = hit;
	if (hit) {
		result["distance"] = factor;
		result["normal"] = p_local_from.y > 0.0f ? Vector4(0, 1, 0, 0) : Vector4(0, -1, 0, 0);
	}
	return result;
}

real_t PlaneShape4D::get_signed_distance_to_surface(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_surface) const {
	if (r_nearest_point_on_surface != nullptr) {
		*r_nearest_point_on_surface = Vector4(p_local_point.x, 0.0f, p_local_point.z, p_local_point.w);
	}
	return p_local_point.y;
}

Vector4 PlaneShape4D::get_nearest_point(const Vector4 &p_local_point) const {
	if (p_local_point.y <= 0.0f) {
		return p_local_point;
	}
	return Vector4(p_local_point.x, 0.0f, p_local_point.z, p_local_point.w);
}

Vector4 PlaneShape4D::get_support_point(const Vector4 &p_local_direction) const {
	Vector4 support_point = Vector4(0, 0, 0, 0);
	if (p_local_direction.x > CMP_EPSILON) {
		support_point.x = PLANE_WIDTH;
	} else if (p_local_direction.x < -CMP_EPSILON) {
		support_point.x = -PLANE_WIDTH;
	}
	if (p_local_direction.z > CMP_EPSILON) {
		support_point.z = PLANE_WIDTH;
	} else if (p_local_direction.z < -CMP_EPSILON) {
		support_point.z = -PLANE_WIDTH;
	}
	if (p_local_direction.w > CMP_EPSILON) {
		support_point.w = PLANE_WIDTH;
	} else if (p_local_direction.w < -CMP_EPSILON) {
		support_point.w = -PLANE_WIDTH;
	}
	if (p_local_direction.y < -CMP_EPSILON) {
		support_point.y = -PLANE_WIDTH;
	}
	return support_point;
}

bool PlaneShape4D::has_point(const Vector4 &p_local_point) const {
	return p_local_point.y <= 0.0f;
}

bool PlaneShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<PlaneShape4D> other = p_shape;
	// PlaneShape4D has no properties, so any valid PlaneShape4D is equal to another.
	return other.is_valid();
}

Ref<WireMesh4D> PlaneShape4D::to_wire_mesh(const Dictionary &p_options) const {
	const double size = p_options.has("size") ? (double)p_options["size"] : (double)5.0;
	const int64_t subdivision_segments = p_options.has("segments") ? (int64_t)p_options["segments"] : (int64_t)10;
	const Vector3i subdiv = Vector3i(subdivision_segments, subdivision_segments, subdivision_segments);
	const bool fill_cell = p_options.has("fill_cell") ? (bool)p_options["fill_cell"] : true;
	const bool breakup_edges = p_options.has("breakup_edges") ? (bool)p_options["breakup_edges"] : false;
	Ref<ArrayWireMesh4D> plane_wire_mesh = WireMeshBuilder4D::create_3d_subdivided_box(Vector3(size, size, size), subdiv, fill_cell, breakup_edges);
	Basis4D swap_yw_rot = Basis4D::from_swap_rotation(1, 3);
	plane_wire_mesh->transform_all_vertices(Transform4D(swap_yw_rot));
	// Make an arrow for the normal vector.
	plane_wire_mesh->append_edge_points(Vector4(0, 1, 0, 0), Vector4(0, 0, 0, 0));
	plane_wire_mesh->append_edge_points(Vector4(0, 1, 0, 0), Vector4(+0.25, 0.75, 0, 0));
	plane_wire_mesh->append_edge_points(Vector4(0, 1, 0, 0), Vector4(-0.25, 0.75, 0, 0));
	plane_wire_mesh->append_edge_points(Vector4(0, 1, 0, 0), Vector4(0, 0.75, +0.25, 0));
	plane_wire_mesh->append_edge_points(Vector4(0, 1, 0, 0), Vector4(0, 0.75, -0.25, 0));
	plane_wire_mesh->append_edge_points(Vector4(0, 1, 0, 0), Vector4(0, 0.75, 0, +0.25));
	plane_wire_mesh->append_edge_points(Vector4(0, 1, 0, 0), Vector4(0, 0.75, 0, -0.25));
	return plane_wire_mesh;
}
