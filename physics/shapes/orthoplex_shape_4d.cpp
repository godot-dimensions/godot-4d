#include "orthoplex_shape_4d.h"

#include "../../model/mesh/poly/orthoplex_poly_mesh_4d.h"
#include "../../model/mesh/tetra/orthoplex_tetra_mesh_4d.h"
#include "../../model/mesh/wire/orthoplex_wire_mesh_4d.h"

#include <algorithm>

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
	// The orthoplex is the intersection of 16 half-spaces, one per facet, where dot(signs / half_extents, point) <= 1.
	// All facet normals have the same length before normalizing, so all facets are the same distance from the center.
	// Clip the ray against each half-space: the ray enters the shape at the last facet it crosses going inwards,
	// and exits at the first facet it crosses going outwards. This avoids needing a tolerance for the hit point.
	const Vector4 inverse_half_extents = get_half_extents().inverse();
	const real_t facet_distance = 1.0f / inverse_half_extents.length();
	real_t enter_distance = -Math_INF;
	real_t exit_distance = Math_INF;
	Vector4 enter_normal = Vector4();
	Vector4 exit_normal = Vector4();
	// Iterate over the 16 planes of the orthoplex.
	for (real_t x = -1.0f; x <= 1.0f; x += 2.0f) {
		for (real_t y = -1.0f; y <= 1.0f; y += 2.0f) {
			for (real_t z = -1.0f; z <= 1.0f; z += 2.0f) {
				for (real_t w = -1.0f; w <= 1.0f; w += 2.0f) {
					const Vector4 facet_normal = Vector4(x, y, z, w) * inverse_half_extents * facet_distance;
					// Positive if the ray starts on the inner side of the facet's plane.
					const real_t from_inside_distance = facet_distance - facet_normal.dot(p_local_from);
					const real_t direction_dot = facet_normal.dot(p_local_direction);
					if (direction_dot == 0.0f) {
						if (from_inside_distance < 0.0f) {
							// The ray is parallel to this facet and outside of it, so it can't hit the shape.
							return result;
						}
						continue;
					}
					const real_t distance = from_inside_distance / direction_dot;
					if (direction_dot < 0.0f) {
						if (distance > enter_distance) {
							enter_distance = distance;
							enter_normal = facet_normal;
						}
					} else if (distance < exit_distance) {
						exit_distance = distance;
						exit_normal = facet_normal;
					}
				}
			}
		}
	}
	if (enter_distance > exit_distance || exit_distance < 0.0f) {
		return result;
	}
	// If the ray starts inside the shape, it hits the surface where it exits.
	const bool starts_inside = enter_distance < 0.0f;
	const real_t hit_distance = starts_inside ? exit_distance : enter_distance;
	const bool hit = hit_distance < p_max_distance;
	result["hit"] = hit;
	if (hit) {
		result["point"] = p_local_from + p_local_direction * hit_distance;
		result["distance"] = hit_distance;
		result["normal"] = starts_inside ? exit_normal : enter_normal;
	}
	return result;
}

real_t OrthoplexShape4D::get_signed_distance_to_surface(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_surface) const {
	const Vector4 half_extents = get_half_extents();
	const Vector4 abs_scaled_point = p_local_point.abs() / half_extents;
	const real_t scaled_taxicab_length = abs_scaled_point.x + abs_scaled_point.y + abs_scaled_point.z + abs_scaled_point.w;
	if (scaled_taxicab_length > 1.0f) {
		// Outside the shape, the nearest point on the surface is the nearest point in the shape.
		const Vector4 nearest_point = get_nearest_point(p_local_point);
		if (r_nearest_point_on_surface != nullptr) {
			*r_nearest_point_on_surface = nearest_point;
		}
		return p_local_point.distance_to(nearest_point);
	}
	// Inside the shape, the nearest facet is the one in the same orthant as the point. All facets are the same
	// distance from the center, so the signed distance to it is proportional to the scaled taxicab length.
	const Vector4 inverse_half_extents = half_extents.inverse();
	const real_t facet_normal_length = inverse_half_extents.length();
	const real_t signed_distance = (scaled_taxicab_length - 1.0f) / facet_normal_length;
	if (r_nearest_point_on_surface != nullptr) {
		// Moving along the facet normal keeps the point in the same orthant, so it lands on that facet.
		// For axes where the point is zero, the facets on both sides are equally near, so pick the positive one.
		const Vector4 facet_signs = Vector4(
				p_local_point.x < 0.0f ? -1.0f : 1.0f,
				p_local_point.y < 0.0f ? -1.0f : 1.0f,
				p_local_point.z < 0.0f ? -1.0f : 1.0f,
				p_local_point.w < 0.0f ? -1.0f : 1.0f);
		const Vector4 facet_normal = facet_signs * inverse_half_extents / facet_normal_length;
		*r_nearest_point_on_surface = p_local_point - facet_normal * signed_distance;
	}
	return signed_distance;
}

Vector4 OrthoplexShape4D::get_nearest_point(const Vector4 &p_local_point) const {
	const Vector4 half_extents = get_half_extents();
	const Vector4 abs_point = p_local_point.abs();
	const Vector4 abs_scaled_point = abs_point / half_extents;
	const real_t scaled_taxicab_length = abs_scaled_point.x + abs_scaled_point.y + abs_scaled_point.z + abs_scaled_point.w;
	if (scaled_taxicab_length <= 1.0f) {
		return p_local_point;
	}
	// Scaling into a unit space and limiting the taxicab length there only works for uniform sizes, because non-uniform
	// scaling does not preserve Euclidean distance. Instead, project onto the facets directly: each axis moves towards
	// zero by lambda / half_extents[axis] (clamped at zero), with lambda chosen so that the result is on the surface.
	// An axis reaches zero once lambda >= abs_point[axis] * half_extents[axis], so sort the axes by that value,
	// and find how many of the axes are still non-zero, which determines lambda.
	const Vector4 zero_thresholds = abs_point * half_extents;
	Vector4::Axis axes[4] = { Vector4::Axis::AXIS_X, Vector4::Axis::AXIS_Y, Vector4::Axis::AXIS_Z, Vector4::Axis::AXIS_W };
	std::sort(axes, axes + 4, [&zero_thresholds](Vector4::Axis a, Vector4::Axis b) {
		return zero_thresholds[a] > zero_thresholds[b];
	});
	real_t lambda = 0.0f;
	real_t nonzero_scaled_sum = 0.0f;
	real_t nonzero_inverse_square_sum = 0.0f;
	for (int i = 0; i < 4; i++) {
		const Vector4::Axis axis = axes[i];
		nonzero_scaled_sum += abs_scaled_point[axis];
		nonzero_inverse_square_sum += 1.0f / (half_extents[axis] * half_extents[axis]);
		const real_t candidate_lambda = (nonzero_scaled_sum - 1.0f) / nonzero_inverse_square_sum;
		// The axis with the largest threshold is always non-zero, so always accept the first candidate.
		if (i > 0 && zero_thresholds[axis] <= candidate_lambda) {
			break;
		}
		lambda = candidate_lambda;
	}
	Vector4 nearest_point = Vector4();
	for (int i = 0; i < 4; i++) {
		const real_t abs_nearest = MAX(abs_point[i] - lambda / half_extents[i], (real_t)0.0);
		nearest_point[i] = (p_local_point[i] < 0.0f) ? -abs_nearest : abs_nearest;
	}
	return nearest_point;
}

Vector4 OrthoplexShape4D::get_support_point(const Vector4 &p_local_direction) const {
	const Vector4 half_extents = get_half_extents();
	// The support point is the vertex that is furthest along the direction, which accounts for each axis's size.
	const Vector4::Axis support_axis = (p_local_direction.abs() * half_extents).max_axis_index();
	Vector4 support = Vector4();
	support[support_axis] = (p_local_direction[support_axis] > 0.0f) ? half_extents[support_axis] : -half_extents[support_axis];
	return support;
}

bool OrthoplexShape4D::has_point(const Vector4 &p_local_point) const {
	const Vector4 abs_scaled_point = p_local_point.abs() / get_half_extents();
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
