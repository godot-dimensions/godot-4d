#include "g4mf_node_physics_motion_4d.h"

#include "../../../../math/vector_4d.h"
#include "../../../../physics/bodies/area_4d.h"
#include "../../../../physics/bodies/character_body_4d.h"
#include "../../../../physics/bodies/rigid_body_4d.h"
#include "../../../../physics/bodies/static_body_4d.h"

PhysicsBody4D *G4MFNodePhysicsMotion4D::to_physics_body() const {
	if (_motion_type == "static") {
		StaticBody4D *static_body = memnew(StaticBody4D);
		return static_body;
	}
	if (_motion_type == "kinematic") {
		CharacterBody4D *character_body = memnew(CharacterBody4D);
		return character_body;
	}
	RigidBody4D *rigid_body = memnew(RigidBody4D);
	rigid_body->set_angular_velocity(_angular_velocity);
	rigid_body->set_linear_velocity(_linear_velocity);
	rigid_body->set_mass(_mass);
	rigid_body->set_gravity_scale(_gravity_factor);
	return rigid_body;
}

Ref<G4MFNodePhysicsMotion4D> G4MFNodePhysicsMotion4D::from_physics_body(const PhysicsBody4D *p_physics_body) {
	Ref<G4MFNodePhysicsMotion4D> motion;
	motion.instantiate();
	const RigidBody4D *rigid_body = Object::cast_to<RigidBody4D>(p_physics_body);
	if (rigid_body) {
		motion->set_angular_velocity(rigid_body->get_angular_velocity());
		motion->set_linear_velocity(rigid_body->get_linear_velocity());
		motion->set_mass(rigid_body->get_mass());
		motion->set_gravity_factor(rigid_body->get_gravity_scale());
		motion->set_motion_type("dynamic");
	} else {
		const StaticBody4D *static_body = Object::cast_to<StaticBody4D>(p_physics_body);
		if (static_body) {
			motion->set_motion_type("static");
		} else {
			motion->set_motion_type("kinematic");
		}
	}
	return motion;
}

Ref<G4MFNodePhysicsMotion4D> G4MFNodePhysicsMotion4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFNodePhysicsMotion4D> motion;
	motion.instantiate();
	motion->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("angularVelocity")) {
		motion->set_angular_velocity(json_array_to_bivector_4d(p_dict["angularVelocity"]));
	}
	if (p_dict.has("gravityFactor")) {
		motion->set_gravity_factor(p_dict["gravityFactor"]);
	}
	if (p_dict.has("inertiaDiagonal")) {
		motion->set_inertia_diagonal(json_array_to_bivector_4d(p_dict["inertiaDiagonal"]));
	}
	if (p_dict.has("inertiaOrientation")) {
		motion->set_inertia_orientation(json_array_to_rotor_4d(p_dict["inertiaOrientation"]));
	}
	if (p_dict.has("linearVelocity")) {
		motion->set_linear_velocity(Vector4D::from_json_array(p_dict["linearVelocity"]));
	}
	if (p_dict.has("mass")) {
		motion->set_mass(p_dict["mass"]);
	}
	if (p_dict.has("type")) {
		motion->set_motion_type(p_dict["type"]);
	}
	return motion;
}

Dictionary G4MFNodePhysicsMotion4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_angular_velocity != Bivector4D()) {
		dict["angularVelocity"] = bivector_4d_to_json_array(_angular_velocity);
	}
	if (_gravity_factor != 1.0) {
		dict["gravityFactor"] = _gravity_factor;
	}
	if (_inertia_diagonal != Bivector4D()) {
		dict["inertiaDiagonal"] = bivector_4d_to_json_array(_inertia_diagonal);
	}
	if (_inertia_orientation != Rotor4D::identity()) {
		dict["inertiaOrientation"] = rotor_4d_to_json_array(_inertia_orientation);
	}
	if (_linear_velocity != Vector4()) {
		dict["linearVelocity"] = Vector4D::to_json_array(_linear_velocity);
	}
	if (_mass != 1.0) {
		dict["mass"] = _mass;
	}
	dict["type"] = _motion_type;
	return dict;
}

void G4MFNodePhysicsMotion4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_inertia_diagonal"), &G4MFNodePhysicsMotion4D::get_inertia_diagonal_bind);
	ClassDB::bind_method(D_METHOD("set_inertia_diagonal", "inertia_diagonal"), &G4MFNodePhysicsMotion4D::set_inertia_diagonal_bind);
	ClassDB::bind_method(D_METHOD("get_angular_velocity"), &G4MFNodePhysicsMotion4D::get_angular_velocity_bind);
	ClassDB::bind_method(D_METHOD("set_angular_velocity", "angular_velocity"), &G4MFNodePhysicsMotion4D::set_angular_velocity_bind);
	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &G4MFNodePhysicsMotion4D::get_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "linear_velocity"), &G4MFNodePhysicsMotion4D::set_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_motion_type"), &G4MFNodePhysicsMotion4D::get_motion_type);
	ClassDB::bind_method(D_METHOD("set_motion_type", "motion_type"), &G4MFNodePhysicsMotion4D::set_motion_type);
	ClassDB::bind_method(D_METHOD("get_mass"), &G4MFNodePhysicsMotion4D::get_mass);
	ClassDB::bind_method(D_METHOD("set_mass", "mass"), &G4MFNodePhysicsMotion4D::set_mass);
	ClassDB::bind_method(D_METHOD("get_gravity_factor"), &G4MFNodePhysicsMotion4D::get_gravity_factor);
	ClassDB::bind_method(D_METHOD("set_gravity_factor", "gravity_factor"), &G4MFNodePhysicsMotion4D::set_gravity_factor);

	ClassDB::bind_method(D_METHOD("to_physics_body"), &G4MFNodePhysicsMotion4D::to_physics_body);
	ClassDB::bind_static_method("G4MFNodePhysicsMotion4D", D_METHOD("from_physics_body", "physics_body"), &G4MFNodePhysicsMotion4D::from_physics_body);

	ClassDB::bind_static_method("G4MFNodePhysicsMotion4D", D_METHOD("from_dictionary", "dict"), &G4MFNodePhysicsMotion4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFNodePhysicsMotion4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::AABB, "inertia_diagonal"), "set_inertia_diagonal", "get_inertia_diagonal");
	ADD_PROPERTY(PropertyInfo(Variant::AABB, "angular_velocity"), "set_angular_velocity", "get_angular_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "linear_velocity"), "set_linear_velocity", "get_linear_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "motion_type"), "set_motion_type", "get_motion_type");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mass"), "set_mass", "get_mass");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity_factor"), "set_gravity_factor", "get_gravity_factor");
}
