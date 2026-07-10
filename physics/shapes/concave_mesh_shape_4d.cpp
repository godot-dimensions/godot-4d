#include "concave_mesh_shape_4d.h"

#include "../../math/geometry_4d.h"
#include "../../math/plane_4d.h"
#include "../../math/vector_4d.h"
#include "../../model/mesh/tetra/array_tetra_mesh_4d.h"
#include "../../model/mesh/wire/array_wire_mesh_4d.h"

PackedVector4Array ConcaveMeshShape4D::_calculate_normals(const PackedVector4Array &p_simplex_cells) {
	const int64_t normal_count = p_simplex_cells.size() / 4;
	PackedVector4Array new_normals;
	new_normals.resize(normal_count);
	for (int64_t i = 0; i < normal_count; i++) {
		const Vector4 pivot = p_simplex_cells[i * 4];
		const Vector4 a = p_simplex_cells[1 + i * 4];
		const Vector4 b = p_simplex_cells[2 + i * 4];
		const Vector4 c = p_simplex_cells[3 + i * 4];
		const Vector4 perp = Vector4D::perpendicular(a - pivot, b - pivot, c - pivot);
		new_normals.set(i, perp);
	}
	return new_normals;
}

PackedFloat64Array ConcaveMeshShape4D::_calculate_inverse_metric_cache(const PackedVector4Array &p_simplex_cells) {
	const int64_t simplex_tet_count = p_simplex_cells.size() / 4;
	PackedFloat64Array new_inverse_metric_cache;
	new_inverse_metric_cache.resize(simplex_tet_count * 6);
	for (int64_t simplex_tet_index = 0; simplex_tet_index < simplex_tet_count; simplex_tet_index++) {
		// These indices are guaranteed to be within bounds due to mesh validation.
		const Vector4 vert0 = p_simplex_cells[simplex_tet_index * 4 + 0];
		const Vector4 vert1 = p_simplex_cells[simplex_tet_index * 4 + 1];
		const Vector4 vert2 = p_simplex_cells[simplex_tet_index * 4 + 2];
		const Vector4 vert3 = p_simplex_cells[simplex_tet_index * 4 + 3];
		const Vector4 edge1 = vert1 - vert0;
		const Vector4 edge2 = vert2 - vert0;
		const Vector4 edge3 = vert3 - vert0;
		const real_t gram00 = edge1.dot(edge1);
		const real_t gram01 = edge1.dot(edge2);
		const real_t gram02 = edge1.dot(edge3);
		const real_t gram11 = edge2.dot(edge2);
		const real_t gram12 = edge2.dot(edge3);
		const real_t gram22 = edge3.dot(edge3);
		real_t inv_gram[6];
		const bool valid = Geometry4D::compute_inverse_metric_3x3(gram00, gram01, gram02, gram11, gram12, gram22, inv_gram);
		if (!valid) {
			new_inverse_metric_cache.clear();
			ERR_PRINT("ConcaveMeshShape4D: Closest-point cache build failed because tetrahedron " + itos(simplex_tet_index) + " is degenerate or non-finite.");
			return new_inverse_metric_cache;
		}
		for (int64_t gram_index = 0; gram_index < 6; gram_index++) {
			new_inverse_metric_cache.set(simplex_tet_index * 6 + gram_index, inv_gram[gram_index]);
		}
	}
	return new_inverse_metric_cache;
}

// Thread safety: Consider deferring or synchronizing calls to this function to avoid updating from underneath another thread.
// The contents utilize basic copy-on-write (CoW) semantics, but race conditions are still possible.
void ConcaveMeshShape4D::set_simplex_cells(const PackedVector4Array &p_simplex_cells) {
	ERR_FAIL_COND_MSG(p_simplex_cells.size() % 4 != 0, "ConcaveMeshShape4D: Simplexes must be a multiple of 4 (every 4 vertices form a simplex cell).");
	const PackedVector4Array new_normals = _calculate_normals(p_simplex_cells);
	const PackedFloat64Array new_inverse_metric_cache = _calculate_inverse_metric_cache(p_simplex_cells);
	const int64_t simplex_tet_count = p_simplex_cells.size() / 4;
	ERR_FAIL_COND_MSG(new_inverse_metric_cache.size() != simplex_tet_count * 6, "ConcaveMeshShape4D: Closest-point cache build failed because one or more tetrahedra are degenerate or non-finite.");
	// Thread safety: Update all of these at once, so that any other thread reading them will see all of them get updated at once.
	// Note that these sequential writes are not guaranteed to be atomic, but it's a very low chance. Consider modifying this if it becomes a problem.
	_simplex_cells = p_simplex_cells;
	_normals = new_normals;
	_inverse_metric_cache = new_inverse_metric_cache;
}

bool ConcaveMeshShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	const Ref<ConcaveMeshShape4D> other_shape = p_shape;
	if (other_shape.is_null()) {
		return false;
	}
	const PackedVector4Array other_cells = other_shape->get_simplex_cells();
	return _simplex_cells == other_cells;
}

Dictionary ConcaveMeshShape4D::raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance, const bool p_inside_is_zero) const {
	Dictionary result;
	result["hit"] = false;
	ERR_FAIL_COND_V_MSG(!p_local_direction.is_normalized(), result, "ConcaveMeshShape4D::raycast_intersects: Ray direction must be normalized.");
	// CoW, cheap layer of thread safety.
	const PackedFloat64Array inverse_metric_cache = _inverse_metric_cache;
	const PackedVector4Array simplex_cells = _simplex_cells;
	const PackedVector4Array normals = _normals;
	// Check if the data is valid before proceeding with the raycast.
	const int64_t simplex_tet_count = simplex_cells.size() / 4;
	if (simplex_tet_count == 0) {
		return result; // No tetrahedra to raycast against.
	}
	CRASH_COND(inverse_metric_cache.size() != simplex_tet_count * 6); // Guaranteed to be populated by `set_simplex_cells()`.
	const int64_t boundary_normals_count = normals.size();
	CRASH_COND(boundary_normals_count != simplex_tet_count); // Guaranteed to be populated by `set_simplex_cells()`.
	Vector4 best_hit_normal = Vector4();
	real_t best_distance = Math_INF;
	int32_t best_tet_cell_index = -1;
	// Iterate through all tetrahedra to find the closest ray intersection.
	for (int64_t tet_index = 0; tet_index < simplex_tet_count; tet_index++) {
		Plane4D tet_plane = Plane4D(normals[tet_index], simplex_cells[tet_index * 4 + 0]);
		real_t tet_plane_intersection_factor = tet_plane.intersect_ray_factor(p_local_from, p_local_direction);
		if (tet_plane_intersection_factor < 0.0f) {
			continue; // No intersection with the plane of this tetrahedron.
		}
		if (tet_plane_intersection_factor > best_distance) {
			continue; // Worse than the best intersection found so far.
		}
		// Check if this candidate intersection is inside the tetrahedron.
		const Vector4 intersection_point = p_local_from + p_local_direction * tet_plane_intersection_factor;
		const bool hit = Geometry4D::is_point_inside_tetrahedron_barycentric(simplex_cells[tet_index * 4], simplex_cells[tet_index * 4 + 1], simplex_cells[tet_index * 4 + 2], simplex_cells[tet_index * 4 + 3], intersection_point, inverse_metric_cache, tet_index);
		if (hit) {
			best_hit_normal = tet_plane.normal;
			best_distance = tet_plane_intersection_factor;
			best_tet_cell_index = tet_index;
		}
	}
	result["hit"] = best_tet_cell_index != -1;
	if (best_tet_cell_index != -1) {
		result["distance"] = best_distance;
		result["normal"] = best_hit_normal;
		result["cell_index"] = best_tet_cell_index;
	}
	return result;
}

Vector4 ConcaveMeshShape4D::get_nearest_point(const Vector4 &p_local_point) const {
	// CoW, cheap layer of thread safety.
	const PackedFloat64Array inverse_metric_cache = _inverse_metric_cache;
	const PackedVector4Array simplex_cells = _simplex_cells;
	const PackedVector4Array normals = _normals;
	// Check if the data is valid before proceeding with the raycast.
	const int64_t simplex_tet_count = simplex_cells.size() / 4;
	ERR_FAIL_COND_V_MSG(simplex_tet_count == 0, Vector4(), "ConcaveMeshShape4D: Cannot get nearest point on a mesh shape with zero tetrahedra.");
	CRASH_COND(inverse_metric_cache.size() != simplex_tet_count * 6); // Guaranteed to be populated by `set_simplex_cells()`.
	// Iterate over all tetrahedra to find the nearest point on the mesh, keeping track of the best one.
	constexpr int32_t MAX_CANDIDATE_TETS = 8;
	Vector4 best_candidate_points_on_tet[MAX_CANDIDATE_TETS];
	int32_t best_candidate_tets[MAX_CANDIDATE_TETS];
	int32_t best_candidate_tet_count = 0;
	Vector4 best_point_on_tet = Vector4();
	real_t best_distance_sq = Math_INF;
	bool best_proj_inside = false;
	// Future: This part could be accelerated with spatial partitioning, and/or accelerated with threading.
	// But those optimizations add a lot of complexity and would only benefit larger meshes.
	for (int64_t tet_index = 0; tet_index < simplex_tet_count; tet_index++) {
		Vector4 nearest_on_tet;
		real_t min_distance_sq = 0.0;
		bool proj_inside = false;
		const Vector4 vert0 = simplex_cells[tet_index * 4 + 0];
		const Vector4 vert1 = simplex_cells[tet_index * 4 + 1];
		const Vector4 vert2 = simplex_cells[tet_index * 4 + 2];
		const Vector4 vert3 = simplex_cells[tet_index * 4 + 3];
		Geometry4D::get_nearest_point_on_tetrahedron_barycentric(vert0, vert1, vert2, vert3, p_local_point, inverse_metric_cache, tet_index, nearest_on_tet, min_distance_sq, proj_inside);
		const bool less_dist = min_distance_sq < best_distance_sq;
		// If the projection is outside the tet, but the projected point is the same distance as what we
		// already found, then we may have multiple candidates for the closest tet to this point.
		// In this case, we need to collect them all for later disambiguation using the boundary normal.
		if (!proj_inside) {
			if (!best_proj_inside && Math::is_equal_approx(min_distance_sq, best_distance_sq)) {
				if (best_candidate_tet_count < MAX_CANDIDATE_TETS) {
					best_candidate_points_on_tet[best_candidate_tet_count] = nearest_on_tet;
					best_candidate_tets[best_candidate_tet_count] = tet_index;
					best_candidate_tet_count++;
				} else {
					WARN_PRINT("TetraMesh4D: Too many candidate tets for nearest point calculation. Results may be sub-optimal.");
				}
			} else if (less_dist) {
				best_candidate_points_on_tet[0] = nearest_on_tet;
				best_candidate_tets[0] = tet_index;
				best_candidate_tet_count = 1;
			}
		}
		// If the projection is closer than what we have already found, then this is the new best point.
		// Update the best point and distance regardless of the projection being inside or outside.
		if (less_dist) {
			best_point_on_tet = nearest_on_tet;
			best_distance_sq = min_distance_sq;
			best_proj_inside = proj_inside;
			if (proj_inside) {
				// If the projection is inside, then this is the single unambiguous nearest point so far.
				best_candidate_tet_count = 0;
			}
		}
	}
	if (best_candidate_tet_count > 1) {
		// We have multiple candidates with the same distance, so we need to disambiguate using
		// the absolute angle to the boundary normal (these are normalized, so use the dot product).
		real_t best_dot_abs = -1.0;
		for (int32_t candidate_num = 0; candidate_num < best_candidate_tet_count; candidate_num++) {
			const int32_t candidate_tet = best_candidate_tets[candidate_num];
			const Vector4 candidate_point_on_tet = best_candidate_points_on_tet[candidate_num];
			const Vector4 tet_point_dir_to_target = (p_local_point - candidate_point_on_tet).normalized();
			const Vector4 normal = normals[candidate_tet];
			const real_t dot_abs = Math::abs(tet_point_dir_to_target.dot(normal));
			if (dot_abs > best_dot_abs) {
				best_dot_abs = dot_abs;
				best_point_on_tet = candidate_point_on_tet;
			}
		}
	}
	return best_point_on_tet;
}

Vector4 ConcaveMeshShape4D::get_support_point(const Vector4 &p_local_direction) const {
	Vector4 farthest_point = Vector4();
	real_t farthest_distance = -Math_INF;
	for (int64_t i = 0; i < _simplex_cells.size(); i++) {
		const Vector4 &point = _simplex_cells[i];
		const real_t distance = point.dot(p_local_direction);
		if (distance > farthest_distance) {
			farthest_distance = distance;
			farthest_point = point;
		}
	}
	return farthest_point;
}

Ref<TetraMesh4D> ConcaveMeshShape4D::to_tetra_mesh(const Dictionary &p_options) const {
	Ref<ArrayTetraMesh4D> mesh;
	mesh.instantiate();
	mesh->append_vertices(_simplex_cells);
	return mesh;
}

Ref<WireMesh4D> ConcaveMeshShape4D::to_wire_mesh(const Dictionary &p_options) const {
	return to_tetra_mesh()->to_wire_mesh();
}

Ref<ConcaveMeshShape4D> ConcaveMeshShape4D::create_from_mesh(const Ref<TetraMesh4D> &p_mesh) {
	Ref<ConcaveMeshShape4D> shape;
	shape.instantiate();
	shape->set_simplex_cells(p_mesh->get_simplex_cell_positions());
	return shape;
}

void ConcaveMeshShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_simplex_cells"), &ConcaveMeshShape4D::get_simplex_cells);
	ClassDB::bind_method(D_METHOD("set_simplex_cells", "simplexes"), &ConcaveMeshShape4D::set_simplex_cells);

	ClassDB::bind_static_method("ConcaveMeshShape4D", D_METHOD("create_from_mesh", "mesh"), &ConcaveMeshShape4D::create_from_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "simplex_cells", PROPERTY_HINT_NONE, "suffix:m"), "set_simplex_cells", "get_simplex_cells");
}
