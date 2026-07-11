#include "orthoplex_shape_4d.h"

#include "../../math/plane_4d.h"
#include "../../math/vector_4d.h"
#include "../../model/mesh/poly/orthoplex_poly_mesh_4d.h"
#include "../../model/mesh/tetra/orthoplex_tetra_mesh_4d.h"
#include "../../model/mesh/wire/orthoplex_wire_mesh_4d.h"

Vector4 OrthoplexShape4D::get_half_extents() const {
	return _size * 0.5f;
}

void OrthoplexShape4D::set_half_extents(const Vector4 &p_half_extents) {
	_size = p_half_extents * 2.0f;
}

Vector4 OrthoplexShape4D::get_size() const {
	return _size;
}

void OrthoplexShape4D::set_size(const Vector4 &p_size) {
	_size = p_size;
}

real_t OrthoplexShape4D::get_hypervolume() const {
	// http://hi.gher.space/wiki/16-cell
	// Bulk of regular orthoplex is (1/6) * edge_length ^ 4, but we have size instead of edge length.
	// To convert we need to divide by sqrt(2) 4 times, so 1/4, so (1/6)*(1/4) becomes 1/24.
	return (1.0 / 24.0) * _size.x * _size.y * _size.z * _size.w;
}

real_t OrthoplexShape4D::get_surface_volume() const {
	// Surcell volume of regular orthoplex is (4*sqrt(2)/3) * edge_length ^ 3, but we have size instead of edge length.
	// To convert we need to divide by sqrt(2) 3 times, so 1/(2*sqrt(2)), the sqrt(2)s cancel out, so (4/3)*(1/2) = 2/3.
	// Then since we have each axis separate, we need to multiply by a quarter 4 times, so (2/3)*(1/4) = 1/6.
	return (1.0 / 6.0) * (_size.x * _size.y * _size.z + _size.x * _size.y * _size.w + _size.x * _size.z * _size.w + _size.y * _size.z * _size.w);
}

Rect4 OrthoplexShape4D::get_rect_bounds(const Transform4D &p_to_target) const {
	Rect4 bounds = Rect4(p_to_target.origin, Vector4());
	const Vector4 half_extents = get_half_extents();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 2; j++) {
			Vector4 point = Vector4();
			point[i] = j == 0 ? -half_extents[i] : half_extents[i];
			bounds = bounds.expand_to_point(p_to_target * point);
		}
	}
	return bounds;
}

Dictionary OrthoplexShape4D::raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance, const bool p_inside_is_zero) const {
	Dictionary result;
	result["hit"] = false;
	ERR_FAIL_COND_V_MSG(!p_local_direction.is_normalized(), result, "OrthoplexShape4D::raycast_intersects: Ray direction must be normalized.");
	if (p_inside_is_zero && has_point(p_local_from)) {
		result["hit"] = true;
		result["distance"] = 0.0f;
		result["normal"] = Vector4(0.0, 0.0, 0.0, 0.0);
		result["point"] = p_local_from;
		return result;
	}
	// Convert the raycast vectors into a space that assumes the orthoplex is of unit size.
	const Vector4 ray_from = p_local_from / _size;
	const Vector4 ray_direction = (p_local_direction / _size).normalized();
	Vector4 best_normal = Vector4();
	real_t best_distance = p_max_distance;
	// Iterate over the 16 planes of the orthoplex.
	for (real_t x = -0.5f; x <= 0.5f; x += 1.0f) {
		for (real_t y = -0.5f; y <= 0.5f; y += 1.0f) {
			for (real_t z = -0.5f; z <= 0.5f; z += 1.0f) {
				for (real_t w = -0.5f; w <= 0.5f; w += 1.0f) {
					const Plane4D plane = Plane4D(Vector4(x, y, z, w), 0.5f);
					const real_t factor = plane.intersect_ray_factor(ray_from, ray_direction);
					if (factor >= 0.0f && factor < best_distance) {
						const Vector4 hit_point_abs = (ray_from + ray_direction * factor).abs();
						// Similar to has_point but does not scale by the size and has a slightly larger tolerance.
						if ((hit_point_abs.x + hit_point_abs.y + hit_point_abs.z + hit_point_abs.w) <= 1.000001f) {
							best_distance = factor;
							best_normal = plane.normal;
						}
					}
				}
			}
		}
	}
	const bool hit = best_distance < p_max_distance;
	result["hit"] = hit;
	if (hit) {
		const Vector4 hit_point_accounting_for_size = (ray_from + ray_direction * best_distance) * _size;
		result["point"] = hit_point_accounting_for_size;
		result["distance"] = p_local_from.distance_to(hit_point_accounting_for_size);
		// Divide by the size again, aka multiply by the inverse of the size.
		// Ex: Larger size on X means the "faces" point more in the YZW directions, so the normal is smaller in X.
		result["normal"] = (best_normal * _size.inverse()).normalized();
	}
	return result;
}

real_t OrthoplexShape4D::get_signed_distance_to_surface(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_surface) const {
	if (p_local_point == Vector4(0.0f, 0.0f, 0.0f, 0.0f)) {
		if (r_nearest_point_on_surface != nullptr) {
			*r_nearest_point_on_surface = Vector4(0.5f * _size.x, 0.0f, 0.0f, 0.0f);
		}
		return -0.5f * _size.x;
	}
	const Vector4 abs_scaled_point = p_local_point.abs() / _size;
	const real_t scaled_signed_distance = (abs_scaled_point.x + abs_scaled_point.y + abs_scaled_point.z + abs_scaled_point.w) - 1.0f;
	const real_t adjust_per_axis = scaled_signed_distance * -0.25f;
	Vector4 nearest_point = abs_scaled_point + Vector4(adjust_per_axis, adjust_per_axis, adjust_per_axis, adjust_per_axis);
	nearest_point *= (p_local_point.sign() * _size);
	if (r_nearest_point_on_surface != nullptr) {
		*r_nearest_point_on_surface = nearest_point;
	}
	const real_t distance = p_local_point.distance_to(nearest_point);
	return (scaled_signed_distance < 0.0f) ? -distance : distance;
}

Vector4 OrthoplexShape4D::get_nearest_point(const Vector4 &p_local_point) const {
	return Vector4D::limit_length_taxicab(p_local_point / _size, 0.5) * _size;
}

Vector4 OrthoplexShape4D::get_support_point(const Vector4 &p_local_direction) const {
	const Vector4 abs_dir = p_local_direction.abs();
	const Vector4::Axis longest_axis = abs_dir.max_axis_index();
	Vector4 support = Vector4();
	support[longest_axis] = (p_local_direction[longest_axis] > 0.0f) ? _size[longest_axis] * 0.5f : -_size[longest_axis] * 0.5f;
	return support;
}

bool OrthoplexShape4D::has_point(const Vector4 &p_local_point) const {
	const Vector4 abs_scaled_point = p_local_point.abs() / _size;
	return (abs_scaled_point.x + abs_scaled_point.y + abs_scaled_point.z + abs_scaled_point.w) <= 1.0f;
}

bool OrthoplexShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<OrthoplexShape4D> other_shape = p_shape;
	if (other_shape.is_null()) {
		return false;
	}
	return _size == other_shape->get_size();
}

Ref<PolyMesh4D> OrthoplexShape4D::to_poly_mesh(const Dictionary &p_options) const {
	Ref<OrthoplexPolyMesh4D> poly_mesh;
	poly_mesh.instantiate();
	poly_mesh->set_size(_size);
	return poly_mesh;
}

Ref<TetraMesh4D> OrthoplexShape4D::to_tetra_mesh(const Dictionary &p_options) const {
	Ref<OrthoplexTetraMesh4D> tetra_mesh;
	tetra_mesh.instantiate();
	tetra_mesh->set_size(_size);
	return tetra_mesh;
}

Ref<WireMesh4D> OrthoplexShape4D::to_wire_mesh(const Dictionary &p_options) const {
	Ref<OrthoplexWireMesh4D> wire_mesh;
	wire_mesh.instantiate();
	wire_mesh->set_size(_size);
	return wire_mesh;
}

void OrthoplexShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_half_extents"), &OrthoplexShape4D::get_half_extents);
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &OrthoplexShape4D::set_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_half_extents", "get_half_extents");

	ClassDB::bind_method(D_METHOD("get_size"), &OrthoplexShape4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &OrthoplexShape4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size", PROPERTY_HINT_NONE, "suffix:m"), "set_size", "get_size");
}
