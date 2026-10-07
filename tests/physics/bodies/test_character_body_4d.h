#pragma once

#include "../../../physics/bodies/character_body_4d.h"
#include "../../../physics/server/physics_server_4d.h"

#include "scene/main/scene_tree.h"
#include "tests/test_macros.h"

namespace TestCharacterBody4D {

// Switches the current 4D physics engine, and switches back when destroyed,
// so a failing REQUIRE cannot leave the swapped engine in place for later tests.
class PhysicsEngineOverride {
	String previous_engine_name;
	String temporary_engine_name;

public:
	PhysicsEngineOverride(const String &p_engine_name, const Ref<PhysicsEngine4D> &p_temporary_engine = Ref<PhysicsEngine4D>()) {
		PhysicsServer4D *physics_server = PhysicsServer4D::get_singleton();
		previous_engine_name = physics_server->get_current_physics_engine_name();
		if (p_temporary_engine.is_valid()) {
			temporary_engine_name = p_engine_name;
			physics_server->register_physics_engine(p_engine_name, p_temporary_engine);
		}
		physics_server->set_current_physics_engine_name(p_engine_name);
	}

	~PhysicsEngineOverride() {
		PhysicsServer4D *physics_server = PhysicsServer4D::get_singleton();
		if (!temporary_engine_name.is_empty()) {
			physics_server->unregister_physics_engine(temporary_engine_name);
		}
		physics_server->set_current_physics_engine_name(previous_engine_name);
	}
};

// Calls move_and_slide on a moving body while move_and_collide returns null, and checks that the body stays put.
static void check_move_and_slide_stops_without_collision_result() {
	CharacterBody4D *body = memnew(CharacterBody4D);
	SceneTree::get_singleton()->get_root()->add_child(body);
	const Vector4 start_position = Vector4(1, 2, 3, 4);
	const Vector4 linear_velocity = Vector4(5, 0, 0, 0);
	body->set_position(start_position);
	body->set_linear_velocity(linear_velocity);

	// With no physics engine set, PhysicsServer4D prints an error.
	ERR_PRINT_OFF;
	const TypedArray<KinematicCollision4D> collisions = body->move_and_slide(1.0 / 60.0);
	ERR_PRINT_ON;
	CHECK(collisions.is_empty());
	CHECK(body->get_position() == start_position);
	CHECK(body->get_linear_velocity() == linear_velocity);

	SceneTree::get_singleton()->get_root()->remove_child(body);
	memdelete(body);
}

TEST_CASE("[SceneTree][CharacterBody4D] move_and_slide stops without moving when there is no physics engine") {
	PhysicsEngineOverride no_engine("");
	check_move_and_slide_stops_without_collision_result();
}

TEST_CASE("[SceneTree][CharacterBody4D] move_and_slide stops without moving when the physics engine returns no collision") {
	// The base PhysicsEngine4D returns null from move_and_collide, like a script engine that doesn't implement _move_and_collide.
	Ref<PhysicsEngine4D> null_engine;
	null_engine.instantiate();
	PhysicsEngineOverride null_engine_override("CharacterBody4DTestNullEngine", null_engine);
	check_move_and_slide_stops_without_collision_result();
}
} //namespace TestCharacterBody4D
