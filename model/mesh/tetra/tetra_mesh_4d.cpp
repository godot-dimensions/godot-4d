#include "tetra_mesh_4d.h"

#include "../../../math/geometry_4d.h"
#include "../../../math/plane_4d.h"
#include "../../../math/vector_4d.h"
#include "../material_4d.h"
#include "array_tetra_mesh_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#elif GODOT_MODULE
#include "scene/resources/material.h"
#include "scene/resources/surface_tool.h"
#endif

// Nearest point and signed distance.

void TetraMesh4D::populate_inverse_metric_cache() {
	ERR_FAIL_COND_MSG(!is_mesh_data_valid(), "TetraMesh4D: Cannot populate closest-point cache for an invalid mesh.");
	const PackedInt32Array &simplex_cell_indices = get_simplex_cell_indices();
	const int64_t simplex_tet_count = simplex_cell_indices.size() / 4;
	if (_nearest_tetra_inverse_metric_cache.size() == simplex_tet_count * 6) {
		return;
	}
	_nearest_tetra_inverse_metric_cache.resize(simplex_tet_count * 6);
	const PackedVector4Array &vertices = get_vertices();
	for (int64_t simplex_tet_index = 0; simplex_tet_index < simplex_tet_count; simplex_tet_index++) {
		// These indices are guaranteed to be within bounds due to mesh validation.
		const int32_t i0 = simplex_cell_indices[simplex_tet_index * 4 + 0];
		const int32_t i1 = simplex_cell_indices[simplex_tet_index * 4 + 1];
		const int32_t i2 = simplex_cell_indices[simplex_tet_index * 4 + 2];
		const int32_t i3 = simplex_cell_indices[simplex_tet_index * 4 + 3];
		const Vector4 vert0 = vertices[i0];
		const Vector4 edge1 = vertices[i1] - vert0;
		const Vector4 edge2 = vertices[i2] - vert0;
		const Vector4 edge3 = vertices[i3] - vert0;
		const real_t gram00 = edge1.dot(edge1);
		const real_t gram01 = edge1.dot(edge2);
		const real_t gram02 = edge1.dot(edge3);
		const real_t gram11 = edge2.dot(edge2);
		const real_t gram12 = edge2.dot(edge3);
		const real_t gram22 = edge3.dot(edge3);
		real_t inv_gram[6];
		const bool valid = Geometry4D::compute_inverse_metric_3x3(gram00, gram01, gram02, gram11, gram12, gram22, inv_gram);
		if (!valid) {
			_nearest_tetra_inverse_metric_cache.clear();
			ERR_PRINT("TetraMesh4D: Closest-point cache build failed because tetrahedron " + itos(simplex_tet_index) + " is degenerate or non-finite.");
			return;
		}
		for (int64_t gram_index = 0; gram_index < 6; gram_index++) {
			_nearest_tetra_inverse_metric_cache.set(simplex_tet_index * 6 + gram_index, inv_gram[gram_index]);
		}
	}
}

real_t TetraMesh4D::get_signed_distance_to_mesh(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_tet, int *r_tetrahedron_index) {
	ERR_FAIL_COND_V_MSG(!is_mesh_data_valid(), Math_INF, "TetraMesh4D: Cannot get signed distance to an invalid mesh.");
	const PackedInt32Array &simplex_cell_indices = get_simplex_cell_indices();
	const int64_t simplex_tet_count = simplex_cell_indices.size() / 4;
	if (_nearest_tetra_inverse_metric_cache.size() != simplex_tet_count * 6) {
		populate_inverse_metric_cache();
	}
	ERR_FAIL_COND_V_MSG(_nearest_tetra_inverse_metric_cache.size() != simplex_tet_count * 6, Math_INF, "TetraMesh4D: Closest-point cache is invalid for this mesh.");
	ERR_FAIL_COND_V_MSG(simplex_tet_count == 0, Math_INF, "TetraMesh4D: Cannot get signed distance to a mesh with zero tetrahedra.");
	const PackedVector4Array &vertices = get_vertices();
	// Iterate over all tetrahedra to find the nearest point on the mesh, keeping track of the best one.
	constexpr int32_t MAX_CANDIDATE_TETS = 8;
	Vector4 best_candidate_points_on_tet[MAX_CANDIDATE_TETS];
	int32_t best_candidate_tets[MAX_CANDIDATE_TETS];
	int32_t best_candidate_tet_count = 0;
	Vector4 best_point_on_tet = Vector4();
	real_t best_distance_sq = Math_INF;
	int best_tet_index = -1;
	bool best_proj_inside = false;
	// Future: This part could be accelerated with spatial partitioning, and/or accelerated with threading.
	// But those optimizations add a lot of complexity and would only benefit larger meshes.
	for (int64_t tet_index = 0; tet_index < simplex_tet_count; tet_index++) {
		Vector4 nearest_on_tet;
		real_t min_distance_sq = 0.0;
		bool proj_inside = false;
		const Vector4 vert0 = vertices[simplex_cell_indices[tet_index * 4 + 0]];
		const Vector4 vert1 = vertices[simplex_cell_indices[tet_index * 4 + 1]];
		const Vector4 vert2 = vertices[simplex_cell_indices[tet_index * 4 + 2]];
		const Vector4 vert3 = vertices[simplex_cell_indices[tet_index * 4 + 3]];
		Geometry4D::get_nearest_point_on_tetrahedron_barycentric(vert0, vert1, vert2, vert3, p_local_point, _nearest_tetra_inverse_metric_cache, tet_index, nearest_on_tet, min_distance_sq, proj_inside);
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
			best_tet_index = tet_index;
			best_proj_inside = proj_inside;
			if (proj_inside) {
				// If the projection is inside, then this is the single unambiguous nearest point so far.
				best_candidate_tet_count = 0;
			}
		}
	}
	const PackedVector4Array &boundary_normals = get_simplex_cell_boundary_normals();
	if (best_candidate_tet_count > 1) {
		// We have multiple candidates with the same distance, so we need to disambiguate using
		// the absolute angle to the boundary normal (these are normalized, so use the dot product).
		real_t best_dot_abs = -1.0;
		for (int32_t candidate_num = 0; candidate_num < best_candidate_tet_count; candidate_num++) {
			const int32_t candidate_tet = best_candidate_tets[candidate_num];
			const Vector4 candidate_point_on_tet = best_candidate_points_on_tet[candidate_num];
			const Vector4 tet_point_dir_to_target = (p_local_point - candidate_point_on_tet).normalized();
			const Vector4 normal = boundary_normals[candidate_tet];
			const real_t dot_abs = Math::abs(tet_point_dir_to_target.dot(normal));
			if (dot_abs > best_dot_abs) {
				best_dot_abs = dot_abs;
				best_tet_index = candidate_tet;
				best_point_on_tet = candidate_point_on_tet;
			}
		}
	}
	// Write the outputs depending on what the caller requested.
	if (r_nearest_point_on_tet) {
		*r_nearest_point_on_tet = best_point_on_tet;
	}
	if (r_tetrahedron_index) {
		*r_tetrahedron_index = best_tet_index;
	}
	if (unlikely(best_tet_index < 0)) {
		// This should be impossible because we check for zero tetrahedra above, but just in case.
		return Math_INF;
	}
	// If we found a nearest point with a nearest tet, check its boundary normal to determine the sign of the distance.
	const Vector4 best_normal = boundary_normals[best_tet_index];
	const real_t side = (p_local_point - best_point_on_tet).dot(best_normal);
	real_t signed_distance = Math::sqrt(best_distance_sq);
	if (side < (real_t)0.0) {
		signed_distance = -signed_distance;
	}
	return signed_distance;
}

real_t TetraMesh4D::get_signed_distance_to_mesh_bind(const Vector4 &p_local_point) {
	return get_signed_distance_to_mesh(p_local_point, nullptr, nullptr);
}

bool TetraMesh4D::raycast_intersects_fast(const Vector4 &p_local_from, const Vector4 &p_local_direction) {
	ERR_FAIL_COND_V_MSG(!is_mesh_data_valid(), false, "TetraMesh4D: Cannot raycast on an invalid mesh.");
	const PackedInt32Array &simplex_cell_indices = get_simplex_cell_indices();
	const int64_t simplex_tet_count = simplex_cell_indices.size() / 4;
	if (simplex_tet_count == 0) {
		return false; // No tetrahedra to raycast against.
	}
	const PackedFloat64Array &inverse_metric_cache = _nearest_tetra_inverse_metric_cache;
	ERR_FAIL_COND_V_MSG(inverse_metric_cache.size() != simplex_tet_count * 6, false, "TetraMesh4D: Closest-point cache is invalid for this mesh. Call `populate_inverse_metric_cache()` before calling `raycast_intersects_fast()`.");
	const PackedVector4Array &vertices = get_vertices();
	const PackedVector4Array &boundary_normals = get_simplex_cell_boundary_normals();
	const int64_t boundary_normals_count = boundary_normals.size();
	// Iterate through all tetrahedra to find any ray intersection.
	for (int64_t tet_index = 0; tet_index < simplex_tet_count; tet_index++) {
		Plane4D tet_plane;
		if (simplex_tet_count == boundary_normals_count) {
			tet_plane = Plane4D(boundary_normals[tet_index], vertices[simplex_cell_indices[tet_index * 4 + 0]]);
		} else {
			// Fallback in case the boundary normals are not available or are mismatched.
			const Vector4 &pivot = vertices[simplex_cell_indices[tet_index * 4]];
			const Vector4 &a = vertices[simplex_cell_indices[1 + tet_index * 4]];
			const Vector4 &b = vertices[simplex_cell_indices[2 + tet_index * 4]];
			const Vector4 &c = vertices[simplex_cell_indices[3 + tet_index * 4]];
			const Vector4 perp = Vector4D::perpendicular(a - pivot, b - pivot, c - pivot);
			tet_plane = Plane4D(perp.normalized(), pivot);
		}
		real_t tet_plane_intersection_factor = tet_plane.intersect_ray_factor(p_local_from, p_local_direction);
		if (tet_plane_intersection_factor < 0.0f) {
			continue; // No intersection with the plane of this tetrahedron.
		}
		// Check if this candidate intersection is inside the tetrahedron.
		const Vector4 intersection_point = p_local_from + p_local_direction * tet_plane_intersection_factor;
		// These indices are guaranteed to be within bounds due to mesh validation.
		const int32_t i0 = simplex_cell_indices[tet_index * 4 + 0];
		const int32_t i1 = simplex_cell_indices[tet_index * 4 + 1];
		const int32_t i2 = simplex_cell_indices[tet_index * 4 + 2];
		const int32_t i3 = simplex_cell_indices[tet_index * 4 + 3];
		const Vector4 vert0 = vertices[i0];
		const Vector4 vert1 = vertices[i1];
		const Vector4 vert2 = vertices[i2];
		const Vector4 vert3 = vertices[i3];
		const bool hit = Geometry4D::is_point_inside_tetrahedron_barycentric(vert0, vert1, vert2, vert3, intersection_point, inverse_metric_cache, tet_index);
		if (hit) {
			// For this fast version, we only care if there is any intersection, so we can return true immediately.
			return true;
		}
	}
	return false;
}

Dictionary TetraMesh4D::raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction) {
	Dictionary result;
	result["hit"] = false;
	ERR_FAIL_COND_V_MSG(!is_mesh_data_valid(), result, "TetraMesh4D: Cannot raycast on an invalid mesh.");
	const PackedInt32Array &simplex_cell_indices = get_simplex_cell_indices();
	const int64_t simplex_tet_count = simplex_cell_indices.size() / 4;
	if (simplex_tet_count == 0) {
		return result; // No tetrahedra to raycast against.
	}
	const PackedFloat64Array &inverse_metric_cache = _nearest_tetra_inverse_metric_cache;
	ERR_FAIL_COND_V_MSG(inverse_metric_cache.size() != simplex_tet_count * 6, result, "TetraMesh4D: Closest-point cache is invalid for this mesh. Call `populate_inverse_metric_cache()` before calling `raycast_intersects()`.");
	const PackedVector4Array &vertices = get_vertices();
	const PackedVector4Array &boundary_normals = get_simplex_cell_boundary_normals();
	const int64_t boundary_normals_count = boundary_normals.size();
	Vector4 best_hit_normal = Vector4();
	real_t best_distance = Math_INF;
	int32_t best_tet_cell_index = -1;
	// Iterate through all tetrahedra to find the closest ray intersection.
	for (int64_t tet_index = 0; tet_index < simplex_tet_count; tet_index++) {
		Plane4D tet_plane;
		if (simplex_tet_count == boundary_normals_count) {
			tet_plane = Plane4D(boundary_normals[tet_index], vertices[simplex_cell_indices[tet_index * 4 + 0]]);
		} else {
			// Fallback in case the boundary normals are not available or are mismatched.
			const Vector4 &pivot = vertices[simplex_cell_indices[tet_index * 4]];
			const Vector4 &a = vertices[simplex_cell_indices[1 + tet_index * 4]];
			const Vector4 &b = vertices[simplex_cell_indices[2 + tet_index * 4]];
			const Vector4 &c = vertices[simplex_cell_indices[3 + tet_index * 4]];
			const Vector4 perp = Vector4D::perpendicular(a - pivot, b - pivot, c - pivot);
			tet_plane = Plane4D(perp.normalized(), pivot);
		}
		real_t tet_plane_intersection_factor = tet_plane.intersect_ray_factor(p_local_from, p_local_direction);
		if (tet_plane_intersection_factor < 0.0f) {
			continue; // No intersection with the plane of this tetrahedron.
		}
		if (tet_plane_intersection_factor > best_distance) {
			continue; // Worse than the best intersection found so far.
		}
		// Check if this candidate intersection is inside the tetrahedron.
		const Vector4 intersection_point = p_local_from + p_local_direction * tet_plane_intersection_factor;
		// These indices are guaranteed to be within bounds due to mesh validation.
		const int32_t i0 = simplex_cell_indices[tet_index * 4 + 0];
		const int32_t i1 = simplex_cell_indices[tet_index * 4 + 1];
		const int32_t i2 = simplex_cell_indices[tet_index * 4 + 2];
		const int32_t i3 = simplex_cell_indices[tet_index * 4 + 3];
		const Vector4 vert0 = vertices[i0];
		const Vector4 vert1 = vertices[i1];
		const Vector4 vert2 = vertices[i2];
		const Vector4 vert3 = vertices[i3];
		const bool hit = Geometry4D::is_point_inside_tetrahedron_barycentric(vert0, vert1, vert2, vert3, intersection_point, inverse_metric_cache, tet_index);
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

void TetraMesh4D::tetra_mesh_clear_cache() {
	_simplex_positions_cache.clear();
	_edge_positions_cache.clear();
	_edge_indices_cache.clear();
	_nearest_tetra_inverse_metric_cache.clear();
	mark_cross_section_mesh_dirty();
}

Ref<ArrayMesh> TetraMesh4D::convert_texture_map_to_mesh(const PackedVector3Array &p_texture_map) {
	Ref<SurfaceTool> surface_tool;
	surface_tool.instantiate();
	surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
	surface_tool->set_smooth_group(-1);
	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	surface_tool->set_material(material);
	const int64_t size = p_texture_map.size();
	float hue = 0.0f;
	for (int64_t i = 0; i < size - 3; i += 4) {
		surface_tool->set_color(Color::from_hsv(hue, 1.0f, 1.0f));
		// Fix for meshes having inverted faces.
		const Vector3 average = (p_texture_map[i] + p_texture_map[i + 1] + p_texture_map[i + 2] + p_texture_map[i + 3]) / 4.0f;
		const Vector3 first_cross = (p_texture_map[i + 1] - p_texture_map[i]).cross(p_texture_map[i + 2] - p_texture_map[i]);
		const real_t dot = first_cross.dot(average - p_texture_map[i]);
		if (dot == 0.0f) {
			continue; // Degenerate tetrahedron.
		} else if (dot > 0.0f) {
			surface_tool->add_vertex(p_texture_map[i]);
			surface_tool->add_vertex(p_texture_map[i + 1]);
			surface_tool->add_vertex(p_texture_map[i + 2]);
			surface_tool->add_vertex(p_texture_map[i]);
			surface_tool->add_vertex(p_texture_map[i + 2]);
			surface_tool->add_vertex(p_texture_map[i + 3]);
			surface_tool->add_vertex(p_texture_map[i]);
			surface_tool->add_vertex(p_texture_map[i + 3]);
			surface_tool->add_vertex(p_texture_map[i + 1]);
			surface_tool->add_vertex(p_texture_map[i + 1]);
			surface_tool->add_vertex(p_texture_map[i + 3]);
			surface_tool->add_vertex(p_texture_map[i + 2]);
		} else { // dot < 0.0f
			surface_tool->add_vertex(p_texture_map[i + 2]);
			surface_tool->add_vertex(p_texture_map[i + 1]);
			surface_tool->add_vertex(p_texture_map[i]);
			surface_tool->add_vertex(p_texture_map[i + 3]);
			surface_tool->add_vertex(p_texture_map[i + 2]);
			surface_tool->add_vertex(p_texture_map[i]);
			surface_tool->add_vertex(p_texture_map[i + 1]);
			surface_tool->add_vertex(p_texture_map[i + 3]);
			surface_tool->add_vertex(p_texture_map[i]);
			surface_tool->add_vertex(p_texture_map[i + 2]);
			surface_tool->add_vertex(p_texture_map[i + 3]);
			surface_tool->add_vertex(p_texture_map[i + 1]);
		}
		// Any irrational number will do here.
		hue += 0.045f * Math_E;
	}
	surface_tool->generate_normals();
	return surface_tool->commit();
}

Ref<ArrayMesh> TetraMesh4D::export_texture_map_mesh() {
	PackedVector3Array texture_map = get_simplex_cell_texture_map();
	// Remove any degenerate entries created by cells without UVW maps.
	for (int64_t i = texture_map.size() - 4; i >= 0; i -= 4) {
		if (unlikely(texture_map[i] == texture_map[i + 1])) {
			texture_map.remove_at(i + 3);
			texture_map.remove_at(i + 2);
			texture_map.remove_at(i + 1);
			texture_map.remove_at(i);
		}
	}
	return convert_texture_map_to_mesh(texture_map);
}

bool TetraMesh4D::validate_mesh_data() {
	const PackedInt32Array cell_indices = get_simplex_cell_indices();
	const int64_t cell_indices_count = cell_indices.size();
	if (cell_indices_count == 0) {
		return true; // Zero cells is allowed, and no need to validate anything else in that case.
	}
	ERR_FAIL_COND_V_MSG(cell_indices_count % 4 != 0, false, "TetraMesh4D: Cell indices count must be a multiple of 4 (" + itos(cell_indices_count) + " % 4 != 0).");
	const int64_t cell_texture_map_count = get_simplex_cell_texture_map().size();
	ERR_FAIL_COND_V_MSG(cell_texture_map_count > 0 && cell_texture_map_count != cell_indices_count, false, "TetraMesh4D: UVW texture map size (" + itos(cell_texture_map_count) + ") must match cell indices size (" + itos(cell_indices_count) + ").");
	const int64_t cell_normals_count = get_simplex_cell_boundary_normals().size();
	ERR_FAIL_COND_V_MSG(cell_normals_count > 0 && cell_normals_count * 4 != cell_indices_count, false, "TetraMesh4D: Boundary normals count (" + itos(cell_normals_count) + ") must have one per cell (expected " + itos(cell_indices_count / 4) + ")");
	const int64_t vertex_count = get_vertices().size();
	for (int32_t cell_index : cell_indices) {
		ERR_FAIL_COND_V_MSG(cell_index < 0 || cell_index >= vertex_count, false, "TetraMesh4D: Cell index " + itos(cell_index) + " is out of range (valid: 0-" + itos(vertex_count - 1) + ")");
	}
	return true;
}

void TetraMesh4D::validate_material_for_mesh(const Ref<Material4D> &p_material) {
	const Material4D::ColorSourceFlags albedo_source_flags = p_material->get_albedo_source_flags();
	if (albedo_source_flags & Material4D::COLOR_SOURCE_FLAG_PER_CELL) {
		const PackedInt32Array cell_indices = get_simplex_cell_indices();
		PackedColorArray color_array = p_material->get_albedo_color_array();
		const int cell_count = cell_indices.size() / 4;
		if (color_array.size() < cell_count) {
			p_material->resize_albedo_color_array(cell_count);
		}
	}
	Mesh4D::validate_material_for_mesh(p_material);
}

Ref<TetraMaterial4D> TetraMesh4D::_fallback_material;

Ref<Material4D> TetraMesh4D::get_fallback_material() {
	return _fallback_material;
}

void TetraMesh4D::init_fallback_material() {
	_fallback_material.instantiate();
}

void TetraMesh4D::cleanup_fallback_material() {
	_fallback_material.unref();
}

Ref<ArrayTetraMesh4D> TetraMesh4D::to_array_tetra_mesh() {
	Ref<ArrayTetraMesh4D> array_mesh;
	array_mesh.instantiate();
	array_mesh->set_vertices(get_vertices());
	array_mesh->set_simplex_cell_indices(get_simplex_cell_indices());
	array_mesh->set_simplex_cell_boundary_normals(get_simplex_cell_boundary_normals());
	array_mesh->set_simplex_cell_vertex_normals(get_simplex_cell_vertex_normals());
	array_mesh->set_simplex_cell_texture_map(get_simplex_cell_texture_map());
	array_mesh->set_material(get_material());
	array_mesh->set_name(get_name());
	// Copy metadata.
#if GDEXTENSION
	TypedArray<StringName> meta_list = get_meta_list();
	for (int i = 0; i < meta_list.size(); i++) {
		const StringName meta_key = meta_list[i];
		array_mesh->set_meta(meta_key, get_meta(meta_key));
	}
#elif GODOT_MODULE
	List<StringName> meta_list;
	get_meta_list(&meta_list);
	for (const StringName &meta_key : meta_list) {
		array_mesh->set_meta(meta_key, get_meta(meta_key));
	}
#endif
	return array_mesh;
}

Ref<TetraMesh4D> TetraMesh4D::to_tetra_mesh() {
	return to_array_tetra_mesh();
}

// Getters.

PackedInt32Array TetraMesh4D::get_simplex_cell_indices() {
	PackedInt32Array indices;
	GDVIRTUAL_CALL(_get_simplex_cell_indices, indices);
	return indices;
}

PackedVector4Array TetraMesh4D::get_simplex_cell_boundary_normals() {
	PackedVector4Array face_normals;
	GDVIRTUAL_CALL(_get_simplex_cell_boundary_normals, face_normals);
	return face_normals;
}

PackedVector4Array TetraMesh4D::get_simplex_cell_vertex_normals() {
	PackedVector4Array vertex_normals;
	GDVIRTUAL_CALL(_get_simplex_cell_vertex_normals, vertex_normals);
	return vertex_normals;
}

PackedVector3Array TetraMesh4D::get_simplex_cell_texture_map() {
	PackedVector3Array texture_map;
	GDVIRTUAL_CALL(_get_simplex_cell_texture_map, texture_map);
	return texture_map;
}

PackedInt32Array TetraMesh4D::calculate_edge_indices_from_simplex_cell_indices(const PackedInt32Array &p_simplex_cell_indices, const bool p_deduplicate) {
	PackedInt32Array edge_indices;
	edge_indices.resize(p_simplex_cell_indices.size() * 3);
	const int stop = p_simplex_cell_indices.size() / 4;
	for (int i = 0; i < stop; i++) {
		const int simplex_cell_index = i * 4;
		edge_indices.set(i * 12 + 0, p_simplex_cell_indices[simplex_cell_index + 0]);
		edge_indices.set(i * 12 + 1, p_simplex_cell_indices[simplex_cell_index + 1]);
		edge_indices.set(i * 12 + 2, p_simplex_cell_indices[simplex_cell_index + 0]);
		edge_indices.set(i * 12 + 3, p_simplex_cell_indices[simplex_cell_index + 2]);
		edge_indices.set(i * 12 + 4, p_simplex_cell_indices[simplex_cell_index + 0]);
		edge_indices.set(i * 12 + 5, p_simplex_cell_indices[simplex_cell_index + 3]);
		edge_indices.set(i * 12 + 6, p_simplex_cell_indices[simplex_cell_index + 1]);
		edge_indices.set(i * 12 + 7, p_simplex_cell_indices[simplex_cell_index + 2]);
		edge_indices.set(i * 12 + 8, p_simplex_cell_indices[simplex_cell_index + 1]);
		edge_indices.set(i * 12 + 9, p_simplex_cell_indices[simplex_cell_index + 3]);
		edge_indices.set(i * 12 + 10, p_simplex_cell_indices[simplex_cell_index + 2]);
		edge_indices.set(i * 12 + 11, p_simplex_cell_indices[simplex_cell_index + 3]);
	}
	if (p_deduplicate) {
		edge_indices = deduplicate_edge_indices(edge_indices);
	}
	return edge_indices;
}

PackedInt32Array TetraMesh4D::get_edge_indices() {
	if (_edge_indices_cache.is_empty()) {
		_edge_indices_cache = calculate_edge_indices_from_simplex_cell_indices(get_simplex_cell_indices(), true);
	}
	return _edge_indices_cache;
}

PackedVector4Array TetraMesh4D::get_edge_positions() {
	if (_edge_positions_cache.is_empty()) {
		const PackedInt32Array edge_indices = get_edge_indices();
		const PackedVector4Array vertices = get_vertices();
		const int32_t vertices_count = vertices.size();
		for (const int32_t edge_index : edge_indices) {
			ERR_FAIL_INDEX_V(edge_index, vertices_count, _simplex_positions_cache);
			_edge_positions_cache.append(vertices[edge_index]);
		}
	}
	return _edge_positions_cache;
}

PackedVector4Array TetraMesh4D::get_simplex_cell_positions() {
	if (_simplex_positions_cache.is_empty()) {
		const PackedInt32Array simplex_cell_indices = get_simplex_cell_indices();
		const PackedVector4Array vertices = get_vertices();
		const int32_t vertices_count = vertices.size();
		for (const int simplex_cell_index : simplex_cell_indices) {
			ERR_FAIL_INDEX_V(simplex_cell_index, vertices_count, _simplex_positions_cache);
			_simplex_positions_cache.append(vertices[simplex_cell_index]);
		}
	}
	return _simplex_positions_cache;
}

void TetraMesh4D::update_cross_section_mesh() {
	ERR_FAIL_COND(_cross_section_mesh.is_null());
	_cross_section_mesh->clear_surfaces();

	Ref<SurfaceTool> surface_tool;
	surface_tool.instantiate();
	surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
	surface_tool->set_smooth_group(-1);

	const PackedVector4Array cell_positions = get_simplex_cell_positions();
	const PackedVector4Array cell_normals = get_simplex_cell_boundary_normals();
	PackedVector3Array cell_tex_map = get_simplex_cell_texture_map();
	if (cell_tex_map.size() != cell_positions.size()) {
		if (cell_tex_map.size() == 0) {
			Ref<TetraMaterial4D> tetra_material = get_material();
			if (tetra_material.is_valid() && tetra_material->get_albedo_texture_3d().is_valid()) {
				WARN_PRINT("Mesh '" + get_name() + "' has an albedo texture but no texture map. The texture will not function correctly.");
			}
		} else {
			ERR_PRINT("Cell texture map size (" + itos(cell_tex_map.size()) + ") does not match cell positions size (" + itos(cell_positions.size()) + ").");
		}
		cell_tex_map.resize(cell_positions.size());
	}
	Ref<Material4D> material = get_material();
	if (material.is_valid()) {
		surface_tool->set_material(material->get_cross_section_material());
	}
	surface_tool->set_custom_format(0, SurfaceTool::CUSTOM_RGBA_FLOAT);
	surface_tool->set_custom_format(1, SurfaceTool::CUSTOM_RGBA_FLOAT);
	surface_tool->set_custom_format(2, SurfaceTool::CUSTOM_RGBA_FLOAT);
	surface_tool->set_custom_format(3, SurfaceTool::CUSTOM_RGBA_FLOAT);
	for (int i = 0; i < cell_positions.size(); i += 4) {
		// Cramming a bunch of data where it fits. Each cell's cross section can be 0-2 triangles. We create two triangles for each cell
		// with all the info about the cell and figure everything out in the vertex shader after transforms have been applied.
		// As of 4.4.1, available slots are:
		// - Vertex position (3)
		// - Custom 0-3 (4 * 4)
		// - UV1 and UV2 (2 * 2)
		// - Normal (3), gets normalized so ~2 slots for arbitrary floats
		// - Tangent (3), also gets normalized
		// - Color (4), gets clamped to [0, 1]
		// Slots that don't work:
		// - Bone weights: get truncated, sorted, and normalized automatically
		// - Binormal: available in shader, but computed in SurfaceTool from normal/tangent
		//
		// Some alternative strategies:
		// - ImmediateMesh to compute cross-section on the CPU every frame, but that's likely slower.
		// - Some kind of compute shader or custom render pipeline, but that's not supported on the Compatibility renderer.

		//// Shared attrs for both triangles:

		// Cell vertex positions: Using custom because there are conveniently four of them and they each take a vector4.
		surface_tool->set_custom(0, Vector4D::to_color(cell_positions[i]));
		surface_tool->set_custom(1, Vector4D::to_color(cell_positions[i + 1]));
		surface_tool->set_custom(2, Vector4D::to_color(cell_positions[i + 2]));
		surface_tool->set_custom(3, Vector4D::to_color(cell_positions[i + 3]));

		// UVW texture coords, need 4*3 float slots.  Using UV, UV2, Normal, Color, vertex.y, and vertex.z.
		// Normal gets normalized somewhere in the pipeline, so last coord of 1.0 will get set to whatever we need to divide by to get
		// to get the original coords.
		Vector3 uvw1 = cell_tex_map[i];
		Vector3 uvw2 = cell_tex_map[i + 1];
		Vector3 uvw3 = cell_tex_map[i + 2];
		Vector3 uvw4 = cell_tex_map[i + 3];
		surface_tool->set_uv(Vector2(uvw1.x, uvw1.y));
		surface_tool->set_uv2(Vector2(uvw2.x, uvw2.y));
		surface_tool->set_normal(Vector3(uvw3.x, uvw3.y, 1.0));
		// This one gets clamped to [0,1], which should be fine for texture coords.
		surface_tool->set_color(Color(uvw4.x, uvw4.y, uvw4.z, uvw1.z));

		// Not enough slots left for normals. Also interpolating the 4D normals gives weird results, needs more experimentation.
		// Currently flat normals are computed in the vertex shader.

		//// Vertices:

		// Not storing actual position data in the vertex positions, x is an index, y is UVW data, z is unused.
		surface_tool->add_vertex(Vector3(0.0, uvw2.z, uvw3.z));
		surface_tool->add_vertex(Vector3(1.0, uvw2.z, uvw3.z));
		surface_tool->add_vertex(Vector3(2.0, uvw2.z, uvw3.z));

		surface_tool->add_vertex(Vector3(3.0, uvw2.z, uvw3.z));
		surface_tool->add_vertex(Vector3(4.0, uvw2.z, uvw3.z));
		surface_tool->add_vertex(Vector3(5.0, uvw2.z, uvw3.z));
	}
	surface_tool->commit(_cross_section_mesh);

	// TODO Second surface for 4D "shadow" effect.
}

void TetraMesh4D::_bind_methods() {
	// Nearest point and distance.
	ClassDB::bind_method(D_METHOD("populate_inverse_metric_cache"), &TetraMesh4D::populate_inverse_metric_cache);
	ClassDB::bind_method(D_METHOD("get_signed_distance_to_mesh", "local_point"), &TetraMesh4D::get_signed_distance_to_mesh_bind);
	// Raycast.
	ClassDB::bind_method(D_METHOD("raycast_intersects_fast", "local_from", "local_direction"), &TetraMesh4D::raycast_intersects_fast);
	ClassDB::bind_method(D_METHOD("raycast_intersects", "local_from", "local_direction"), &TetraMesh4D::raycast_intersects);
	// Cache (validation is bound in base Mesh4D).
	ClassDB::bind_method(D_METHOD("tetra_mesh_clear_cache"), &TetraMesh4D::tetra_mesh_clear_cache);
	// Conversion.
	ClassDB::bind_method(D_METHOD("export_texture_map_mesh"), &TetraMesh4D::export_texture_map_mesh);
	ClassDB::bind_method(D_METHOD("to_array_tetra_mesh"), &TetraMesh4D::to_array_tetra_mesh);
	ClassDB::bind_method(D_METHOD("to_tetra_mesh"), &TetraMesh4D::to_tetra_mesh);
	// Getters.
	ClassDB::bind_method(D_METHOD("get_simplex_cell_indices"), &TetraMesh4D::get_simplex_cell_indices);
	ClassDB::bind_method(D_METHOD("get_simplex_cell_positions"), &TetraMesh4D::get_simplex_cell_positions);
	ClassDB::bind_method(D_METHOD("get_simplex_cell_boundary_normals"), &TetraMesh4D::get_simplex_cell_boundary_normals);
	ClassDB::bind_method(D_METHOD("get_simplex_cell_vertex_normals"), &TetraMesh4D::get_simplex_cell_vertex_normals);
	ClassDB::bind_method(D_METHOD("get_simplex_cell_texture_map"), &TetraMesh4D::get_simplex_cell_texture_map);

	GDVIRTUAL_BIND(_get_simplex_cell_indices);
	GDVIRTUAL_BIND(_get_simplex_cell_boundary_normals);
	GDVIRTUAL_BIND(_get_simplex_cell_vertex_normals);
	GDVIRTUAL_BIND(_get_simplex_cell_texture_map);
}
