#pragma once

#include "../g4mf_item_4d.h"

#include "../../../../physics/bodies/physics_body_4d.h"

class G4MFNode4D;
class G4MFState4D;

class G4MFNodePhysicsMotion4D : public G4MFItem4D {
	GDCLASS(G4MFNodePhysicsMotion4D, G4MFItem4D);

	Rotor4D _inertia_orientation = Rotor4D::identity();
	Bivector4D _inertia_diagonal;
	Bivector4D _angular_velocity;
	Vector4 _linear_velocity;
	String _motion_type = "dynamic";
	double _mass = 1.0;
	double _gravity_factor = 1.0;

protected:
	static void _bind_methods();

public:
	Rotor4D get_inertia_orientation() const { return _inertia_orientation; }
	void set_inertia_orientation(const Rotor4D &p_inertia_orientation) { _inertia_orientation = p_inertia_orientation; }

	Bivector4D get_inertia_diagonal() const { return _inertia_diagonal; }
	void set_inertia_diagonal(const Bivector4D &p_inertia_diagonal) { _inertia_diagonal = p_inertia_diagonal; }

	AABB get_inertia_diagonal_bind() const { return _inertia_diagonal; }
	void set_inertia_diagonal_bind(const AABB &p_inertia_diagonal) { _inertia_diagonal = p_inertia_diagonal; }

	Bivector4D get_angular_velocity() const { return _angular_velocity; }
	void set_angular_velocity(const Bivector4D &p_angular_velocity) { _angular_velocity = p_angular_velocity; }

	AABB get_angular_velocity_bind() const { return _angular_velocity; }
	void set_angular_velocity_bind(const AABB &p_angular_velocity) { _angular_velocity = p_angular_velocity; }

	Vector4 get_linear_velocity() const { return _linear_velocity; }
	void set_linear_velocity(const Vector4 &p_linear_velocity) { _linear_velocity = p_linear_velocity; }

	String get_motion_type() const { return _motion_type; }
	void set_motion_type(const String &p_motion_type) { _motion_type = p_motion_type; }

	double get_mass() const { return _mass; }
	void set_mass(const double p_mass) { _mass = p_mass; }

	double get_gravity_factor() const { return _gravity_factor; }
	void set_gravity_factor(const double p_gravity_factor) { _gravity_factor = p_gravity_factor; }

	PhysicsBody4D *to_physics_body() const;
	static Ref<G4MFNodePhysicsMotion4D> from_physics_body(const PhysicsBody4D *p_physics_body);

	static Ref<G4MFNodePhysicsMotion4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
