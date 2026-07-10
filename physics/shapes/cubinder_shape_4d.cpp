#include "cubinder_shape_4d.h"

#include "../../model/mesh/poly/poly_mesh_builder_4d.h"

real_t CubinderShape4D::get_height() const {
	return _height;
}

void CubinderShape4D::set_height(const real_t p_height) {
	_height = p_height;
}

real_t CubinderShape4D::get_radius() const {
	return _radius;
}

void CubinderShape4D::set_radius(const real_t p_radius) {
	_radius = p_radius;
}

real_t CubinderShape4D::get_thickness() const {
	return _thickness;
}

void CubinderShape4D::set_thickness(const real_t p_thickness) {
	_thickness = p_thickness;
}

Vector2 CubinderShape4D::get_size_wy() const {
	return Vector2(_thickness, _height);
}

void CubinderShape4D::set_size_wy(const Vector2 &p_size_wy) {
	_thickness = p_size_wy.x;
	_height = p_size_wy.y;
}

real_t CubinderShape4D::get_hypervolume() const {
	// http://hi.gher.space/wiki/Cubinder
	return Math_PI * _radius * _radius * _height * _thickness;
}

real_t CubinderShape4D::get_surface_volume() const {
	// Wiki describes the surcell volume as 2π*r(r*h + h^2) but that only applies when height == thickness.
	// Distributing gives us 2π*h*r^2 + 2π*r*h^2, where:
	// * The 2π*r*h^2 term is the surcell volume of the square prism, 2π*r long with a h*h base (square).
	// * The 2π*h*r^2 term is the surcell volume of the cylinders, h long with an 2π*r^2 base (circle).
	// With different height and thickness, 2π*r*h^2 becomes 2π*r*h*t, and the cylinders become π*h*r^2 + π*t*r^2 so π*r^2 * (h+t).
	// Proof: Try substituting t with h and it should collapse to the original formulas.
	return Math_TAU * _radius * _height * _thickness + Math_PI * _radius * _radius * (_height + _thickness);
}

Dictionary CubinderShape4D::raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance, const bool p_inside_is_zero) const {
	Dictionary result;
	result["hit"] = false;
	ERR_FAIL_COND_V_MSG(!p_local_direction.is_normalized(), result, "CubinderShape4D::raycast_intersects: Ray direction must be normalized.");
	// Cubinder: circle in XZ plane, extruded in Y and W.
	// See also the similar code in `CylinderShape4D::raycast_intersects`.
	// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
	const real_t half_height = _height * 0.5f;
	const real_t half_thickness = _thickness * 0.5f;
	// Ray-infinite-cylinder intersection in the XZ plane.
	const Vector4 radial_point = Vector4(p_local_from.x, 0.0f, p_local_from.z, 0.0f);
	const real_t radial_point_len_sq = radial_point.length_squared();
	const real_t radius_sq = _radius * _radius;
	// Check if starting point is inside the cubinder (radially and within YW bounds).
	const bool start_inside = radial_point_len_sq <= radius_sq && Math::abs(p_local_from.y) <= half_height && Math::abs(p_local_from.w) <= half_thickness;
	if (p_inside_is_zero && start_inside) {
		result["hit"] = true;
		result["distance"] = 0.0f;
		result["normal"] = Vector4(0.0, 0.0, 0.0, 0.0);
		result["point"] = p_local_from;
		return result;
	}
	const Vector4 radial_dir = Vector4(p_local_direction.x, 0.0f, p_local_direction.z, 0.0f);
	const real_t radial_dir_len_sq = radial_dir.length_squared();
	const real_t radial_point_dot_radial_dir = radial_point.dot(radial_dir);
	Vector4 best_normal = Vector4();
	real_t best_distance = p_max_distance;
	// Check if the ray is parallel to the cubinder axes (Y and/or W axes).
	if (radial_dir_len_sq <= CMP_EPSILON2) {
		// Ray moves only along Y and/or W axes, parallel to the cubinder axes.
		// Check if we're inside the cylinder radially.
		if (radial_point_len_sq <= radius_sq && (Math::abs(p_local_direction.y) > CMP_EPSILON2 || Math::abs(p_local_direction.w) > CMP_EPSILON2)) {
			// We're inside radially, so we'll hit a Y or W cap.
			// Check Y caps.
			if (Math::abs(p_local_direction.y) > CMP_EPSILON2) {
				const real_t distance_to_y_cap = (p_local_direction.y > 0.0f) ? (half_height - p_local_from.y) / p_local_direction.y : (-half_height - p_local_from.y) / p_local_direction.y;
				if (distance_to_y_cap >= 0.0f && distance_to_y_cap < best_distance) {
					const Vector4 cap_point = p_local_from + p_local_direction * distance_to_y_cap;
					if (Math::abs(cap_point.w) <= half_thickness) {
						best_distance = distance_to_y_cap;
						best_normal = (p_local_direction.y > 0.0f) ? Vector4(0, 1, 0, 0) : Vector4(0, -1, 0, 0);
					}
				}
			}
			// Check W caps.
			if (Math::abs(p_local_direction.w) > CMP_EPSILON2) {
				const real_t distance_to_w_cap = (p_local_direction.w > 0.0f) ? (half_thickness - p_local_from.w) / p_local_direction.w : (-half_thickness - p_local_from.w) / p_local_direction.w;
				if (distance_to_w_cap >= 0.0f && distance_to_w_cap < best_distance) {
					const Vector4 cap_point = p_local_from + p_local_direction * distance_to_w_cap;
					if (Math::abs(cap_point.y) <= half_height) {
						best_distance = distance_to_w_cap;
						best_normal = (p_local_direction.w > 0.0f) ? Vector4(0, 0, 0, 1) : Vector4(0, 0, 0, -1);
					}
				}
			}
			if (best_distance < p_max_distance) {
				result["hit"] = true;
				result["distance"] = best_distance;
				result["normal"] = best_normal;
				result["point"] = p_local_from + p_local_direction * best_distance;
			}
		}
		// Regardless of whether we hit the surface or not, return here, because the ray is perpendicular to the curved surface.
		return result;
	}
	const real_t discriminant = radial_point_dot_radial_dir * radial_point_dot_radial_dir - radial_dir_len_sq * (radial_point_len_sq - radius_sq);
	if (discriminant < 0.0f) {
		return result; // No intersection with infinite cylinder.
	}
	const real_t sqrt_discriminant = Math::sqrt(discriminant);
	if (start_inside) {
		// Ray starts inside, use exit point.
		const real_t distance_to_exit = (-radial_point_dot_radial_dir + sqrt_discriminant) / radial_dir_len_sq;
		if (distance_to_exit >= 0.0f) {
			const Vector4 exit_point = p_local_from + p_local_direction * distance_to_exit;
			if (Math::abs(exit_point.y) <= half_height && Math::abs(exit_point.w) <= half_thickness) {
				// Exit through curved surface.
				const Vector4 hit_radial = Vector4(exit_point.x, 0.0f, exit_point.z, 0.0f);
				const Vector4 normal = hit_radial.normalized();
				best_distance = distance_to_exit;
				result["hit"] = true;
				result["distance"] = distance_to_exit;
				result["normal"] = normal;
				return result;
			}
		}
		// Exit point is outside YW bounds, check rectangular caps.
		if (Math::abs(p_local_direction.y) > CMP_EPSILON2) {
			const real_t distance_to_y_cap = (p_local_direction.y > 0.0f) ? (half_height - p_local_from.y) / p_local_direction.y : (-half_height - p_local_from.y) / p_local_direction.y;
			if (distance_to_y_cap >= 0.0f && distance_to_y_cap < best_distance) {
				const Vector4 cap_point = p_local_from + p_local_direction * distance_to_y_cap;
				const Vector4 cap_radial = Vector4(cap_point.x, 0.0f, cap_point.z, 0.0f);
				if (cap_radial.length_squared() <= radius_sq && Math::abs(cap_point.w) <= half_thickness) {
					best_distance = distance_to_y_cap;
					best_normal = (p_local_direction.y > 0.0f) ? Vector4(0, 1, 0, 0) : Vector4(0, -1, 0, 0);
				}
			}
		}
		if (Math::abs(p_local_direction.w) > CMP_EPSILON2) {
			const real_t distance_to_w_cap = (p_local_direction.w > 0.0f) ? (half_thickness - p_local_from.w) / p_local_direction.w : (-half_thickness - p_local_from.w) / p_local_direction.w;
			if (distance_to_w_cap >= 0.0f && distance_to_w_cap < best_distance) {
				const Vector4 cap_point = p_local_from + p_local_direction * distance_to_w_cap;
				const Vector4 cap_radial = Vector4(cap_point.x, 0.0f, cap_point.z, 0.0f);
				if (cap_radial.length_squared() <= radius_sq && Math::abs(cap_point.y) <= half_height) {
					best_distance = distance_to_w_cap;
					best_normal = (p_local_direction.w > 0.0f) ? Vector4(0, 0, 0, 1) : Vector4(0, 0, 0, -1);
				}
			}
		}
		if (best_distance < p_max_distance) {
			result["hit"] = true;
			result["distance"] = best_distance;
			result["normal"] = best_normal;
			result["point"] = p_local_from + p_local_direction * best_distance;
		}
		// Regardless of whether we hit the surface or not, return here, because we started inside and this is the end of that case.
		return result;
	}
	// Ray starts outside.
	// Check all four rectangular caps.
	if (Math::abs(p_local_direction.y) > CMP_EPSILON2) {
		// Try Y caps (positive and negative).
		const real_t distance_to_y_cap_pos = (half_height - p_local_from.y) / p_local_direction.y;
		if (distance_to_y_cap_pos >= 0.0f && distance_to_y_cap_pos < best_distance) {
			const Vector4 cap_point = p_local_from + p_local_direction * distance_to_y_cap_pos;
			const Vector4 cap_radial = Vector4(cap_point.x, 0.0f, cap_point.z, 0.0f);
			if (cap_radial.length_squared() <= radius_sq && Math::abs(cap_point.w) <= half_thickness) {
				best_distance = distance_to_y_cap_pos;
				best_normal = Vector4(0, 1, 0, 0);
				result["hit"] = true;
				result["distance"] = distance_to_y_cap_pos;
				result["normal"] = best_normal;
			}
		}
		const real_t distance_to_y_cap_neg = (-half_height - p_local_from.y) / p_local_direction.y;
		if (distance_to_y_cap_neg >= 0.0f && distance_to_y_cap_neg < best_distance) {
			const Vector4 cap_point = p_local_from + p_local_direction * distance_to_y_cap_neg;
			const Vector4 cap_radial = Vector4(cap_point.x, 0.0f, cap_point.z, 0.0f);
			if (cap_radial.length_squared() <= radius_sq && Math::abs(cap_point.w) <= half_thickness) {
				best_distance = distance_to_y_cap_neg;
				best_normal = Vector4(0, -1, 0, 0);
				result["hit"] = true;
				result["distance"] = distance_to_y_cap_neg;
				result["normal"] = best_normal;
			}
		}
	}
	if (Math::abs(p_local_direction.w) > CMP_EPSILON2) {
		// Try W caps (positive and negative).
		const real_t distance_to_w_cap_pos = (half_thickness - p_local_from.w) / p_local_direction.w;
		if (distance_to_w_cap_pos >= 0.0f && distance_to_w_cap_pos < best_distance) {
			const Vector4 cap_point = p_local_from + p_local_direction * distance_to_w_cap_pos;
			const Vector4 cap_radial = Vector4(cap_point.x, 0.0f, cap_point.z, 0.0f);
			if (cap_radial.length_squared() <= radius_sq && Math::abs(cap_point.y) <= half_height) {
				best_distance = distance_to_w_cap_pos;
				best_normal = Vector4(0, 0, 0, 1);
				result["hit"] = true;
				result["distance"] = distance_to_w_cap_pos;
				result["normal"] = best_normal;
				result["point"] = cap_point;
			}
		}
		const real_t distance_to_w_cap_neg = (-half_thickness - p_local_from.w) / p_local_direction.w;
		if (distance_to_w_cap_neg >= 0.0f && distance_to_w_cap_neg < best_distance) {
			const Vector4 cap_point = p_local_from + p_local_direction * distance_to_w_cap_neg;
			const Vector4 cap_radial = Vector4(cap_point.x, 0.0f, cap_point.z, 0.0f);
			if (cap_radial.length_squared() <= radius_sq && Math::abs(cap_point.y) <= half_height) {
				best_distance = distance_to_w_cap_neg;
				best_normal = Vector4(0, 0, 0, -1);
				result["hit"] = true;
				result["distance"] = distance_to_w_cap_neg;
				result["normal"] = best_normal;
				result["point"] = cap_point;
			}
		}
	}
	// Check curved surface entrance (only if no cap hit or curved surface is closer).
	const real_t distance_to_entrance = (-radial_point_dot_radial_dir - sqrt_discriminant) / radial_dir_len_sq;
	if (distance_to_entrance >= 0.0f && distance_to_entrance < best_distance) {
		const Vector4 entrance_point = p_local_from + p_local_direction * distance_to_entrance;
		if (Math::abs(entrance_point.y) <= half_height && Math::abs(entrance_point.w) <= half_thickness) {
			// Entrance through curved surface.
			const Vector4 hit_radial = Vector4(entrance_point.x, 0.0f, entrance_point.z, 0.0f);
			const Vector4 normal = hit_radial.normalized();
			best_distance = distance_to_entrance;
			result["hit"] = true;
			result["distance"] = distance_to_entrance;
			result["normal"] = normal;
			result["point"] = entrance_point;
		}
	}
	return result;
}

real_t CubinderShape4D::get_signed_distance_to_surface(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_surface) const {
	const real_t half_height = _height * 0.5f;
	const real_t half_thickness = _thickness * 0.5f;
	Vector4 nearest = Vector4(p_local_point.x, 0.0f, p_local_point.z, 0.0f);
	const real_t flat_length = nearest.length();
	const real_t radial_signed_distance = flat_length - _radius;
	const real_t vertical_signed_distance = Math::abs(p_local_point.y) - half_height;
	const real_t thickness_signed_distance = Math::abs(p_local_point.w) - half_thickness;
	const real_t abs_radial_signed_distance = Math::abs(radial_signed_distance);
	const real_t abs_vertical_signed_distance = Math::abs(vertical_signed_distance);
	const real_t abs_thickness_signed_distance = Math::abs(thickness_signed_distance);
	if (r_nearest_point_on_surface != nullptr) {
		if (radial_signed_distance > 0.0f || abs_radial_signed_distance >= abs_vertical_signed_distance || abs_radial_signed_distance >= abs_thickness_signed_distance) {
			if (flat_length == 0.0f) {
				nearest = Vector4(_radius, 0.0f, 0.0f, 0.0f);
			} else {
				nearest *= _radius / flat_length;
			}
		}
		if (vertical_signed_distance > 0.0f || abs_vertical_signed_distance > abs_radial_signed_distance) {
			nearest.y = (p_local_point.y > 0.0f) ? half_height : -half_height;
		} else {
			nearest.y = p_local_point.y;
		}
		if (thickness_signed_distance > 0.0f || abs_thickness_signed_distance > abs_radial_signed_distance) {
			nearest.w = (p_local_point.w > 0.0f) ? half_thickness : -half_thickness;
		} else {
			nearest.w = p_local_point.w;
		}
		*r_nearest_point_on_surface = nearest;
	}
	// Return the smallest signed distance, which corresponds to the closest surface.
	if (abs_thickness_signed_distance < abs_radial_signed_distance && abs_thickness_signed_distance < abs_vertical_signed_distance) {
		return thickness_signed_distance;
	}
	if (abs_vertical_signed_distance < abs_radial_signed_distance) {
		return vertical_signed_distance;
	}
	return radial_signed_distance;
}

Vector4 CubinderShape4D::get_nearest_point(const Vector4 &p_local_point) const {
	const real_t half_height = _height * (real_t)0.5;
	const real_t half_thickness = _thickness * (real_t)0.5;
	Vector4 nearest = Vector4(p_local_point.x, 0.0f, p_local_point.z, 0.0f);
	const real_t length_sq = nearest.length_squared();
	if (length_sq > _radius * _radius) {
		nearest = nearest * _radius / Math::sqrt(length_sq);
	}
	nearest.y = CLAMP(p_local_point.y, -half_height, half_height);
	nearest.w = CLAMP(p_local_point.w, -half_thickness, half_thickness);
	return nearest;
}

Vector4 CubinderShape4D::get_support_point(const Vector4 &p_local_direction) const {
	const real_t half_height = _height * (real_t)0.5;
	const real_t half_thickness = _thickness * (real_t)0.5;
	Vector4 support = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	if (p_local_direction.x != 0.0f || p_local_direction.z != 0.0f) {
		Vector2 circle_dir = Vector2(p_local_direction.x, p_local_direction.z).normalized() * _radius;
		support.x = circle_dir.x;
		support.z = circle_dir.y;
	}
	support.y = (p_local_direction.y > 0.0f) ? half_height : -half_height;
	support.w = (p_local_direction.w > 0.0f) ? half_thickness : -half_thickness;
	return support;
}

bool CubinderShape4D::has_point(const Vector4 &p_local_point) const {
	Vector4 abs_point = p_local_point.abs();
	if (abs_point.y > _height * 0.5f) {
		return false;
	}
	abs_point.y = 0.0f;
	if (abs_point.w > _thickness * 0.5f) {
		return false;
	}
	abs_point.w = 0.0f;
	return abs_point.length_squared() <= _radius * _radius;
}

bool CubinderShape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	Ref<CubinderShape4D> cubinder = p_shape;
	if (cubinder.is_null()) {
		return false;
	}
	return _height == cubinder->_height && _radius == cubinder->_radius && _thickness == cubinder->_thickness;
}

Ref<PolyMesh4D> CubinderShape4D::to_poly_mesh(const Dictionary &p_options) const {
	const int64_t segments = p_options.has("segments") ? (int64_t)p_options["segments"] : (int64_t)16;
	ERR_FAIL_COND_V(segments < 3, Ref<PolyMesh4D>());
	// Create a base circle for the cubinder in the XZ plane.
	PackedVector4Array circle_vertices;
	PackedInt32Array circle_edge_indices;
	{
		circle_vertices.resize(segments);
		circle_edge_indices.resize(2 * segments);
		int vert_iter = 0;
		Vector4 point = Vector4(0.0, 0.0, 0.0, 0.0);
		const int ring_start_vert = vert_iter;
		point[0] = _radius;
		circle_vertices.set(vert_iter, point);
		for (int i = 1; i < segments; i++) {
			const double angle = (double)i * (Math_TAU / (double)segments);
			point[0] = _radius * Math::cos(angle);
			point[2] = _radius * Math::sin(angle);
			circle_edge_indices.set(vert_iter * 2, vert_iter);
			circle_edge_indices.set(vert_iter * 2 + 1, vert_iter + 1);
			vert_iter++;
			circle_vertices.set(vert_iter, point);
		}
		circle_edge_indices.set(vert_iter * 2, ring_start_vert);
		circle_edge_indices.set(vert_iter * 2 + 1, vert_iter);
		vert_iter++;
	}
	// Create a single face from the ring of vertices.
	PackedInt32Array circle_face_edge_indices;
	circle_face_edge_indices.resize(segments);
	for (int i = 0; i < segments; i++) {
		circle_face_edge_indices.set(i, i);
	}
	// Double pack: Outer is for dimension (faces vs cells), inner is elements of the dimension (each face).
	const Vector<Vector<PackedInt32Array>> circle_poly_cell_indices = { { circle_face_edge_indices } };
	// Set the data into a poly mesh.
	Ref<ArrayPolyMesh4D> ret;
	ret.instantiate();
	ret->set_poly_cell_vertices(circle_vertices);
	ret->set_edge_vertex_indices(circle_edge_indices);
	ret->set_poly_cell_indices(circle_poly_cell_indices);
	CRASH_COND(!ret->is_mesh_data_valid());
	// Extrude that circle along the height and thickness.
	// The extrude_linear's extrusion vector is +/- the vector given, so we need to use the half extents.
	const real_t height_half_extent = _height * (real_t)0.5;
	const real_t thickness_half_extent = _thickness * (real_t)0.5;
	ret = PolyMeshBuilder4D::extrude_linear(ret, Vector4(0.0, height_half_extent, 0.0, 0.0));
	ret = PolyMeshBuilder4D::extrude_linear(ret, Vector4(0.0, 0.0, 0.0, thickness_half_extent));
	// Set custom seams.
	const PackedVector4Array poly_vertices = ret->get_poly_cell_vertices();
	const Vector<PackedInt32Array> face_vertex_indices = ret->get_all_face_vertex_indices();
	HashSet<int32_t> seam_face_indices;
	for (int64_t face_index = 0; face_index < face_vertex_indices.size(); face_index++) {
		const PackedInt32Array &face = face_vertex_indices[face_index];
		const Vector4 &vert1 = poly_vertices[face[1]];
		const Vector4 &vert3 = poly_vertices[face[3]];
		// Seam face if exactly one of Y or W differs between diagonally-opposite vertices.
		if ((vert1.y != vert3.y) != (vert1.w != vert3.w)) {
			seam_face_indices.insert(face_index);
		}
	}
	ret->set_seam_face_indices(seam_face_indices);
	return ret;
}

Ref<TetraMesh4D> CubinderShape4D::to_tetra_mesh(const Dictionary &p_options) const {
	return to_poly_mesh(p_options);
}

Ref<WireMesh4D> CubinderShape4D::to_wire_mesh(const Dictionary &p_options) const {
	return to_poly_mesh(p_options)->to_wire_mesh();
}

void CubinderShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_height"), &CubinderShape4D::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &CubinderShape4D::set_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_NONE, "suffix:m"), "set_height", "get_height");

	ClassDB::bind_method(D_METHOD("get_radius"), &CubinderShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CubinderShape4D::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_NONE, "suffix:m"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("get_thickness"), &CubinderShape4D::get_thickness);
	ClassDB::bind_method(D_METHOD("set_thickness", "thickness"), &CubinderShape4D::set_thickness);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "thickness", PROPERTY_HINT_NONE, "suffix:m"), "set_thickness", "get_thickness");

	ClassDB::bind_method(D_METHOD("get_size_wy"), &CubinderShape4D::get_size_wy);
	ClassDB::bind_method(D_METHOD("set_size_wy", "size_wy"), &CubinderShape4D::set_size_wy);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "size_wy", PROPERTY_HINT_NONE, "suffix:m", PROPERTY_USAGE_NONE), "set_size_wy", "get_size_wy");
}
