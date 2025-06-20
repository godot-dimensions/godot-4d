#pragma once

#include "physics_body_4d.h"

#include "../../math/geometric_algebra/bivector_4d.h"

class RigidBody4D : public PhysicsBody4D {
	GDCLASS(RigidBody4D, PhysicsBody4D);

	Bivector4D _angular_velocity;
	real_t _mass = 1.0f;
	real_t _gravity_scale = 1.0f;
	Vector4 _linear_velocity;

protected:
	static void _bind_methods();

public:
	void apply_acceleration(const Vector4 &p_acceleration);
	void apply_local_acceleration(const Vector4 &p_local_acceleration);

	void apply_force(const Vector4 &p_force, const Vector4 &p_position_offset = Vector4());
	void apply_impulse(const Vector4 &p_impulse, const Vector4 &p_position_offset = Vector4());
	void apply_local_force(const Vector4 &p_local_force, const Vector4 &p_local_position_offset = Vector4());
	void apply_local_impulse(const Vector4 &p_local_impulse, const Vector4 &p_local_position_offset = Vector4());

	void apply_torque(const Bivector4D &p_torque);
	void apply_torque_impulse(const Bivector4D &p_torque_impulse);
	void apply_local_torque(const Bivector4D &p_torque);
	void apply_local_torque_impulse(const Bivector4D &p_torque_impulse);

	void apply_torque_bind(const AABB &p_torque);
	void apply_torque_impulse_bind(const AABB &p_torque_impulse);
	void apply_local_torque_bind(const AABB &p_torque);
	void apply_local_torque_impulse_bind(const AABB &p_torque_impulse);

	Vector4 get_scaled_gravity() const;

	real_t get_mass() const;
	void set_mass(const real_t p_mass);

	real_t get_gravity_scale() const;
	void set_gravity_scale(const real_t p_gravity_scale);

	Vector4 get_linear_velocity() const;
	void set_linear_velocity(const Vector4 &p_linear_velocity);

	Bivector4D get_angular_velocity() const;
	void set_angular_velocity(const Bivector4D &p_angular_velocity);

	AABB get_angular_velocity_bind() const;
	void set_angular_velocity_bind(const AABB &p_angular_velocity);

	Bivector4D get_angular_velocity_degrees() const;
	void set_angular_velocity_degrees(const Bivector4D &p_angular_velocity_degrees);

	AABB get_angular_velocity_degrees_bind() const;
	void set_angular_velocity_degrees_bind(const AABB &p_angular_velocity_degrees);
};
