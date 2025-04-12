#include "euler_4d_bind.h"

// Beware, this file contains a massive amount of boring repetitive boilerplate code.

#include "transform_4d_bind.h"

// Instance methods, providing a friendly readable Euler4D object, which unfortunately uses lots of memory and is passed by reference.

#define EULER4D_BIND_RETURN_REF(m_value)    \
	Ref<godot_4d_bind::Euler4D> euler_bind; \
	euler_bind.instantiate();               \
	euler_bind->set_euler(m_value);         \
	return euler_bind;

// Misc methods.

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::add(const Ref<godot_4d_bind::Euler4D> &p_other) const {
	EULER4D_BIND_RETURN_REF(euler + p_other->get_euler());
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::compose(const Ref<godot_4d_bind::Euler4D> &p_child) const {
	EULER4D_BIND_RETURN_REF(euler.compose(p_child->get_euler()));
}

bool godot_4d_bind::Euler4D::is_equal_approx(const Ref<godot_4d_bind::Euler4D> &p_other) const {
	return euler.is_equal_approx(p_other->euler);
}

bool godot_4d_bind::Euler4D::is_equal_exact(const Ref<godot_4d_bind::Euler4D> &p_other) const {
	return euler == p_other->euler;
}

Projection godot_4d_bind::Euler4D::rotate_basis(const Projection &p_basis) const {
	return euler.rotate_basis(p_basis);
}

Vector4 godot_4d_bind::Euler4D::rotate_point(const Vector4 &p_point) const {
	return euler.rotate_point(p_point);
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::rotation_to(const Ref<godot_4d_bind::Euler4D> &p_to) const {
	EULER4D_BIND_RETURN_REF(euler.rotation_to(p_to->get_euler()));
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::wrapped() const {
	EULER4D_BIND_RETURN_REF(euler.wrapped());
}

// Radians/degrees.

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::deg_to_rad() const {
	EULER4D_BIND_RETURN_REF(euler.deg_to_rad());
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::rad_to_deg() const {
	EULER4D_BIND_RETURN_REF(euler.rad_to_deg());
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::with_yz(const real_t p_yz) const {
	EULER4D_BIND_RETURN_REF(euler.with_yz(p_yz));
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::with_zx(const real_t p_zx) const {
	EULER4D_BIND_RETURN_REF(euler.with_zx(p_zx));
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::with_xy(const real_t p_xy) const {
	EULER4D_BIND_RETURN_REF(euler.with_xy(p_xy));
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::with_xw(const real_t p_xw) const {
	EULER4D_BIND_RETURN_REF(euler.with_xw(p_xw));
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::with_wy(const real_t p_wy) const {
	EULER4D_BIND_RETURN_REF(euler.with_wy(p_wy));
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::with_zw(const real_t p_zw) const {
	EULER4D_BIND_RETURN_REF(euler.with_zw(p_zw));
}

// Conversion.

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::from_3d(const Vector3 &p_from_euler_3d) {
	EULER4D_BIND_RETURN_REF(::Euler4D::from_3d(p_from_euler_3d));
}

Vector3 godot_4d_bind::Euler4D::to_3d() const {
	return euler.to_3d();
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::from_array(const PackedRealArray &p_from_array) {
	EULER4D_BIND_RETURN_REF(::Euler4D::from_array(p_from_array));
}

PackedRealArray godot_4d_bind::Euler4D::to_array() const {
	return euler.to_array();
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::from_basis(const Projection &p_basis, const bool p_use_special_cases) {
	EULER4D_BIND_RETURN_REF(::Euler4D::from_basis(p_basis, p_use_special_cases));
}

Projection godot_4d_bind::Euler4D::to_basis() const {
	return euler.to_basis();
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::from_transform(const Ref<godot_4d_bind::Transform4D> &p_transform, const bool p_use_special_cases) {
	EULER4D_BIND_RETURN_REF(::Euler4D::from_basis(p_transform->get_basis(), p_use_special_cases));
}

Ref<godot_4d_bind::Transform4D> godot_4d_bind::Euler4D::to_transform() const {
	Ref<godot_4d_bind::Transform4D> transform_bind;
	transform_bind.instantiate();
	transform_bind->set_basis(euler.to_basis());
	return transform_bind;
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::from_numbers(const real_t p_yz, const real_t p_zx, const real_t p_xy, const real_t p_xw, const real_t p_wy, const real_t p_zw) {
	EULER4D_BIND_RETURN_REF(::Euler4D(p_yz, p_zx, p_xy, p_xw, p_wy, p_zw));
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::from_vectors(const Vector3 &p_yz_zx_xy, const Vector3 &p_xw_wy_zw) {
	EULER4D_BIND_RETURN_REF(::Euler4D(p_yz_zx_xy, p_xw_wy_zw));
}

Ref<godot_4d_bind::Euler4D> godot_4d_bind::Euler4D::copy() const {
	EULER4D_BIND_RETURN_REF(euler);
}

#if GDEXTENSION
String godot_4d_bind::Euler4D::_to_string() const
#elif GODOT_MODULE
String godot_4d_bind::Euler4D::to_string()
#endif
{
	return euler.operator String();
}

// Static methods, allowing the user to use AABB to represent an Euler4D, which saves memory and is passed by value.

// Misc methods.

AABB godot_4d_bind::Euler4D::aabb_compose(const AABB &p_parent, const AABB &p_child) {
	return ::Euler4D(p_parent).compose(::Euler4D(p_child));
}

bool godot_4d_bind::Euler4D::aabb_is_equal_approx(const AABB &p_a, const AABB &p_b) {
	return p_a.is_equal_approx(p_b);
}

Projection godot_4d_bind::Euler4D::aabb_rotate_basis(const AABB &p_euler_4d, const Projection &p_basis) {
	return ::Euler4D(p_euler_4d).rotate_basis(p_basis);
}

Vector4 godot_4d_bind::Euler4D::aabb_rotate_point(const AABB &p_euler_4d, const Vector4 &p_point) {
	return ::Euler4D(p_euler_4d).rotate_point(p_point);
}

AABB godot_4d_bind::Euler4D::aabb_rotation_to(const AABB &p_from, const AABB &p_to) {
	return ::Euler4D(p_from).rotation_to(::Euler4D(p_to));
}

AABB godot_4d_bind::Euler4D::aabb_wrapped(const AABB &p_euler_4d) {
	return ::Euler4D(p_euler_4d).wrapped();
}

// Radians/degrees.

AABB godot_4d_bind::Euler4D::aabb_deg_to_rad(const AABB &p_euler_4d) {
	return ::Euler4D(p_euler_4d).deg_to_rad();
}

AABB godot_4d_bind::Euler4D::aabb_rad_to_deg(const AABB &p_euler_4d) {
	return ::Euler4D(p_euler_4d).rad_to_deg();
}

// Individual component getters.

real_t godot_4d_bind::Euler4D::aabb_get_yz(const AABB &p_euler_4d) {
	return p_euler_4d.position.x;
}

real_t godot_4d_bind::Euler4D::aabb_get_zx(const AABB &p_euler_4d) {
	return p_euler_4d.position.y;
}

real_t godot_4d_bind::Euler4D::aabb_get_xy(const AABB &p_euler_4d) {
	return p_euler_4d.position.z;
}

real_t godot_4d_bind::Euler4D::aabb_get_xw(const AABB &p_euler_4d) {
	return p_euler_4d.size.x;
}

real_t godot_4d_bind::Euler4D::aabb_get_wy(const AABB &p_euler_4d) {
	return p_euler_4d.size.y;
}

real_t godot_4d_bind::Euler4D::aabb_get_zw(const AABB &p_euler_4d) {
	return p_euler_4d.size.z;
}

// Individual component with-style setters.

AABB godot_4d_bind::Euler4D::aabb_with_yz(const AABB &p_euler_4d, const real_t p_yz) {
	return AABB(Vector3(p_yz, p_euler_4d.position.y, p_euler_4d.position.z), p_euler_4d.size);
}

AABB godot_4d_bind::Euler4D::aabb_with_zx(const AABB &p_euler_4d, const real_t p_zx) {
	return AABB(Vector3(p_euler_4d.position.x, p_zx, p_euler_4d.position.z), p_euler_4d.size);
}

AABB godot_4d_bind::Euler4D::aabb_with_xy(const AABB &p_euler_4d, const real_t p_xy) {
	return AABB(Vector3(p_euler_4d.position.x, p_euler_4d.position.y, p_xy), p_euler_4d.size);
}

AABB godot_4d_bind::Euler4D::aabb_with_xw(const AABB &p_euler_4d, const real_t p_xw) {
	return AABB(p_euler_4d.position, Vector3(p_xw, p_euler_4d.size.y, p_euler_4d.size.z));
}

AABB godot_4d_bind::Euler4D::aabb_with_wy(const AABB &p_euler_4d, const real_t p_wy) {
	return AABB(p_euler_4d.position, Vector3(p_euler_4d.size.x, p_wy, p_euler_4d.size.z));
}

AABB godot_4d_bind::Euler4D::aabb_with_zw(const AABB &p_euler_4d, const real_t p_zw) {
	return AABB(p_euler_4d.position, Vector3(p_euler_4d.size.x, p_euler_4d.size.y, p_zw));
}

// Conversion.

AABB godot_4d_bind::Euler4D::aabb_from_3d(const Vector3 &p_from_euler_3d) {
	return AABB(p_from_euler_3d, Vector3());
}

Vector3 godot_4d_bind::Euler4D::aabb_to_3d(const AABB &p_from_euler_4d) {
	return p_from_euler_4d.position;
}

AABB godot_4d_bind::Euler4D::aabb_from_array(const PackedRealArray &p_from_array) {
	return ::Euler4D::from_array(p_from_array);
}

PackedRealArray godot_4d_bind::Euler4D::aabb_to_array(const AABB &p_from_euler_4d) {
	return ::Euler4D(p_from_euler_4d).to_array();
}

AABB godot_4d_bind::Euler4D::aabb_from_basis(const Projection &p_from_basis_4d, const bool p_use_special_cases) {
	return ::Euler4D::from_basis(Basis4D(p_from_basis_4d), p_use_special_cases);
}

Projection godot_4d_bind::Euler4D::aabb_to_basis(const AABB &p_from_euler_4d) {
	return ::Euler4D(p_from_euler_4d).to_basis();
}

// Constructors.

AABB godot_4d_bind::Euler4D::aabb_from_numbers(const real_t p_yz, const real_t p_zx, const real_t p_xy, const real_t p_xw, const real_t p_wy, const real_t p_zw) {
	return AABB(Vector3(p_yz, p_zx, p_xy), Vector3(p_xw, p_wy, p_zw));
}

AABB godot_4d_bind::Euler4D::aabb_from_vectors(const Vector3 &p_yz_zx_xy, const Vector3 &p_xw_wy_zw) {
	return AABB(p_yz_zx_xy, p_xw_wy_zw);
}

// Bindings.

void godot_4d_bind::Euler4D::_bind_methods() {
	// Instance method bindings.
	// Misc methods.
	ClassDB::bind_method(D_METHOD("add", "other"), &godot_4d_bind::Euler4D::add);
	ClassDB::bind_method(D_METHOD("compose", "child"), &godot_4d_bind::Euler4D::compose);
	ClassDB::bind_method(D_METHOD("is_equal_approx", "other"), &godot_4d_bind::Euler4D::is_equal_approx);
	ClassDB::bind_method(D_METHOD("is_equal_exact", "other"), &godot_4d_bind::Euler4D::is_equal_exact);
	ClassDB::bind_method(D_METHOD("rotate_basis", "basis"), &godot_4d_bind::Euler4D::rotate_basis);
	ClassDB::bind_method(D_METHOD("rotate_point", "point"), &godot_4d_bind::Euler4D::rotate_point);
	ClassDB::bind_method(D_METHOD("rotation_to", "to"), &godot_4d_bind::Euler4D::rotation_to);
	ClassDB::bind_method(D_METHOD("wrapped"), &godot_4d_bind::Euler4D::wrapped);
	// Radians/degrees.
	ClassDB::bind_method(D_METHOD("deg_to_rad"), &godot_4d_bind::Euler4D::deg_to_rad);
	ClassDB::bind_method(D_METHOD("rad_to_deg"), &godot_4d_bind::Euler4D::rad_to_deg);
	// Individual component getters/setters.
	ClassDB::bind_method(D_METHOD("get_yz"), &godot_4d_bind::Euler4D::get_yz);
	ClassDB::bind_method(D_METHOD("get_zx"), &godot_4d_bind::Euler4D::get_zx);
	ClassDB::bind_method(D_METHOD("get_xy"), &godot_4d_bind::Euler4D::get_xy);
	ClassDB::bind_method(D_METHOD("get_xw"), &godot_4d_bind::Euler4D::get_xw);
	ClassDB::bind_method(D_METHOD("get_wy"), &godot_4d_bind::Euler4D::get_wy);
	ClassDB::bind_method(D_METHOD("get_zw"), &godot_4d_bind::Euler4D::get_zw);
	ClassDB::bind_method(D_METHOD("set_yz", "yz"), &godot_4d_bind::Euler4D::set_yz);
	ClassDB::bind_method(D_METHOD("set_zx", "zx"), &godot_4d_bind::Euler4D::set_zx);
	ClassDB::bind_method(D_METHOD("set_xy", "xy"), &godot_4d_bind::Euler4D::set_xy);
	ClassDB::bind_method(D_METHOD("set_xw", "xw"), &godot_4d_bind::Euler4D::set_xw);
	ClassDB::bind_method(D_METHOD("set_wy", "wy"), &godot_4d_bind::Euler4D::set_wy);
	ClassDB::bind_method(D_METHOD("set_zw", "zw"), &godot_4d_bind::Euler4D::set_zw);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "yz"), "set_yz", "get_yz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "zx"), "set_zx", "get_zx");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "xy"), "set_xy", "get_xy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "xw"), "set_xw", "get_xw");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "wy"), "set_wy", "get_wy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "zw"), "set_zw", "get_zw");
	// Individual component with-style setters.
	ClassDB::bind_method(D_METHOD("with_yz", "yz"), &godot_4d_bind::Euler4D::with_yz);
	ClassDB::bind_method(D_METHOD("with_zx", "zx"), &godot_4d_bind::Euler4D::with_zx);
	ClassDB::bind_method(D_METHOD("with_xy", "xy"), &godot_4d_bind::Euler4D::with_xy);
	ClassDB::bind_method(D_METHOD("with_xw", "xw"), &godot_4d_bind::Euler4D::with_xw);
	ClassDB::bind_method(D_METHOD("with_wy", "wy"), &godot_4d_bind::Euler4D::with_wy);
	ClassDB::bind_method(D_METHOD("with_zw", "zw"), &godot_4d_bind::Euler4D::with_zw);
	// Conversion.
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_3d", "from_euler_3d"), &godot_4d_bind::Euler4D::from_3d);
	ClassDB::bind_method(D_METHOD("to_3d"), &godot_4d_bind::Euler4D::to_3d);
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_array", "from_array"), &godot_4d_bind::Euler4D::from_array);
	ClassDB::bind_method(D_METHOD("to_array"), &godot_4d_bind::Euler4D::to_array);
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_basis", "from_basis", "use_special_cases"), &godot_4d_bind::Euler4D::from_basis, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("to_basis"), &godot_4d_bind::Euler4D::to_basis);
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_transform", "transform", "use_special_cases"), &godot_4d_bind::Euler4D::from_transform, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("to_transform"), &godot_4d_bind::Euler4D::to_transform);
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_numbers", "yz", "zx", "xy", "xw", "wy", "zw"), &godot_4d_bind::Euler4D::from_numbers, DEFVAL(0), DEFVAL(0), DEFVAL(0));
	ClassDB::bind_static_method("Euler4D", D_METHOD("from_vectors", "yz_zx_xy", "xw_wy_zw"), &godot_4d_bind::Euler4D::from_vectors, DEFVAL(Vector3()));
	ClassDB::bind_method(D_METHOD("copy"), &godot_4d_bind::Euler4D::copy);

	// Static method bindings.
	// Misc methods.
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_compose", "parent", "child"), &godot_4d_bind::Euler4D::aabb_compose);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_is_equal_approx", "a", "b"), &godot_4d_bind::Euler4D::aabb_is_equal_approx);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_rotate_basis", "euler_4d", "basis"), &godot_4d_bind::Euler4D::aabb_rotate_basis);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_rotate_point", "euler_4d", "point"), &godot_4d_bind::Euler4D::aabb_rotate_point);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_rotation_to", "from", "to"), &godot_4d_bind::Euler4D::aabb_rotation_to);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_wrapped", "euler"), &godot_4d_bind::Euler4D::aabb_wrapped);
	// Radians/degrees.
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_deg_to_rad", "euler"), &godot_4d_bind::Euler4D::aabb_deg_to_rad);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_rad_to_deg", "euler"), &godot_4d_bind::Euler4D::aabb_rad_to_deg);
	// Individual component getters.
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_get_yz", "euler"), &godot_4d_bind::Euler4D::aabb_get_yz);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_get_zx", "euler"), &godot_4d_bind::Euler4D::aabb_get_zx);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_get_xy", "euler"), &godot_4d_bind::Euler4D::aabb_get_xy);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_get_xw", "euler"), &godot_4d_bind::Euler4D::aabb_get_xw);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_get_wy", "euler"), &godot_4d_bind::Euler4D::aabb_get_wy);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_get_zw", "euler"), &godot_4d_bind::Euler4D::aabb_get_zw);
	// Individual component with-style setters.
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_with_yz", "euler", "yz"), &godot_4d_bind::Euler4D::aabb_with_yz);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_with_zx", "euler", "zx"), &godot_4d_bind::Euler4D::aabb_with_zx);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_with_xy", "euler", "xy"), &godot_4d_bind::Euler4D::aabb_with_xy);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_with_xw", "euler", "xw"), &godot_4d_bind::Euler4D::aabb_with_xw);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_with_wy", "euler", "wy"), &godot_4d_bind::Euler4D::aabb_with_wy);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_with_zw", "euler", "zw"), &godot_4d_bind::Euler4D::aabb_with_zw);
	// Conversion methods.
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_from_3d", "from_euler_3d"), &godot_4d_bind::Euler4D::aabb_from_3d);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_to_3d", "from_euler_4d"), &godot_4d_bind::Euler4D::aabb_to_3d);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_from_array", "from_array"), &godot_4d_bind::Euler4D::aabb_from_array);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_to_array", "from_euler_4d"), &godot_4d_bind::Euler4D::aabb_to_array);
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_from_basis", "from_basis", "use_special_cases"), &godot_4d_bind::Euler4D::aabb_from_basis, DEFVAL(true));
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_to_basis", "from_euler_4d"), &godot_4d_bind::Euler4D::aabb_to_basis);
	// Constructors.
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_from_numbers", "yz", "zx", "xy", "xw", "wy", "zw"), &godot_4d_bind::Euler4D::aabb_from_numbers, DEFVAL(0.0), DEFVAL(0.0), DEFVAL(0.0));
	ClassDB::bind_static_method("Euler4D", D_METHOD("aabb_from_vectors", "yz_zx_xy", "xw_wy_zw"), &godot_4d_bind::Euler4D::aabb_from_vectors, DEFVAL(Vector3()));
}
