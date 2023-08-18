#include "euler_4d_bind.h"

#include "euler_4d.h"

// Misc methods.

AABB godot_4d_bind::Euler4D::compose(const AABB &p_parent, const AABB &p_child) {
	return ::Euler4D(p_parent).compose(::Euler4D(p_child));
}

bool godot_4d_bind::Euler4D::is_equal_approx(const AABB &p_a, const AABB &p_b) {
	return p_a.is_equal_approx(p_b);
}

AABB godot_4d_bind::Euler4D::rotation_to(const AABB &p_from, const AABB &p_to) {
	return ::Euler4D(p_from).rotation_to(::Euler4D(p_to));
}

AABB godot_4d_bind::Euler4D::wrapped(const AABB &p_euler_4d) {
	return ::Euler4D(p_euler_4d).wrapped();
}

// Radians/degrees.

AABB godot_4d_bind::Euler4D::deg_to_rad(const AABB &p_euler_4d) {
	return ::Euler4D(p_euler_4d).deg_to_rad();
}

AABB godot_4d_bind::Euler4D::rad_to_deg(const AABB &p_euler_4d) {
	return ::Euler4D(p_euler_4d).rad_to_deg();
}

// Individual component getters.

real_t godot_4d_bind::Euler4D::get_yz(const AABB &p_euler_4d) {
	return p_euler_4d.position.x;
}

real_t godot_4d_bind::Euler4D::get_zx(const AABB &p_euler_4d) {
	return p_euler_4d.position.y;
}

real_t godot_4d_bind::Euler4D::get_xy(const AABB &p_euler_4d) {
	return p_euler_4d.position.z;
}

real_t godot_4d_bind::Euler4D::get_xw(const AABB &p_euler_4d) {
	return p_euler_4d.size.x;
}

real_t godot_4d_bind::Euler4D::get_wy(const AABB &p_euler_4d) {
	return p_euler_4d.size.y;
}

real_t godot_4d_bind::Euler4D::get_zw(const AABB &p_euler_4d) {
	return p_euler_4d.size.z;
}

// Individual component with-style setters.

AABB godot_4d_bind::Euler4D::with_yz(const AABB &p_euler_4d, const real_t p_yz) {
	return AABB(Vector3(p_yz, p_euler_4d.position.y, p_euler_4d.position.z), p_euler_4d.size);
}

AABB godot_4d_bind::Euler4D::with_zx(const AABB &p_euler_4d, const real_t p_zx) {
	return AABB(Vector3(p_euler_4d.position.x, p_zx, p_euler_4d.position.z), p_euler_4d.size);
}

AABB godot_4d_bind::Euler4D::with_xy(const AABB &p_euler_4d, const real_t p_xy) {
	return AABB(Vector3(p_euler_4d.position.x, p_euler_4d.position.y, p_xy), p_euler_4d.size);
}

AABB godot_4d_bind::Euler4D::with_xw(const AABB &p_euler_4d, const real_t p_xw) {
	return AABB(p_euler_4d.position, Vector3(p_xw, p_euler_4d.size.y, p_euler_4d.size.z));
}

AABB godot_4d_bind::Euler4D::with_wy(const AABB &p_euler_4d, const real_t p_wy) {
	return AABB(p_euler_4d.position, Vector3(p_euler_4d.size.x, p_wy, p_euler_4d.size.z));
}

AABB godot_4d_bind::Euler4D::with_zw(const AABB &p_euler_4d, const real_t p_zw) {
	return AABB(p_euler_4d.position, Vector3(p_euler_4d.size.x, p_euler_4d.size.y, p_zw));
}

// Conversion.

AABB godot_4d_bind::Euler4D::from_3d(const Vector3 &p_from_euler_3d) {
	return AABB(p_from_euler_3d, Vector3());
}

Vector3 godot_4d_bind::Euler4D::to_3d(const AABB &p_from_euler_4d) {
	return p_from_euler_4d.position;
}

AABB godot_4d_bind::Euler4D::from_array(const PackedRealArray &p_from_array) {
	return ::Euler4D::from_array(p_from_array);
}

PackedRealArray godot_4d_bind::Euler4D::to_array(const AABB &p_from_euler_4d) {
	return ::Euler4D(p_from_euler_4d).to_array();
}

AABB godot_4d_bind::Euler4D::from_basis(const Projection &p_from_basis_4d, const bool p_use_special_cases) {
	return ::Euler4D::from_basis(Basis4D(p_from_basis_4d), p_use_special_cases);
}

Projection godot_4d_bind::Euler4D::to_basis(const AABB &p_from_euler_4d) {
	return ::Euler4D(p_from_euler_4d).to_basis();
}

// Constructors.

AABB godot_4d_bind::Euler4D::from_numbers(const real_t p_yz, const real_t p_zx, const real_t p_xy, const real_t p_xw, const real_t p_wy, const real_t p_zw) {
	return AABB(Vector3(p_yz, p_zx, p_xy), Vector3(p_xw, p_wy, p_zw));
}

AABB godot_4d_bind::Euler4D::from_vectors(const Vector3 &p_yz_zx_xy, const Vector3 &p_xw_wy_zw) {
	return AABB(p_yz_zx_xy, p_xw_wy_zw);
}

// Bindings.

godot_4d_bind::Euler4D *godot_4d_bind::Euler4D::singleton = nullptr;

void godot_4d_bind::Euler4D::_bind_methods() {
	// Misc methods.
	ClassDB::bind_static_method("Euler4D", D_METHOD("compose", "parent", "child"), &godot_4d_bind::Euler4D::compose);
	ClassDB::bind_static_method("Euler4D", D_METHOD("is_equal_approx", "a", "b"), &godot_4d_bind::Euler4D::is_equal_approx);
	ClassDB::bind_static_method("Euler4D", D_METHOD("rotation_to", "from", "to"), &godot_4d_bind::Euler4D::rotation_to);
	ClassDB::bind_static_method("Euler4D", D_METHOD("wrapped", "euler"), &godot_4d_bind::Euler4D::wrapped);
	// Radians/degrees.
	ClassDB::bind_static_method("Euler4D", D_METHOD("deg_to_rad", "euler"), &godot_4d_bind::Euler4D::deg_to_rad);
	ClassDB::bind_static_method("Euler4D", D_METHOD("rad_to_deg", "euler"), &godot_4d_bind::Euler4D::rad_to_deg);
	// Individual component getters.
	ClassDB::bind_static_method("Euler4D", D_METHOD("get_yz", "euler"), &godot_4d_bind::Euler4D::get_yz);
	ClassDB::bind_static_method("Euler4D", D_METHOD("get_zx", "euler"), &godot_4d_bind::Euler4D::get_zx);
	ClassDB::bind_static_method("Euler4D", D_METHOD("get_xy", "euler"), &godot_4d_bind::Euler4D::get_xy);
	ClassDB::bind_static_method("Euler4D", D_METHOD("get_xw", "euler"), &godot_4d_bind::Euler4D::get_xw);
	ClassDB::bind_static_method("Euler4D", D_METHOD("get_wy", "euler"), &godot_4d_bind::Euler4D::get_wy);
	ClassDB::bind_static_method("Euler4D", D_METHOD("get_zw", "euler"), &godot_4d_bind::Euler4D::get_zw);
	// Individual component with-style setters.
	ClassDB::bind_static_method("Euler4D", D_METHOD("with_yz", "euler", "yz"), &godot_4d_bind::Euler4D::with_yz);
	ClassDB::bind_static_method("Euler4D", D_METHOD("with_zx", "euler", "zx"), &godot_4d_bind::Euler4D::with_zx);
	ClassDB::bind_static_method("Euler4D", D_METHOD("with_xy", "euler", "xy"), &godot_4d_bind::Euler4D::with_xy);
	ClassDB::bind_static_method("Euler4D", D_METHOD("with_xw", "euler", "xw"), &godot_4d_bind::Euler4D::with_xw);
	ClassDB::bind_static_method("Euler4D", D_METHOD("with_wy", "euler", "wy"), &godot_4d_bind::Euler4D::with_wy);
	ClassDB::bind_static_method("Euler4D", D_METHOD("with_zw", "euler", "zw"), &godot_4d_bind::Euler4D::with_zw);
	// Conversion methods.
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_3d", "from_euler_3d"), &godot_4d_bind::Euler4D::from_3d);
	ClassDB::bind_static_method("Euler4D", D_METHOD("to_3d", "from_euler_4d"), &godot_4d_bind::Euler4D::to_3d);
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_array", "from_array"), &godot_4d_bind::Euler4D::from_array);
	ClassDB::bind_static_method("Euler4D", D_METHOD("to_array", "from_euler_4d"), &godot_4d_bind::Euler4D::to_array);
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_basis", "from_basis", "use_special_cases"), &godot_4d_bind::Euler4D::from_basis, DEFVAL(true));
	ClassDB::bind_static_method("Euler4D", D_METHOD("to_basis", "from_euler_4d"), &godot_4d_bind::Euler4D::to_basis);
	// Constructors.
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_numbers", "yz", "zx", "xy", "xw", "wy", "zw"), &godot_4d_bind::Euler4D::from_numbers, DEFVAL(0.0), DEFVAL(0.0), DEFVAL(0.0));
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_vectors", "yz_zx_xy", "xw_wy_zw"), &godot_4d_bind::Euler4D::from_vectors, DEFVAL(Vector3()));
}
