#include "rotor_4d_bind.h"

// Beware, this file contains a massive amount of boring repetitive boilerplate code.

#define ROTOR4D_BIND_RETURN_REF(m_value)    \
	Ref<godot_4d_bind::Rotor4D> rotor_bind; \
	rotor_bind.instantiate();               \
	rotor_bind->set_rotor(m_value);         \
	return rotor_bind;

// Geometric algebra functions.

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::conjugate() const {
	ROTOR4D_BIND_RETURN_REF(rotor.conjugate());
}

real_t godot_4d_bind::Rotor4D::dot(const Ref<godot_4d_bind::Rotor4D> &p_b) const {
	return rotor.dot(p_b->get_rotor());
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::dual() const {
	ROTOR4D_BIND_RETURN_REF(rotor.dual());
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::inner_product(const Ref<godot_4d_bind::Rotor4D> &p_b) const {
	ROTOR4D_BIND_RETURN_REF(rotor.inner_product(p_b->get_rotor()));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::inverse() const {
	ROTOR4D_BIND_RETURN_REF(rotor.inverse());
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::involute() const {
	ROTOR4D_BIND_RETURN_REF(rotor.involute());
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::regressive_product(const Ref<godot_4d_bind::Rotor4D> &p_b) const {
	ROTOR4D_BIND_RETURN_REF(rotor.regressive_product(p_b->get_rotor()));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::reverse() const {
	ROTOR4D_BIND_RETURN_REF(rotor.reverse());
}

real_t godot_4d_bind::Rotor4D::scalar_product(const Ref<godot_4d_bind::Rotor4D> &p_b) const {
	return rotor.scalar_product(p_b->get_rotor());
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::wedge_product(const Ref<godot_4d_bind::Rotor4D> &p_b) const {
	ROTOR4D_BIND_RETURN_REF(rotor.wedge_product(p_b->get_rotor()));
}

// Rotation functions.

Projection godot_4d_bind::Rotor4D::to_basis() const {
	return rotor.to_basis();
}

// real_t godot_4d_bind::Rotor4D::get_rotation_angle() const {
// 	return rotor.get_rotation_angle();
// }

// AABB godot_4d_bind::Rotor4D::get_rotation_bivector_magnitude() const {
// 	return rotor.get_rotation_bivector_magnitude();
// }

// AABB godot_4d_bind::Rotor4D::get_rotation_bivector_normal() const {
// 	return rotor.get_rotation_bivector_normal();
// }

Projection godot_4d_bind::Rotor4D::rotate_basis(const Projection &p_basis) const {
	return rotor.rotate_basis(p_basis);
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::rotate_rotor(const Ref<godot_4d_bind::Rotor4D> &p_rotor) const {
	ROTOR4D_BIND_RETURN_REF(rotor * p_rotor->get_rotor());
}

Vector4 godot_4d_bind::Rotor4D::rotate_vector(const Vector4 &p_vec) const {
	return rotor.rotate_vector(p_vec);
}

Vector4 godot_4d_bind::Rotor4D::sandwich(const Vector4 &p_vec, const Ref<godot_4d_bind::Rotor4D> &p_right) const {
	return rotor.sandwich(p_vec, p_right->get_rotor());
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::slerp(const Ref<godot_4d_bind::Rotor4D> &p_to, const real_t p_weight) const {
	ROTOR4D_BIND_RETURN_REF(rotor.slerp(p_to->get_rotor(), p_weight));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::slerp_fraction(const real_t p_weight) const {
	ROTOR4D_BIND_RETURN_REF(rotor.slerp_fraction(p_weight));
}

// Length functions.

real_t godot_4d_bind::Rotor4D::length() const {
	return rotor.length();
}

real_t godot_4d_bind::Rotor4D::length_squared() const {
	return rotor.length_squared();
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::normalized() const {
	ROTOR4D_BIND_RETURN_REF(rotor.normalized());
}

bool godot_4d_bind::Rotor4D::is_normalized() const {
	return rotor.is_normalized();
}

// Static functions for doing math on non-Rotor4D types and returning a Rotor4D.

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::vector_product(const Vector4 &p_a, const Vector4 &p_b) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::vector_product(p_a, p_b));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_basis(const Projection &p_basis) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_basis(p_basis));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_bivector_magnitude(const AABB &p_bivector) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_bivector_magnitude(p_bivector));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_bivector_normal_angle(const AABB &p_bivector_normal, const real_t p_angle) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_bivector_normal_angle(p_bivector_normal, p_angle));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_xy(const real_t p_angle) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_xy(p_angle));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_xz(const real_t p_angle) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_xz(p_angle));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_zx(const real_t p_angle) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_zx(p_angle));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_xw(const real_t p_angle) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_xw(p_angle));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_yz(const real_t p_angle) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_yz(p_angle));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_yw(const real_t p_angle) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_yw(p_angle));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_wy(const real_t p_angle) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_wy(p_angle));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_zw(const real_t p_angle) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_zw(p_angle));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::identity() {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::identity());
}

// Individual component with-style setters.

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::with_s(const real_t p_s) const {
	ROTOR4D_BIND_RETURN_REF(rotor.with_s(p_s));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::with_xy(const real_t p_xy) const {
	ROTOR4D_BIND_RETURN_REF(rotor.with_xy(p_xy));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::with_xz(const real_t p_xz) const {
	ROTOR4D_BIND_RETURN_REF(rotor.with_xz(p_xz));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::with_xw(const real_t p_xw) const {
	ROTOR4D_BIND_RETURN_REF(rotor.with_xw(p_xw));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::with_yz(const real_t p_yz) const {
	ROTOR4D_BIND_RETURN_REF(rotor.with_yz(p_yz));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::with_yw(const real_t p_yw) const {
	ROTOR4D_BIND_RETURN_REF(rotor.with_yw(p_yw));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::with_zw(const real_t p_zw) const {
	ROTOR4D_BIND_RETURN_REF(rotor.with_zw(p_zw));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::with_xyzw(const real_t p_xyzw) const {
	ROTOR4D_BIND_RETURN_REF(rotor.with_xyzw(p_xyzw));
}

// Conversion.

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_array(const PackedRealArray &p_from_array) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D::from_array(p_from_array));
}

PackedRealArray godot_4d_bind::Rotor4D::to_array() const {
	return rotor.to_array();
}

AABB godot_4d_bind::Rotor4D::get_bivector_aabb() const {
	return rotor.parts.bivector;
}

void godot_4d_bind::Rotor4D::set_bivector_aabb(const AABB &p_bivector_aabb) {
	rotor.parts.bivector = p_bivector_aabb;
}

Vector2 godot_4d_bind::Rotor4D::get_split_complex() const {
	return rotor.get_split_complex();
}

void godot_4d_bind::Rotor4D::set_split_complex(const Vector2 &p_split) {
	rotor.set_split_complex(p_split);
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_numbers(const real_t p_s, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw, const real_t p_xyzw) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D(p_s, p_xy, p_xz, p_xw, p_yz, p_yw, p_zw, p_xyzw));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::from_vectors(const Vector4 &p_s_xy_xz_xw, const Vector4 &p_yz_yw_zw_xyzw) {
	ROTOR4D_BIND_RETURN_REF(::Rotor4D(p_s_xy_xz_xw.x, p_s_xy_xz_xw.y, p_s_xy_xz_xw.z, p_s_xy_xz_xw.w, p_yz_yw_zw_xyzw.x, p_yz_yw_zw_xyzw.y, p_yz_yw_zw_xyzw.z, p_yz_yw_zw_xyzw.w));
}

Ref<godot_4d_bind::Rotor4D> godot_4d_bind::Rotor4D::copy() const {
	ROTOR4D_BIND_RETURN_REF(rotor);
}

#if GDEXTENSION
String godot_4d_bind::Rotor4D::_to_string() const
#elif GODOT_MODULE
String godot_4d_bind::Rotor4D::to_string()
#endif
{
	return rotor.operator String();
}

godot_4d_bind::Rotor4D::Rotor4D(const real_t p_scalar, const Bivector4D &p_bivector, const real_t p_pseudoscalar) {
	rotor = ::Rotor4D(p_scalar, p_bivector, p_pseudoscalar);
}

godot_4d_bind::Rotor4D::Rotor4D(const real_t p_s, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw, const real_t p_xyzw) {
	rotor = ::Rotor4D(p_s, p_xy, p_xz, p_xw, p_yz, p_yw, p_zw, p_xyzw);
}

// Bindings.

void godot_4d_bind::Rotor4D::_bind_methods() {
	// Geometric algebra functions.
	ClassDB::bind_method(D_METHOD("conjugate"), &godot_4d_bind::Rotor4D::conjugate);
	ClassDB::bind_method(D_METHOD("dot", "b"), &godot_4d_bind::Rotor4D::dot);
	ClassDB::bind_method(D_METHOD("dual"), &godot_4d_bind::Rotor4D::dual);
	ClassDB::bind_method(D_METHOD("inner_product", "b"), &godot_4d_bind::Rotor4D::inner_product);
	ClassDB::bind_method(D_METHOD("inverse"), &godot_4d_bind::Rotor4D::inverse);
	ClassDB::bind_method(D_METHOD("involute"), &godot_4d_bind::Rotor4D::involute);
	ClassDB::bind_method(D_METHOD("regressive_product", "b"), &godot_4d_bind::Rotor4D::regressive_product);
	ClassDB::bind_method(D_METHOD("reverse"), &godot_4d_bind::Rotor4D::reverse);
	ClassDB::bind_method(D_METHOD("scalar_product", "b"), &godot_4d_bind::Rotor4D::scalar_product);
	ClassDB::bind_method(D_METHOD("wedge_product", "b"), &godot_4d_bind::Rotor4D::wedge_product);

	// Rotation functions.
	ClassDB::bind_method(D_METHOD("to_basis"), &godot_4d_bind::Rotor4D::to_basis);
	// ClassDB::bind_method(D_METHOD("get_rotation_angle"), &godot_4d_bind::Rotor4D::get_rotation_angle);
	// ClassDB::bind_method(D_METHOD("get_rotation_bivector_magnitude"), &godot_4d_bind::Rotor4D::get_rotation_bivector_magnitude);
	// ClassDB::bind_method(D_METHOD("get_rotation_bivector_normal"), &godot_4d_bind::Rotor4D::get_rotation_bivector_normal);
	ClassDB::bind_method(D_METHOD("rotate_basis", "basis"), &godot_4d_bind::Rotor4D::rotate_basis);
	ClassDB::bind_method(D_METHOD("rotate_rotor", "rotor"), &godot_4d_bind::Rotor4D::rotate_rotor);
	ClassDB::bind_method(D_METHOD("rotate_vector", "vector"), &godot_4d_bind::Rotor4D::rotate_vector);
	ClassDB::bind_method(D_METHOD("sandwich", "vector", "right"), &godot_4d_bind::Rotor4D::sandwich);
	ClassDB::bind_method(D_METHOD("slerp", "to", "weight"), &godot_4d_bind::Rotor4D::slerp);
	ClassDB::bind_method(D_METHOD("slerp_fraction", "weight"), &godot_4d_bind::Rotor4D::slerp_fraction, DEFVAL(0.5f));

	// Length functions.
	ClassDB::bind_method(D_METHOD("length"), &godot_4d_bind::Rotor4D::length);
	ClassDB::bind_method(D_METHOD("length_squared"), &godot_4d_bind::Rotor4D::length_squared);
	ClassDB::bind_method(D_METHOD("normalized"), &godot_4d_bind::Rotor4D::normalized);
	ClassDB::bind_method(D_METHOD("is_normalized"), &godot_4d_bind::Rotor4D::is_normalized);

	// Static functions for doing math on non-Rotor4D types and returning a Rotor4D.
	ClassDB::bind_static_method("Rotor4D", D_METHOD("vector_product", "a", "b"), &godot_4d_bind::Rotor4D::vector_product);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_basis", "basis"), &godot_4d_bind::Rotor4D::from_basis);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_bivector_magnitude", "bivector"), &godot_4d_bind::Rotor4D::from_bivector_magnitude);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_bivector_normal_angle", "bivector_normal", "angle"), &godot_4d_bind::Rotor4D::from_bivector_normal_angle);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_xy", "angle"), &godot_4d_bind::Rotor4D::from_xy);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_xz", "angle"), &godot_4d_bind::Rotor4D::from_xz);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_zx", "angle"), &godot_4d_bind::Rotor4D::from_zx);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_xw", "angle"), &godot_4d_bind::Rotor4D::from_xw);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_yz", "angle"), &godot_4d_bind::Rotor4D::from_yz);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_yw", "angle"), &godot_4d_bind::Rotor4D::from_yw);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_wy", "angle"), &godot_4d_bind::Rotor4D::from_wy);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_zw", "angle"), &godot_4d_bind::Rotor4D::from_zw);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("identity"), &godot_4d_bind::Rotor4D::identity);

	ClassDB::bind_method(D_METHOD("get_bivector"), &godot_4d_bind::Rotor4D::get_bivector_aabb);
	ClassDB::bind_method(D_METHOD("set_bivector", "bivector_aabb"), &godot_4d_bind::Rotor4D::set_bivector_aabb);
	ADD_PROPERTY(PropertyInfo(Variant::AABB, "bivector"), "set_bivector", "get_bivector");

	ClassDB::bind_method(D_METHOD("get_split_complex"), &godot_4d_bind::Rotor4D::get_split_complex);
	ClassDB::bind_method(D_METHOD("set_split_complex", "split"), &godot_4d_bind::Rotor4D::set_split_complex);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "split_complex"), "set_split_complex", "get_split_complex");

	// Individual component with-style setters.
	ClassDB::bind_method(D_METHOD("with_s", "s"), &godot_4d_bind::Rotor4D::with_s);
	ClassDB::bind_method(D_METHOD("with_xy", "xy"), &godot_4d_bind::Rotor4D::with_xy);
	ClassDB::bind_method(D_METHOD("with_xz", "xz"), &godot_4d_bind::Rotor4D::with_xz);
	ClassDB::bind_method(D_METHOD("with_xw", "xw"), &godot_4d_bind::Rotor4D::with_xw);
	ClassDB::bind_method(D_METHOD("with_yz", "yz"), &godot_4d_bind::Rotor4D::with_yz);
	ClassDB::bind_method(D_METHOD("with_yw", "yw"), &godot_4d_bind::Rotor4D::with_yw);
	ClassDB::bind_method(D_METHOD("with_zw", "zw"), &godot_4d_bind::Rotor4D::with_zw);
	ClassDB::bind_method(D_METHOD("with_xyzw", "xyzw"), &godot_4d_bind::Rotor4D::with_xyzw);

	// Individual component getters/setters.
	ClassDB::bind_method(D_METHOD("get_s"), &godot_4d_bind::Rotor4D::get_s);
	ClassDB::bind_method(D_METHOD("get_xy"), &godot_4d_bind::Rotor4D::get_xy);
	ClassDB::bind_method(D_METHOD("get_xz"), &godot_4d_bind::Rotor4D::get_xz);
	ClassDB::bind_method(D_METHOD("get_xw"), &godot_4d_bind::Rotor4D::get_xw);
	ClassDB::bind_method(D_METHOD("get_yz"), &godot_4d_bind::Rotor4D::get_yz);
	ClassDB::bind_method(D_METHOD("get_yw"), &godot_4d_bind::Rotor4D::get_yw);
	ClassDB::bind_method(D_METHOD("get_zw"), &godot_4d_bind::Rotor4D::get_zw);
	ClassDB::bind_method(D_METHOD("get_xyzw"), &godot_4d_bind::Rotor4D::get_xyzw);
	ClassDB::bind_method(D_METHOD("set_s", "s"), &godot_4d_bind::Rotor4D::set_s);
	ClassDB::bind_method(D_METHOD("set_xy", "xy"), &godot_4d_bind::Rotor4D::set_xy);
	ClassDB::bind_method(D_METHOD("set_xz", "xz"), &godot_4d_bind::Rotor4D::set_xz);
	ClassDB::bind_method(D_METHOD("set_xw", "xw"), &godot_4d_bind::Rotor4D::set_xw);
	ClassDB::bind_method(D_METHOD("set_yz", "yz"), &godot_4d_bind::Rotor4D::set_yz);
	ClassDB::bind_method(D_METHOD("set_yw", "yw"), &godot_4d_bind::Rotor4D::set_yw);
	ClassDB::bind_method(D_METHOD("set_zw", "zw"), &godot_4d_bind::Rotor4D::set_zw);
	ClassDB::bind_method(D_METHOD("set_xyzw", "xyzw"), &godot_4d_bind::Rotor4D::set_xyzw);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "s"), "set_s", "get_s");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "xy"), "set_xy", "get_xy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "xz"), "set_xz", "get_xz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "xw"), "set_xw", "get_xw");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "yz"), "set_yz", "get_yz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "yw"), "set_yw", "get_yw");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "zw"), "set_zw", "get_zw");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "xyzw"), "set_xyzw", "get_xyzw");

	// Conversion.
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_array", "from_array"), &godot_4d_bind::Rotor4D::from_array);
	ClassDB::bind_method(D_METHOD("to_array"), &godot_4d_bind::Rotor4D::to_array);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_numbers", "s", "xy", "xz", "xw", "yz", "yw", "zw", "xyzw"), &godot_4d_bind::Rotor4D::from_numbers);
	ClassDB::bind_static_method("Rotor4D", D_METHOD("from_vectors", "s_xy_xz_xw", "yz_yw_zw_xyzw"), &godot_4d_bind::Rotor4D::from_vectors);
	ClassDB::bind_method(D_METHOD("copy"), &godot_4d_bind::Rotor4D::copy);
}
