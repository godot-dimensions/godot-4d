#pragma once

#include "../bodies/area_4d.h"
#include "../bodies/physics_body_4d.h"
#include "../bodies/rigid_body_4d.h"
#include "../kinematic_collision_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"
#endif

class PhysicsEngine4D : public RefCounted {
	GDCLASS(PhysicsEngine4D, RefCounted);

protected:
	static void _bind_methods();

public:
	virtual Ref<KinematicCollision4D> move_and_collide(PhysicsBody4D *p_body, Vector4 p_motion, bool p_test_only);
	virtual void move_area(Area4D *p_area, Vector4 p_motion);
	virtual void physics_process(double p_delta);

	GDVIRTUAL3R(Ref<KinematicCollision4D>, _move_and_collide, PhysicsBody4D *, Vector4, bool);
	GDVIRTUAL2(_move_area, Area4D *, Vector4);
	GDVIRTUAL1(_physics_process, double);
};
