#include "rigid_body_4d.h"

void RigidBody4D::apply_force(const Vector4 &p_force, const Vector4 &p_position_offset) {
	// Force is in kg*m/s^2, so multiplying that by s/kg gives m/s for velocity.
	const Vector4 effective_force = p_force * (get_physics_process_delta_time() / _mass);
	_linear_velocity += effective_force;
	// A force at a position may also cause a torque.
	if (p_position_offset != Vector4()) {
		Bivector4D effective_torque = Bivector4D::vector_wedge_product(p_position_offset, effective_force);
		_angular_velocity += effective_torque;
	}
}

void RigidBody4D::apply_impulse(const Vector4 &p_impulse, const Vector4 &p_position_offset) {
	// Impulse is in kg*m/s, so dividing that by kg gives m/s for velocity.
	const Vector4 effective_impulse = p_impulse / _mass;
	_linear_velocity += effective_impulse;
	// An impulse at a position may also cause a torque.
	if (p_position_offset != Vector4()) {
		Bivector4D effective_torque = Bivector4D::vector_wedge_product(p_position_offset, effective_impulse);
		_angular_velocity += effective_torque;
	}
}

void RigidBody4D::apply_local_force(const Vector4 &p_local_force, const Vector4 &p_local_position_offset) {
	const Basis4D global_basis = get_global_basis();
	// Force is in kg*m/s^2, so multiplying that by s/kg gives m/s for velocity.
	const Vector4 effective_global_force = global_basis.xform(p_local_force * (get_physics_process_delta_time() / _mass));
	_linear_velocity += effective_global_force;
	// A force at a position may also cause a torque.
	if (p_local_position_offset != Vector4()) {
		const Vector4 global_position_offset = global_basis.xform(p_local_position_offset);
		const Bivector4D effective_global_torque = Bivector4D::vector_wedge_product(global_position_offset, effective_global_force);
		_angular_velocity += effective_global_torque;
	}
}

void RigidBody4D::apply_local_impulse(const Vector4 &p_local_impulse, const Vector4 &p_local_position_offset) {
	const Basis4D global_basis = get_global_basis();
	// Impulse is in kg*m/s, so dividing that by kg gives m/s for velocity.
	const Vector4 effective_global_impulse = global_basis.xform(p_local_impulse / _mass);
	_linear_velocity += effective_global_impulse;
	// An impulse at a position may also cause a torque.
	if (p_local_position_offset != Vector4()) {
		const Vector4 global_position_offset = global_basis.xform(p_local_position_offset);
		const Bivector4D effective_global_torque = Bivector4D::vector_wedge_product(global_position_offset, effective_global_impulse);
		_angular_velocity += effective_global_torque;
	}
}

void RigidBody4D::apply_torque(const Bivector4D &p_torque) {
	// TODO: None of the code in this class considers the inertia tensor.
	// Non-trivial physical objects can be rotated more easily in different directions.
	// However, for now, this works pretty well, and works perfectly for simple objects (boxes, spheres, etc).
	_angular_velocity += p_torque / _mass;
}

void RigidBody4D::apply_torque_impulse(const Bivector4D &p_torque_impulse) {
	_angular_velocity += p_torque_impulse / _mass;
}

void RigidBody4D::apply_local_torque(const Bivector4D &p_torque) {
	Basis4D global_basis = get_global_basis();
	_angular_velocity += global_basis.rotate_bivector(p_torque) / _mass;
}

void RigidBody4D::apply_local_torque_impulse(const Bivector4D &p_torque_impulse) {
	Basis4D global_basis = get_global_basis();
	_angular_velocity += global_basis.rotate_bivector(p_torque_impulse) / _mass;
}

void RigidBody4D::apply_torque_bind(const AABB &p_torque) {
	apply_torque(p_torque);
}

void RigidBody4D::apply_torque_impulse_bind(const AABB &p_torque_impulse) {
	apply_torque_impulse(p_torque_impulse);
}

void RigidBody4D::apply_local_torque_bind(const AABB &p_torque) {
	apply_local_torque(p_torque);
}

void RigidBody4D::apply_local_torque_impulse_bind(const AABB &p_torque_impulse) {
	apply_local_torque_impulse(p_torque_impulse);
}

real_t RigidBody4D::get_mass() const {
	return _mass;
}

void RigidBody4D::set_mass(const real_t p_mass) {
	ERR_FAIL_COND_MSG(p_mass <= 0.0, "RigidBody4D mass must be greater than 0.");
	_mass = p_mass;
}

real_t RigidBody4D::get_gravity_scale() const {
	return _gravity_scale;
}

void RigidBody4D::set_gravity_scale(const real_t p_gravity_scale) {
	_gravity_scale = p_gravity_scale;
}

Vector4 RigidBody4D::get_linear_velocity() const {
	return _linear_velocity;
}

void RigidBody4D::set_linear_velocity(const Vector4 &p_linear_velocity) {
	_linear_velocity = p_linear_velocity;
}

Bivector4D RigidBody4D::get_angular_velocity() const {
	return _angular_velocity;
}

void RigidBody4D::set_angular_velocity(const Bivector4D &p_angular_velocity) {
	_angular_velocity = p_angular_velocity;
}

AABB RigidBody4D::get_angular_velocity_bind() const {
	return _angular_velocity;
}

void RigidBody4D::set_angular_velocity_bind(const AABB &p_angular_velocity) {
	_angular_velocity = p_angular_velocity;
}

Bivector4D RigidBody4D::get_angular_velocity_degrees() const {
	return _angular_velocity * (360.0 / Math_TAU);
}

void RigidBody4D::set_angular_velocity_degrees(const Bivector4D &p_angular_velocity_degrees) {
	_angular_velocity = p_angular_velocity_degrees * (Math_TAU / 360.0);
}

AABB RigidBody4D::get_angular_velocity_degrees_bind() const {
	return get_angular_velocity_degrees();
}

void RigidBody4D::set_angular_velocity_degrees_bind(const AABB &p_angular_velocity_degrees) {
	set_angular_velocity_degrees(p_angular_velocity_degrees);
}

void RigidBody4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("apply_force", "force", "position_offset"), &RigidBody4D::apply_force, DEFVAL(Vector4()));
	ClassDB::bind_method(D_METHOD("apply_impulse", "impulse", "position_offset"), &RigidBody4D::apply_impulse, DEFVAL(Vector4()));
	ClassDB::bind_method(D_METHOD("apply_local_force", "local_force", "local_position_offset"), &RigidBody4D::apply_local_force, DEFVAL(Vector4()));
	ClassDB::bind_method(D_METHOD("apply_local_impulse", "local_impulse", "local_position_offset"), &RigidBody4D::apply_local_impulse, DEFVAL(Vector4()));

	ClassDB::bind_method(D_METHOD("apply_torque", "torque"), &RigidBody4D::apply_torque_bind);
	ClassDB::bind_method(D_METHOD("apply_torque_impulse", "torque_impulse"), &RigidBody4D::apply_torque_impulse_bind);
	ClassDB::bind_method(D_METHOD("apply_local_torque", "torque"), &RigidBody4D::apply_local_torque_bind);
	ClassDB::bind_method(D_METHOD("apply_local_torque_impulse", "torque_impulse"), &RigidBody4D::apply_local_torque_impulse_bind);

	ClassDB::bind_method(D_METHOD("get_mass"), &RigidBody4D::get_mass);
	ClassDB::bind_method(D_METHOD("set_mass", "mass"), &RigidBody4D::set_mass);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mass", PROPERTY_HINT_RANGE, "0.01,100000,0.01,or_greater,exp,suffix:kg"), "set_mass", "get_mass");

	ClassDB::bind_method(D_METHOD("get_gravity_scale"), &RigidBody4D::get_gravity_scale);
	ClassDB::bind_method(D_METHOD("set_gravity_scale", "gravity_scale"), &RigidBody4D::set_gravity_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity_scale", PROPERTY_HINT_RANGE, "-8,8,0.001,or_less,or_greater"), "set_gravity_scale", "get_gravity_scale");

	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &RigidBody4D::get_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "linear_velocity"), &RigidBody4D::set_linear_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "linear_velocity", PROPERTY_HINT_NONE, "suffix:m/s"), "set_linear_velocity", "get_linear_velocity");

	ClassDB::bind_method(D_METHOD("get_angular_velocity"), &RigidBody4D::get_angular_velocity_bind);
	ClassDB::bind_method(D_METHOD("set_angular_velocity", "angular_velocity"), &RigidBody4D::set_angular_velocity_bind);
	ADD_PROPERTY(PropertyInfo(Variant::AABB, "angular_velocity", PROPERTY_HINT_NONE, "radians_as_degrees", PROPERTY_USAGE_STORAGE), "set_angular_velocity", "get_angular_velocity");

	ClassDB::bind_method(D_METHOD("get_angular_velocity_degrees"), &RigidBody4D::get_angular_velocity_degrees_bind);
	ClassDB::bind_method(D_METHOD("set_angular_velocity_degrees", "angular_velocity_degrees"), &RigidBody4D::set_angular_velocity_degrees_bind);
	ADD_PROPERTY(PropertyInfo(Variant::AABB, "angular_velocity_degrees", PROPERTY_HINT_NONE, U"suffix:\u00B0", PROPERTY_USAGE_EDITOR), "set_angular_velocity_degrees", "get_angular_velocity_degrees");
}
