#pragma once

#include "collision_object_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/physics_material.hpp>
#elif GODOT_MODULE
#include "scene/resources/physics_material.h"
#endif

class PhysicsBody4D : public CollisionObject4D {
	GDCLASS(PhysicsBody4D, CollisionObject4D);

	Ref<PhysicsMaterial> _physics_material;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void move_and_collide(Vector4 p_motion);

	Ref<PhysicsMaterial> get_physics_material() const;
	void set_physics_material(const Ref<PhysicsMaterial> &p_physics_material);
};
