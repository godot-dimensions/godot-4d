#include "basis_4d_bind.h"

#include "basis_4d.h"

// Misc methods.

real_t godot_4d_bind::Basis4D::determinant(const Projection &p_basis) {
	return ::Basis4D(p_basis).determinant();
}

bool godot_4d_bind::Basis4D::is_equal_approx(const Projection &p_a, const Projection &p_b) {
	return ::Basis4D(p_a).is_equal_approx(::Basis4D(p_b));
}

// Interpolation.

Projection godot_4d_bind::Basis4D::lerp(const Projection &p_from, const Projection &p_to, const real_t p_weight) {
	return ::Basis4D(p_from).lerp(::Basis4D(p_to), p_weight);
}

Projection godot_4d_bind::Basis4D::slerp(const Projection &p_from, const Projection &p_to, const real_t p_weight) {
	return ::Basis4D(p_from).slerp(::Basis4D(p_to), p_weight);
}

Projection godot_4d_bind::Basis4D::slerp_rotation(const Projection &p_from, const Projection &p_to, const real_t p_weight) {
	return ::Basis4D(p_from).slerp_rotation(::Basis4D(p_to), p_weight);
}

// Transformation methods.

Projection godot_4d_bind::Basis4D::compose(const Projection &p_parent, const Projection &p_child) {
	return ::Basis4D(p_parent) * ::Basis4D(p_child);
}

Projection godot_4d_bind::Basis4D::transform_to(const Projection &p_from, const Projection &p_to) {
	return ::Basis4D(p_from).transform_to(::Basis4D(p_to));
}

Vector4 godot_4d_bind::Basis4D::xform(const Projection &p_basis, const Vector4 &p_vector) {
	return ::Basis4D(p_basis).xform(p_vector);
}

Vector4 godot_4d_bind::Basis4D::xform_inv(const Projection &p_basis, const Vector4 &p_vector) {
	return ::Basis4D(p_basis).xform_inv(p_vector);
}

Vector4 godot_4d_bind::Basis4D::xform_transposed(const Projection &p_basis, const Vector4 &p_vector) {
	return ::Basis4D(p_basis).xform_transposed(p_vector);
}

// Rotation methods.

AABB godot_4d_bind::Basis4D::rotate_bivector(const Projection &p_basis, const AABB &p_bivector) {
	return ::Basis4D(p_basis).rotate_bivector(p_bivector);
}

Projection godot_4d_bind::Basis4D::rotate_plane_global(const Projection &p_basis, const Vector4 &p_plane_from, const Vector4 &p_plane_to, const real_t p_angle) {
	return ::Basis4D(p_basis).rotate_plane_global(p_plane_from, p_plane_to, p_angle);
}

Projection godot_4d_bind::Basis4D::rotate_plane_local(const Projection &p_basis, const Vector4 &p_local_plane_from, const Vector4 &p_local_plane_to, const real_t p_angle) {
	return ::Basis4D(p_basis).rotate_plane_local(p_local_plane_from, p_local_plane_to, p_angle);
}

// Inversion methods.

Projection godot_4d_bind::Basis4D::inverse(const Projection &p_basis) {
	return ::Basis4D(p_basis).inverse();
}

Projection godot_4d_bind::Basis4D::transposed(const Projection &p_basis) {
	return ::Basis4D(p_basis).transposed();
}

// Scale methods.

Projection godot_4d_bind::Basis4D::scaled_global(const Projection &p_basis, const Vector4 &p_scale) {
	return ::Basis4D(p_basis).scaled_global(p_scale);
}

Projection godot_4d_bind::Basis4D::scaled_local(const Projection &p_basis, const Vector4 &p_scale) {
	return ::Basis4D(p_basis).scaled_local(p_scale);
}

Projection godot_4d_bind::Basis4D::scaled_uniform(const Projection &p_basis, const real_t p_scale) {
	return ::Basis4D(p_basis).scaled_uniform(p_scale);
}

Vector4 godot_4d_bind::Basis4D::get_scale(const Projection &p_basis) {
	return ::Basis4D(p_basis).get_scale();
}

Vector4 godot_4d_bind::Basis4D::get_scale_abs(const Projection &p_basis) {
	return ::Basis4D(p_basis).get_scale_abs();
}

real_t godot_4d_bind::Basis4D::get_uniform_scale(const Projection &p_basis) {
	return ::Basis4D(p_basis).get_uniform_scale();
}

// Validation methods.

Projection godot_4d_bind::Basis4D::orthonormalized(const Projection &p_basis) {
	return ::Basis4D(p_basis).orthonormalized();
}

Projection godot_4d_bind::Basis4D::orthogonalized(const Projection &p_basis) {
	return ::Basis4D(p_basis).orthogonalized();
}

bool godot_4d_bind::Basis4D::is_orthogonal(const Projection &p_basis) {
	return ::Basis4D(p_basis).is_orthogonal();
}

bool godot_4d_bind::Basis4D::is_orthonormal(const Projection &p_basis) {
	return ::Basis4D(p_basis).is_orthonormal();
}

bool godot_4d_bind::Basis4D::is_conformal(const Projection &p_basis) {
	return ::Basis4D(p_basis).is_conformal();
}

bool godot_4d_bind::Basis4D::is_diagonal(const Projection &p_basis) {
	return ::Basis4D(p_basis).is_diagonal();
}

bool godot_4d_bind::Basis4D::is_rotation(const Projection &p_basis) {
	return ::Basis4D(p_basis).is_rotation();
}

// Helper setters/getters.

Vector4 godot_4d_bind::Basis4D::get_column(const Projection &p_basis, int p_index) {
	return ::Basis4D(p_basis).get_column(p_index);
}

Vector4 godot_4d_bind::Basis4D::get_row(const Projection &p_basis, int p_index) {
	return ::Basis4D(p_basis).get_row(p_index);
}

Vector4 godot_4d_bind::Basis4D::get_main_diagonal(const Projection &p_basis) {
	return ::Basis4D(p_basis).get_main_diagonal();
}

// Helper Euler4D and Rotor4D functions. Can't be done internally
// because of dependency order but exposed for convenience.

Projection godot_4d_bind::Basis4D::from_euler(const Ref<Euler4D> &p_euler) {
	return p_euler->to_basis();
}

Projection godot_4d_bind::Basis4D::from_euler_aabb(const AABB &p_euler) {
	return ::Euler4D(p_euler).to_basis();
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Basis4D::to_euler(const Projection &p_basis, const bool p_use_special_cases) {
	return Euler4D::from_basis(::Basis4D(p_basis), p_use_special_cases);
}

AABB godot_4d_bind::Basis4D::to_euler_aabb(const Projection &p_basis, const bool p_use_special_cases) {
	return ::Euler4D::from_basis(::Basis4D(p_basis), p_use_special_cases);
}

Projection godot_4d_bind::Basis4D::from_rotor(const Ref<Rotor4D> &p_rotor) {
	return p_rotor->to_basis();
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Basis4D::to_rotor(const Projection &p_basis) {
	return Rotor4D::from_basis(::Basis4D(p_basis));
}

// Conversion.

Projection godot_4d_bind::Basis4D::from_3d(const Basis &p_from_3d) {
	return ::Basis4D::from_3d(p_from_3d);
}

Basis godot_4d_bind::Basis4D::to_3d(const Projection &p_from_4d, const bool p_orthonormalized) {
	return ::Basis4D(p_from_4d).to_3d(p_orthonormalized);
}

Projection godot_4d_bind::Basis4D::from_array(const PackedRealArray &p_from_array) {
	return ::Basis4D::from_array(p_from_array);
}

PackedRealArray godot_4d_bind::Basis4D::to_array(const Projection &p_from_basis) {
	return ::Basis4D(p_from_basis).to_array();
}

// Constructors.

Projection godot_4d_bind::Basis4D::from_scale(const Vector4 &p_scale) {
	return ::Basis4D::from_scale(p_scale);
}

Projection godot_4d_bind::Basis4D::from_scale_uniform(const real_t p_scale) {
	return ::Basis4D::from_scale_uniform(p_scale);
}

Projection godot_4d_bind::Basis4D::from_yz(const real_t p_yz) {
	return ::Basis4D::from_yz(p_yz);
}

Projection godot_4d_bind::Basis4D::from_zx(const real_t p_zx) {
	return ::Basis4D::from_zx(p_zx);
}

Projection godot_4d_bind::Basis4D::from_xy(const real_t p_xy) {
	return ::Basis4D::from_xy(p_xy);
}

Projection godot_4d_bind::Basis4D::from_xw(const real_t p_xw) {
	return ::Basis4D::from_xw(p_xw);
}

Projection godot_4d_bind::Basis4D::from_wy(const real_t p_wy) {
	return ::Basis4D::from_wy(p_wy);
}

Projection godot_4d_bind::Basis4D::from_zw(const real_t p_zw) {
	return ::Basis4D::from_zw(p_zw);
}

Projection godot_4d_bind::Basis4D::from_swap_rotation(const int p_rot_from, const int p_rot_to) {
	return ::Basis4D::from_swap_rotation(p_rot_from, p_rot_to);
}

Projection godot_4d_bind::Basis4D::looking_at(const Vector4 &p_target, const Vector4 &p_perp, const Vector4 &p_up, bool p_use_model_front) {
	return ::Basis4D::looking_at(p_target, p_perp, p_up, p_use_model_front);
}

// Bindings.

godot_4d_bind::Basis4D *godot_4d_bind::Basis4D::singleton = nullptr;

void godot_4d_bind::Basis4D::_bind_methods() {
	// Misc methods.
	ClassDB::bind_static_method("Basis4D", D_METHOD("determinant", "basis"), &godot_4d_bind::Basis4D::determinant);
	ClassDB::bind_static_method("Basis4D", D_METHOD("is_equal_approx", "a", "b"), &godot_4d_bind::Basis4D::is_equal_approx);
	// Interpolation.
	ClassDB::bind_static_method("Basis4D", D_METHOD("lerp", "from", "to", "weight"), &godot_4d_bind::Basis4D::lerp);
	ClassDB::bind_static_method("Basis4D", D_METHOD("slerp", "from", "to", "weight"), &godot_4d_bind::Basis4D::slerp);
	ClassDB::bind_static_method("Basis4D", D_METHOD("slerp_rotation", "from", "to", "weight"), &godot_4d_bind::Basis4D::slerp_rotation);
	// Transformation methods.
	ClassDB::bind_static_method("Basis4D", D_METHOD("compose", "parent", "child"), &godot_4d_bind::Basis4D::compose);
	ClassDB::bind_static_method("Basis4D", D_METHOD("transform_to", "from", "to"), &godot_4d_bind::Basis4D::transform_to);
	ClassDB::bind_static_method("Basis4D", D_METHOD("xform", "basis", "vector"), &godot_4d_bind::Basis4D::xform);
	ClassDB::bind_static_method("Basis4D", D_METHOD("xform_inv", "basis", "vector"), &godot_4d_bind::Basis4D::xform_inv);
	ClassDB::bind_static_method("Basis4D", D_METHOD("xform_transposed", "basis", "vector"), &godot_4d_bind::Basis4D::xform_transposed);
	// Rotation methods.
	ClassDB::bind_static_method("Basis4D", D_METHOD("rotate_bivector", "basis", "bivector"), &godot_4d_bind::Basis4D::rotate_bivector);
	ClassDB::bind_static_method("Basis4D", D_METHOD("rotate_plane_global", "basis", "plane_from", "plane_to", "angle"), &godot_4d_bind::Basis4D::rotate_plane_global);
	ClassDB::bind_static_method("Basis4D", D_METHOD("rotate_plane_local", "basis", "local_plane_from", "local_plane_to", "angle"), &godot_4d_bind::Basis4D::rotate_plane_local);
	// Inversion methods.
	ClassDB::bind_static_method("Basis4D", D_METHOD("inverse", "basis"), &godot_4d_bind::Basis4D::inverse);
	ClassDB::bind_static_method("Basis4D", D_METHOD("transposed", "basis"), &godot_4d_bind::Basis4D::transposed);
	// Scale methods.
	ClassDB::bind_static_method("Basis4D", D_METHOD("scaled_global", "basis", "scale"), &godot_4d_bind::Basis4D::scaled_global);
	ClassDB::bind_static_method("Basis4D", D_METHOD("scaled_local", "basis", "scale"), &godot_4d_bind::Basis4D::scaled_local);
	ClassDB::bind_static_method("Basis4D", D_METHOD("scaled_uniform", "basis", "scale"), &godot_4d_bind::Basis4D::scaled_uniform);
	ClassDB::bind_static_method("Basis4D", D_METHOD("get_scale", "basis"), &godot_4d_bind::Basis4D::get_scale);
	ClassDB::bind_static_method("Basis4D", D_METHOD("get_scale_abs", "basis"), &godot_4d_bind::Basis4D::get_scale_abs);
	ClassDB::bind_static_method("Basis4D", D_METHOD("get_uniform_scale", "basis"), &godot_4d_bind::Basis4D::get_uniform_scale);
	// Validation methods.
	ClassDB::bind_static_method("Basis4D", D_METHOD("orthonormalized", "basis"), &godot_4d_bind::Basis4D::orthonormalized);
	ClassDB::bind_static_method("Basis4D", D_METHOD("orthogonalized", "basis"), &godot_4d_bind::Basis4D::orthogonalized);
	ClassDB::bind_static_method("Basis4D", D_METHOD("is_orthogonal", "basis"), &godot_4d_bind::Basis4D::is_orthogonal);
	ClassDB::bind_static_method("Basis4D", D_METHOD("is_orthonormal", "basis"), &godot_4d_bind::Basis4D::is_orthonormal);
	ClassDB::bind_static_method("Basis4D", D_METHOD("is_conformal", "basis"), &godot_4d_bind::Basis4D::is_conformal);
	ClassDB::bind_static_method("Basis4D", D_METHOD("is_diagonal", "basis"), &godot_4d_bind::Basis4D::is_diagonal);
	ClassDB::bind_static_method("Basis4D", D_METHOD("is_rotation", "basis"), &godot_4d_bind::Basis4D::is_rotation);
	// Helper setters/getters.
	ClassDB::bind_static_method("Basis4D", D_METHOD("get_column", "basis", "index"), &godot_4d_bind::Basis4D::get_column);
	ClassDB::bind_static_method("Basis4D", D_METHOD("get_row", "basis", "index"), &godot_4d_bind::Basis4D::get_row);
	ClassDB::bind_static_method("Basis4D", D_METHOD("get_main_diagonal", "basis"), &godot_4d_bind::Basis4D::get_main_diagonal);
	// Helper Euler4D and Rotor4D functions.
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_euler", "euler"), &godot_4d_bind::Basis4D::from_euler);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_euler_aabb", "euler"), &godot_4d_bind::Basis4D::from_euler_aabb);
	ClassDB::bind_static_method("Basis4D", D_METHOD("to_euler", "basis", "use_special_cases"), &godot_4d_bind::Basis4D::to_euler, DEFVAL(true));
	ClassDB::bind_static_method("Basis4D", D_METHOD("to_euler_aabb", "basis", "use_special_cases"), &godot_4d_bind::Basis4D::to_euler_aabb, DEFVAL(true));
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_rotor", "rotor"), &godot_4d_bind::Basis4D::from_rotor);
	ClassDB::bind_static_method("Basis4D", D_METHOD("to_rotor", "basis"), &godot_4d_bind::Basis4D::to_rotor);
	// Conversion.
	ClassDB::bind_static_method("Basis4D", D_METHOD("to_3d", "from_4d", "orthonormalized"), &godot_4d_bind::Basis4D::to_3d, DEFVAL(false));
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_3d", "from_3d"), &godot_4d_bind::Basis4D::from_3d);
	ClassDB::bind_static_method("Basis4D", D_METHOD("to_array", "from_basis"), &godot_4d_bind::Basis4D::to_array);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_array", "from_array"), &godot_4d_bind::Basis4D::from_array);
	// Constructors.
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_scale", "scale"), &godot_4d_bind::Basis4D::from_scale);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_scale_uniform", "scale"), &godot_4d_bind::Basis4D::from_scale_uniform);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_yz", "yz"), &godot_4d_bind::Basis4D::from_yz);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_zx", "zx"), &godot_4d_bind::Basis4D::from_zx);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_xy", "xy"), &godot_4d_bind::Basis4D::from_xy);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_xw", "xw"), &godot_4d_bind::Basis4D::from_xw);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_wy", "wy"), &godot_4d_bind::Basis4D::from_wy);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_zw", "zw"), &godot_4d_bind::Basis4D::from_zw);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_swap_rotation", "rot_from", "rot_to"), &godot_4d_bind::Basis4D::from_swap_rotation);
	ClassDB::bind_static_method("Basis4D", D_METHOD("looking_at", "target", "perp", "up", "use_model_front"), &godot_4d_bind::Basis4D::looking_at, DEFVAL(Vector4(0, 0, 0, 1)), DEFVAL(Vector4(0, 1, 0, 0)), DEFVAL(false));
}
