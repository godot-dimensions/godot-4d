#pragma once

#include "../../../physics/bodies/area_4d.h"
#include "../../../physics/bodies/static_body_4d.h"
#include "../../../physics/server/physics_server_4d.h"
#include "../../../physics/shapes/box_shape_4d.h"
#include "../../../physics/shapes/plane_shape_4d.h"
#include "../../../physics/shapes/sphere_shape_4d.h"

#include "core/os/thread.h"
#include "scene/main/scene_tree.h"
#include "tests/test_macros.h"

namespace TestPhysicsServer4D {

struct RaycastTestEntry {
	CollisionObject4D *body = nullptr;
	CollisionShape4D *shape = nullptr;
};

class RaycastTestScene {
	Vector<RaycastTestEntry> entries;

public:
	RaycastTestEntry add_shape(const Ref<Shape4D> &p_shape, const Transform4D &p_transform = Transform4D(), bool p_is_area = false, uint32_t p_collision_layer = 1) {
		CollisionObject4D *body = p_is_area ? static_cast<CollisionObject4D *>(memnew(Area4D)) : static_cast<CollisionObject4D *>(memnew(StaticBody4D));
		CollisionShape4D *collision_shape = memnew(CollisionShape4D);
		collision_shape->set_shape(p_shape);
		collision_shape->set_transform(p_transform);
		collision_shape->set_collision_layer(p_collision_layer);
		body->add_child(collision_shape);
		SceneTree::get_singleton()->get_root()->add_child(body);
		const RaycastTestEntry entry = { body, collision_shape };
		entries.append(entry);
		return entry;
	}

	RaycastTestEntry add_box(const Vector4 &p_position, const Vector4 &p_size = Vector4(2, 2, 2, 2), const Basis4D &p_basis = Basis4D(), bool p_is_area = false, uint32_t p_collision_layer = 1) {
		Ref<BoxShape4D> box;
		box.instantiate();
		box->set_size(p_size);
		return add_shape(box, Transform4D(p_basis, p_position), p_is_area, p_collision_layer);
	}

	~RaycastTestScene() {
		for (int64_t i = entries.size() - 1; i >= 0; i--) {
			CollisionObject4D *body = entries[i].body;
			CollisionShape4D *shape = entries[i].shape;
			if (body->get_parent() != nullptr) {
				body->get_parent()->remove_child(body);
			}
			if (shape->get_parent() != nullptr) {
				shape->get_parent()->remove_child(shape);
			}
			memdelete(shape);
			memdelete(body);
		}
	}
};

static Ref<RaycastParameters4D> make_parameters(const Vector4 &p_origin, const Vector4 &p_direction) {
	Ref<RaycastParameters4D> parameters;
	parameters.instantiate();
	parameters->set_global_ray_origin(p_origin);
	parameters->set_global_ray_direction(p_direction);
	return parameters;
}

static bool result_is_hit(const Dictionary &p_result) {
	return p_result.has("hit") && (bool)p_result["hit"];
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast returns local and global hit information") {
	RaycastTestScene scene;
	const RaycastTestEntry entry = scene.add_box(Vector4(3, 0, 0, 0));
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(), Vector4(1, 0, 0, 0));

	const Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((double)result["distance"] == doctest::Approx(2.0));
	CHECK((double)result["global_distance"] == doctest::Approx(2.0));
	CHECK(((Vector4)result["point"]).is_equal_approx(Vector4(-1, 0, 0, 0)));
	CHECK(((Vector4)result["global_point"]).is_equal_approx(Vector4(2, 0, 0, 0)));
	CHECK(((Vector4)result["normal"]).is_equal_approx(Vector4(-1, 0, 0, 0)));
	CHECK(((Vector4)result["global_normal"]).is_equal_approx(Vector4(-1, 0, 0, 0)));
	CHECK((Object *)result["body"] == entry.body);
	CHECK((Object *)result["shape"] == entry.shape);
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast returns the nearest hit regardless of registration order") {
	RaycastTestScene scene;
	const RaycastTestEntry far_entry = scene.add_box(Vector4(7, 0, 0, 0));
	const RaycastTestEntry near_entry = scene.add_box(Vector4(3, 0, 0, 0));
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(), Vector4(1, 0, 0, 0));

	const Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((double)result["global_distance"] == doctest::Approx(2.0));
	CHECK((Object *)result["shape"] == near_entry.shape);
	CHECK((Object *)result["shape"] != far_entry.shape);
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast collision masks filter individual shapes") {
	RaycastTestScene scene;
	const RaycastTestEntry layer_two = scene.add_box(Vector4(3, 0, 0, 0), Vector4(2, 2, 2, 2), Basis4D(), false, 1u << 1);
	const RaycastTestEntry layer_one = scene.add_box(Vector4(7, 0, 0, 0), Vector4(2, 2, 2, 2), Basis4D(), false, 1u << 0);
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(), Vector4(1, 0, 0, 0));

	parameters->set_collision_mask(1u << 0);
	Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((Object *)result["shape"] == layer_one.shape);

	parameters->set_collision_mask(1u << 1);
	result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((Object *)result["shape"] == layer_two.shape);

	parameters->set_collision_mask(1u << 2);
	result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	CHECK_FALSE(result_is_hit(result));
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast filters bodies and areas independently") {
	RaycastTestScene scene;
	const RaycastTestEntry body = scene.add_box(Vector4(3, 0, 0, 0));
	const RaycastTestEntry area = scene.add_box(Vector4(7, 0, 0, 0), Vector4(2, 2, 2, 2), Basis4D(), true);
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(), Vector4(1, 0, 0, 0));

	Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((Object *)result["body"] == body.body);

	parameters->set_collide_with_bodies(false);
	parameters->set_collide_with_areas(true);
	result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((Object *)result["body"] == area.body);

	parameters->set_collide_with_areas(false);
	ERR_PRINT_OFF;
	result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	ERR_PRINT_ON;
	CHECK(result.is_empty());
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast excludes whole bodies or individual shapes") {
	RaycastTestScene scene;
	const RaycastTestEntry near_entry = scene.add_box(Vector4(3, 0, 0, 0));
	const RaycastTestEntry far_entry = scene.add_box(Vector4(7, 0, 0, 0));
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(), Vector4(1, 0, 0, 0));
	Vector<ObjectID> excluded_nodes;

	excluded_nodes.append(near_entry.shape->get_instance_id());
	parameters->set_exclude_nodes(excluded_nodes);
	Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((Object *)result["shape"] == far_entry.shape);

	excluded_nodes.set(0, near_entry.body->get_instance_id());
	parameters->set_exclude_nodes(excluded_nodes);
	result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((Object *)result["shape"] == far_entry.shape);

	excluded_nodes.append(far_entry.body->get_instance_id());
	parameters->set_exclude_nodes(excluded_nodes);
	result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	CHECK_FALSE(result_is_hit(result));
}

TEST_CASE("[RaycastParameters4D] Excluded node bindings round-trip live nodes as ObjectIDs") {
	Ref<RaycastParameters4D> parameters;
	parameters.instantiate();
	Node4D *first_node = memnew(Node4D);
	Node4D *second_node = memnew(Node4D);
	TypedArray<Node4D> excluded_nodes;
	excluded_nodes.append(first_node);
	excluded_nodes.append(second_node);

	parameters->set_exclude_nodes_bind(excluded_nodes);
	const Vector<ObjectID> &excluded_ids = parameters->get_exclude_nodes();
	REQUIRE(excluded_ids.size() == 2);
	CHECK(excluded_ids[0] == first_node->get_instance_id());
	CHECK(excluded_ids[1] == second_node->get_instance_id());

	const TypedArray<Node4D> round_trip_nodes = parameters->get_exclude_nodes_bind();
	REQUIRE(round_trip_nodes.size() == 2);
	CHECK((Object *)round_trip_nodes[0] == first_node);
	CHECK((Object *)round_trip_nodes[1] == second_node);

	excluded_nodes.clear();
	memdelete(first_node);
	memdelete(second_node);
}

TEST_CASE("[SceneTree][RaycastParameters4D] Freed excluded nodes remain safe") {
	RaycastTestScene scene;
	const RaycastTestEntry entry = scene.add_box(Vector4(3, 0, 0, 0));
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(), Vector4(1, 0, 0, 0));
	Node4D *excluded_node = memnew(Node4D);
	const ObjectID excluded_id = excluded_node->get_instance_id();
	TypedArray<Node4D> excluded_nodes;
	excluded_nodes.append(excluded_node);
	parameters->set_exclude_nodes_bind(excluded_nodes);
	excluded_nodes.clear();
	memdelete(excluded_node);
	REQUIRE(ObjectDB::get_instance(excluded_id) == nullptr);

	ERR_PRINT_OFF;
	const TypedArray<Node4D> resolved_nodes = parameters->get_exclude_nodes_bind();
	ERR_PRINT_ON;
	REQUIRE(resolved_nodes.size() == 1);
	CHECK((Object *)resolved_nodes[0] == nullptr);

	const Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((Object *)result["shape"] == entry.shape);
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast max distance is exclusive") {
	RaycastTestScene scene;
	scene.add_box(Vector4(3, 0, 0, 0));
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(), Vector4(1, 0, 0, 0));

	parameters->set_max_distance(1.999);
	CHECK_FALSE(result_is_hit(PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters)));
	parameters->set_max_distance(2.0);
	CHECK_FALSE(result_is_hit(PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters)));
	parameters->set_max_distance(2.001);
	const Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((double)result["global_distance"] == doctest::Approx(2.0));
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast inside handling supports skip, zero, and exit modes") {
	RaycastTestScene scene;
	scene.add_box(Vector4());
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(), Vector4(1, 0, 0, 0));

	CHECK_FALSE(result_is_hit(PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters)));

	parameters->set_inside_is_skip(false);
	parameters->set_inside_is_zero(true);
	Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((double)result["distance"] == doctest::Approx(0.0));
	CHECK((double)result["global_distance"] == doctest::Approx(0.0));
	CHECK(((Vector4)result["point"]).is_zero_approx());
	CHECK(((Vector4)result["normal"]).is_zero_approx());

	parameters->set_inside_is_zero(false);
	result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((double)result["distance"] == doctest::Approx(1.0));
	CHECK((double)result["global_distance"] == doctest::Approx(1.0));
	CHECK(((Vector4)result["point"]).is_equal_approx(Vector4(1, 0, 0, 0)));
	CHECK(((Vector4)result["normal"]).is_equal_approx(Vector4(1, 0, 0, 0)));
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast broad phase uses zero distance inside shape bounds") {
	RaycastTestScene scene;
	Ref<SphereShape4D> sphere;
	sphere.instantiate();
	sphere->set_radius(1.0);
	scene.add_shape(sphere);
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(0.9, 0.9, 0, 0), Vector4(-1, 0, 0, 0));
	parameters->set_max_distance(1.0);

	const Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((double)result["global_distance"] == doctest::Approx(0.9 - Math::sqrt(0.19)));
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast converts distances through uniform scaling") {
	RaycastTestScene scene;
	scene.add_box(Vector4(-10, 0, 0, 0));
	scene.add_box(Vector4(10, 0, 0, 0), Vector4(2, 2, 2, 2), Basis4D::from_scale_uniform(10.0));

	Ref<RaycastParameters4D> parameters = make_parameters(Vector4(-10, 2, 0, 0), Vector4(0, -1, 0, 0));
	Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((double)result["distance"] == doctest::Approx(1.0));
	CHECK((double)result["global_distance"] == doctest::Approx(1.0));

	parameters = make_parameters(Vector4(10, 11, 0, 0), Vector4(0, -1, 0, 0));
	result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	CHECK((double)result["distance"] == doctest::Approx(0.1));
	CHECK((double)result["global_distance"] == doctest::Approx(1.0));
	CHECK(((Vector4)result["global_point"]).is_equal_approx(Vector4(10, 10, 0, 0)));

	parameters->set_max_distance(1.0);
	CHECK_FALSE(result_is_hit(PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters)));
	parameters->set_max_distance(1.001);
	CHECK(result_is_hit(PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters)));
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast transforms normals under nonuniform scaling") {
	RaycastTestScene scene;
	Ref<SphereShape4D> sphere;
	sphere.instantiate();
	sphere->set_radius(1.0);
	const Basis4D nonuniform_scale = Basis4D::from_scale(Vector4(2, 1, 1, 1));
	scene.add_shape(sphere, Transform4D(nonuniform_scale));
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(1, 3, 0, 0), Vector4(0, -1, 0, 0));

	const Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	REQUIRE(result_is_hit(result));
	const double expected_y = Math::sqrt(0.75);
	CHECK((double)result["global_distance"] == doctest::Approx(3.0 - expected_y));
	CHECK(((Vector4)result["global_point"]).is_equal_approx(Vector4(1, expected_y, 0, 0)));
	const Vector4 local_normal = result["normal"];
	const Vector4 expected_global_normal = nonuniform_scale.inverse().transposed().xform(local_normal).normalized();
	CHECK(((Vector4)result["global_normal"]).is_equal_approx(expected_global_normal));
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast rejects a plane hit beyond max distance") {
	RaycastTestScene scene;
	Ref<PlaneShape4D> plane;
	plane.instantiate();
	scene.add_shape(plane, Transform4D(Basis4D(), Vector4(0, 10, 0, 0)));
	const Ref<RaycastParameters4D> parameters = make_parameters(Vector4(), Vector4(0, 1, 0, 0));
	parameters->set_inside_is_skip(false);
	parameters->set_max_distance(5.0);

	const Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	CHECK_FALSE(result_is_hit(result));
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast rejects invalid parameter values") {
	RaycastTestScene scene;
	scene.add_box(Vector4(3, 0, 0, 0));
	Ref<RaycastParameters4D> parameters = make_parameters(Vector4(), Vector4());

	ERR_PRINT_OFF;
	Dictionary result = PhysicsServer4D::get_singleton()->raycast_physics_objects(Ref<RaycastParameters4D>());
	ERR_PRINT_ON;
	CHECK(result.is_empty());

	ERR_PRINT_OFF;
	result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	ERR_PRINT_ON;
	CHECK(result.is_empty());

	parameters = make_parameters(Vector4(), Vector4(1, 0, 0, 0));
	parameters->set_max_distance(0.0);
	ERR_PRINT_OFF;
	result = PhysicsServer4D::get_singleton()->raycast_physics_objects(parameters);
	ERR_PRINT_ON;
	CHECK(result.is_empty());
}

struct ThreadRaycastData {
	Ref<RaycastParameters4D> parameters;
	Dictionary result;
};

static void raycast_from_worker_thread(void *p_userdata) {
	ThreadRaycastData *data = static_cast<ThreadRaycastData *>(p_userdata);
	data->result = PhysicsServer4D::get_singleton()->raycast_physics_objects(data->parameters);
}

TEST_CASE("[SceneTree][PhysicsServer4D] Raycast rejects calls from worker threads") {
	ThreadRaycastData data;
	data.parameters = make_parameters(Vector4(), Vector4(1, 0, 0, 0));
	Thread worker;
	ERR_PRINT_OFF;
	worker.start(raycast_from_worker_thread, &data);
	worker.wait_to_finish();
	ERR_PRINT_ON;
	CHECK(data.result.is_empty());
}
} //namespace TestPhysicsServer4D
