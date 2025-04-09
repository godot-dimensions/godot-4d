#pragma once

#include "rotor_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#endif

namespace godot_4d_bind {
class Rotor4D : public RefCounted {
	GDCLASS(Rotor4D, RefCounted);

	::Rotor4D rotor;

protected:
	static void _bind_methods();

public:
	// Geometric algebra functions.
	Ref<Rotor4D> conjugate() const;
	real_t dot(const Ref<Rotor4D> &p_b) const;
	Ref<Rotor4D> dual() const;
	Ref<Rotor4D> inner_product(const Ref<Rotor4D> &p_b) const;
	Ref<Rotor4D> inverse() const;
	Ref<Rotor4D> involute() const;
	Ref<Rotor4D> regressive_product(const Ref<Rotor4D> &p_b) const;
	Ref<Rotor4D> reverse() const;
	real_t scalar_product(const Ref<Rotor4D> &p_b) const;
	Ref<Rotor4D> wedge_product(const Ref<Rotor4D> &p_b) const;

	// Rotation functions.
	Projection get_rotation_basis() const;
	real_t get_rotation_angle() const;
	AABB get_rotation_bivector_magnitude() const;
	AABB get_rotation_bivector_normal() const;
	Projection rotate_basis(const Projection &p_basis) const;
	Vector4 rotate_vector(const Vector4 &p_vec) const;
	Vector4 sandwich(const Vector4 &p_vec, const Ref<Rotor4D> &p_right) const;
	Ref<Rotor4D> slerp(const Ref<godot_4d_bind::Rotor4D> &p_to, const real_t p_weight) const;
	Ref<Rotor4D> slerp_fraction(const real_t p_weight = 0.5f) const;

	// Length functions.
	real_t length() const;
	real_t length_squared() const;
	Ref<Rotor4D> normalized() const;
	bool is_normalized() const;

	// Static functions for doing math on non-Rotor4D types and returning a Rotor4D.
	static Ref<Rotor4D> vector_product(const Vector4 &p_a, const Vector4 &p_b);
	static Ref<Rotor4D> from_bivector_magnitude(const AABB &p_bivector);
	static Ref<Rotor4D> from_bivector_normal_angle(const AABB &p_bivector_normal, const real_t p_angle);
	static Ref<Rotor4D> from_xy(const real_t p_angle);
	static Ref<Rotor4D> from_xz(const real_t p_angle);
	static Ref<Rotor4D> from_zx(const real_t p_angle);
	static Ref<Rotor4D> from_xw(const real_t p_angle);
	static Ref<Rotor4D> from_yz(const real_t p_angle);
	static Ref<Rotor4D> from_yw(const real_t p_angle);
	static Ref<Rotor4D> from_wy(const real_t p_angle);
	static Ref<Rotor4D> from_zw(const real_t p_angle);
	static Ref<Rotor4D> identity();

	// Individual component with-style setters.
	Ref<Rotor4D> with_s(const real_t p_s) const;
	Ref<Rotor4D> with_xy(const real_t p_xy) const;
	Ref<Rotor4D> with_xz(const real_t p_zx) const;
	Ref<Rotor4D> with_xw(const real_t p_xw) const;
	Ref<Rotor4D> with_yz(const real_t p_yz) const;
	Ref<Rotor4D> with_yw(const real_t p_yw) const;
	Ref<Rotor4D> with_zw(const real_t p_zw) const;
	Ref<Rotor4D> with_xyzw(const real_t p_xyzw) const;

	// Individual component getters/setters.
	real_t get_s() const { return rotor.s; }
	real_t get_xy() const { return rotor.xy; }
	real_t get_xz() const { return rotor.xz; }
	real_t get_xw() const { return rotor.xw; }
	real_t get_yz() const { return rotor.yz; }
	real_t get_yw() const { return rotor.yw; }
	real_t get_zw() const { return rotor.zw; }
	real_t get_xyzw() const { return rotor.xyzw; }
	void set_s(const real_t p_s) { rotor.s = p_s; }
	void set_xy(const real_t p_xy) { rotor.xy = p_xy; }
	void set_xz(const real_t p_xz) { rotor.xz = p_xz; }
	void set_xw(const real_t p_xw) { rotor.xw = p_xw; }
	void set_yz(const real_t p_yz) { rotor.yz = p_yz; }
	void set_yw(const real_t p_yw) { rotor.yw = p_yw; }
	void set_zw(const real_t p_zw) { rotor.zw = p_zw; }
	void set_xyzw(const real_t p_xyzw) { rotor.xyzw = p_xyzw; }

	// Helper getters/setters.
	::Rotor4D get_rotor() const { return rotor; }
	void set_rotor(const ::Rotor4D &p_rotor) { rotor = p_rotor; }

	AABB get_bivector_aabb() const;
	void set_bivector_aabb(const AABB &p_bivector_aabb);

	// Conversion.
	static Ref<Rotor4D> from_array(const PackedRealArray &p_from_array);
	PackedRealArray to_array() const;
	static Ref<Rotor4D> from_numbers(const real_t p_s, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw, const real_t p_xyzw = 0.0);
	static Ref<Rotor4D> from_vectors(const Vector4 &p_s_xy_xz_xw, const Vector4 &p_yz_yw_zw_xyzw);
	Ref<Rotor4D> copy() const;
#if GDEXTENSION
	String _to_string() const;
#elif GODOT_MODULE
	virtual String to_string() override;
#endif

	// Constructors.
	Rotor4D() {}
	Rotor4D(const real_t p_scalar, const Bivector4D &p_bivector, const real_t p_pseudoscalar = 0.0);
	Rotor4D(const real_t p_s, const real_t p_xy, const real_t p_xz, const real_t p_xw, const real_t p_yz, const real_t p_yw, const real_t p_zw, const real_t p_xyzw = 0.0);
};
} // namespace godot_4d_bind
