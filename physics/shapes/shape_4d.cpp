#include "shape_4d.h"

#include "../../model/mesh/tetra/tetra_mesh_4d.h"
#include "../../model/mesh/wire/wire_mesh_4d.h"

real_t Shape4D::get_hypervolume() const {
	real_t hypervolume = 0.0;
	GDVIRTUAL_CALL(_get_hypervolume, hypervolume);
	return hypervolume;
}

real_t Shape4D::get_surface_volume() const {
	real_t surface_volume = 0.0;
	GDVIRTUAL_CALL(_get_surface_volume, surface_volume);
	return surface_volume;
}

Rect4 Shape4D::get_rect_bounds(const Transform4D &p_to_target) const {
	PackedVector4Array result;
	GDVIRTUAL_CALL(_get_rect_bounds, p_to_target.basis, p_to_target.origin, result);
	if (result.size() == 2) {
		return Rect4(result[0], result[1]);
	}
	// General rect bounds implementation that uses get_support_point to work for all shapes.
	// Some shapes provide their own custom implementation instead (ex: box and orthoplex).
	Rect4 bounds = Rect4(p_to_target.origin, Vector4());
	const Basis4D transposed = p_to_target.basis.transposed();
	for (int sign = 0; sign < 2; sign++) {
		for (int axis = 0; axis < 4; axis++) {
			// Check for a support point in the direction of the transpose's axes.
			// This gives us, after transforming to the target space, the farthest point in the cardinal directions.
			Vector4 support = get_support_point(sign == 0 ? -transposed[axis] : transposed[axis]);
			bounds = bounds.expand_to_point(p_to_target * support);
		}
	}
	return bounds;
}

PackedVector4Array Shape4D::get_rect_bounds_bind(const Projection &p_to_target_basis, const Vector4 &p_to_target_origin) const {
	Rect4 bounds = get_rect_bounds(Transform4D(p_to_target_basis, p_to_target_origin));
	return PackedVector4Array({ bounds.position, bounds.size });
}

// Raycasting.

Dictionary Shape4D::raycast_intersects(const Vector4 &p_local_from, const Vector4 &p_local_direction, const real_t p_max_distance, const bool p_inside_is_zero) const {
	Dictionary result;
	GDVIRTUAL_CALL(_raycast_intersects, p_local_from, p_local_direction, p_max_distance, p_inside_is_zero, result);
	if (!result.is_empty()) {
		return result;
	}
	// Fallback implementation that uses get_rect_bounds to work for all shapes.
	const Rect4 bounds = get_rect_bounds();
	return bounds.raycast_intersects_dict(p_local_from, p_local_direction, p_max_distance, p_inside_is_zero);
}

real_t Shape4D::get_signed_distance_to_surface(const Vector4 &p_local_point, Vector4 *r_nearest_point_on_surface) const {
	real_t signed_distance = Math_NAN;
	GDVIRTUAL_CALL(_get_signed_distance_to_surface, p_local_point, signed_distance);
#if GDEXTENSION
	constexpr bool ERR_HANDLER_WARNING = true;
#endif
	if (r_nearest_point_on_surface != nullptr || Math::is_nan(signed_distance)) {
		const Vector4 nearest_point = get_nearest_point(p_local_point);
		if (r_nearest_point_on_surface != nullptr) {
			// Manual expansion of WARN_PRINT_ONCE macro to skip numeric comparison when already printed.
			static bool should_show_signed_distance_nearest_point_warning = true;
			if (unlikely(should_show_signed_distance_nearest_point_warning) && signed_distance < 0.0) {
				should_show_signed_distance_nearest_point_warning = false;
				_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Shape4D::get_signed_distance_to_surface: The nearest point on the surface may be incorrect for this custom shape. This warning will only be printed once.", false, ERR_HANDLER_WARNING);
			}
			*r_nearest_point_on_surface = nearest_point;
		}
		if (Math::is_nan(signed_distance)) {
			signed_distance = (nearest_point - p_local_point).length();
			// Manual expansion of WARN_PRINT_ONCE macro to skip numeric comparison when already printed.
			static bool should_show_signed_distance_lower_warning = true;
			if (unlikely(should_show_signed_distance_lower_warning) && signed_distance <= 0.0) {
				should_show_signed_distance_lower_warning = false;
				_err_print_error(FUNCTION_STR, __FILE__, __LINE__, "Shape4D::get_signed_distance_to_surface: The true signed distance may be lower than this value. This warning will only be printed once.", false, ERR_HANDLER_WARNING);
			}
		}
	}
	return signed_distance;
}

real_t Shape4D::get_signed_distance_to_surface_bind(const Vector4 &p_local_point) const {
	return get_signed_distance_to_surface(p_local_point, nullptr);
}

Vector4 Shape4D::get_nearest_point(const Vector4 &p_local_point) const {
	Vector4 nearest_point;
	GDVIRTUAL_CALL(_get_nearest_point, p_local_point, nearest_point);
	return nearest_point;
}

Vector4 Shape4D::get_support_point(const Vector4 &p_local_direction) const {
	Vector4 support_point;
	GDVIRTUAL_CALL(_get_support_point, p_local_direction, support_point);
	return support_point;
}

bool Shape4D::has_point(const Vector4 &p_local_point) const {
	bool has = false;
	GDVIRTUAL_CALL(_has_point, p_local_point, has);
	return has;
}

bool Shape4D::is_equal_exact(const Ref<Shape4D> &p_shape) const {
	bool equal = false;
	GDVIRTUAL_CALL(_is_equal_exact, p_shape, equal);
	return equal;
}

Ref<PolyMesh4D> Shape4D::to_poly_mesh(const Dictionary &p_options) const {
	Ref<PolyMesh4D> poly_mesh;
	GDVIRTUAL_CALL(_to_poly_mesh, p_options, poly_mesh);
	return poly_mesh;
}

Ref<TetraMesh4D> Shape4D::to_tetra_mesh(const Dictionary &p_options) const {
	Ref<TetraMesh4D> tetra_mesh;
	GDVIRTUAL_CALL(_to_tetra_mesh, p_options, tetra_mesh);
	return tetra_mesh;
}

Ref<WireMesh4D> Shape4D::to_wire_mesh(const Dictionary &p_options) const {
	Ref<WireMesh4D> wire_mesh;
	GDVIRTUAL_CALL(_to_wire_mesh, p_options, wire_mesh);
	return wire_mesh;
}

void Shape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_hypervolume"), &Shape4D::get_hypervolume);
	ClassDB::bind_method(D_METHOD("get_surface_volume"), &Shape4D::get_surface_volume);
	ClassDB::bind_method(D_METHOD("get_rect_bounds", "to_target_basis", "to_target_origin"), &Shape4D::get_rect_bounds_bind, DEFVAL(Projection()), DEFVAL(Vector4()));
	ClassDB::bind_method(D_METHOD("get_signed_distance_to_surface", "local_point"), &Shape4D::get_signed_distance_to_surface_bind);
	ClassDB::bind_method(D_METHOD("raycast_intersects", "local_from", "local_direction", "max_distance", "inside_is_zero"), &Shape4D::raycast_intersects);
	ClassDB::bind_method(D_METHOD("get_nearest_point", "point"), &Shape4D::get_nearest_point);
	ClassDB::bind_method(D_METHOD("get_support_point", "direction"), &Shape4D::get_support_point);
	ClassDB::bind_method(D_METHOD("has_point", "point"), &Shape4D::has_point);
	ClassDB::bind_method(D_METHOD("is_equal_exact", "shape"), &Shape4D::is_equal_exact);
	ClassDB::bind_method(D_METHOD("to_poly_mesh", "options"), &Shape4D::to_poly_mesh, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("to_tetra_mesh", "options"), &Shape4D::to_tetra_mesh, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("to_wire_mesh", "options"), &Shape4D::to_wire_mesh, DEFVAL(Dictionary()));
	// Bind the virtual methods to allow users to make their own shapes.
	GDVIRTUAL_BIND(_get_hypervolume);
	GDVIRTUAL_BIND(_get_surface_volume);
	GDVIRTUAL_BIND(_get_rect_bounds, "inv_relative_to_basis", "inv_relative_to_origin");
	GDVIRTUAL_BIND(_get_signed_distance_to_surface, "local_point");
	GDVIRTUAL_BIND(_raycast_intersects, "local_from", "local_direction", "max_distance", "inside_is_zero");
	GDVIRTUAL_BIND(_get_nearest_point, "point");
	GDVIRTUAL_BIND(_get_support_point, "direction");
	GDVIRTUAL_BIND(_has_point, "point");
	GDVIRTUAL_BIND(_is_equal_exact, "shape");
	GDVIRTUAL_BIND(_to_poly_mesh, "options");
	GDVIRTUAL_BIND(_to_tetra_mesh, "options");
	GDVIRTUAL_BIND(_to_wire_mesh, "options");
}
