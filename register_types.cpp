#include "register_types.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_plugin_registration.hpp>
#include <godot_cpp/classes/engine.hpp>
#elif GODOT_MODULE
#include "core/config/engine.h"
#include "editor/plugins/editor_plugin.h"
#endif

// General.
#include "math/basis_4d_bind.h"
#include "math/euler_4d_bind.h"
#include "math/geometric_algebra/rotor_4d_bind.h"
#include "math/geometry_4d.h"
#include "math/transform_4d_bind.h"
#include "math/vector_4d.h"
#include "nodes/camera_4d.h"
#include "nodes/node_4d.h"

// Virtual classes.
#include "mesh/material_4d.h"
#include "mesh/mesh_4d.h"
#include "mesh/tetra/tetra_mesh_4d.h"
#include "mesh/wire/wire_mesh_4d.h"
#include "physics/shapes/shape_4d.h"

// Mesh.
#include "mesh/mesh_instance_4d.h"
#include "mesh/off/off_document.h"
#include "mesh/tetra/array_tetra_mesh_4d.h"
#include "mesh/tetra/box_tetra_mesh_4d.h"
#include "mesh/tetra/orthoplex_tetra_mesh_4d.h"
#include "mesh/tetra/tetra_material_4d.h"
#include "mesh/wire/array_wire_mesh_4d.h"
#include "mesh/wire/box_wire_mesh_4d.h"
#include "mesh/wire/orthoplex_wire_mesh_4d.h"
#include "mesh/wire/wire_material_4d.h"

// Physics.
#include "physics/bodies/area_4d.h"
#include "physics/bodies/character_body_4d.h"
#include "physics/bodies/rigid_body_4d.h"
#include "physics/bodies/static_body_4d.h"
#include "physics/collision_shape_4d.h"
#include "physics/server/ghost_physics_engine_4d.h"
#include "physics/server/physics_engine_4d.h"
#include "physics/server/physics_server_4d.h"
#include "physics/shapes/box_shape_4d.h"
#include "physics/shapes/capsule_shape_4d.h"
#include "physics/shapes/cubinder_shape_4d.h"
#include "physics/shapes/cylinder_shape_4d.h"
#include "physics/shapes/duocylinder_shape_4d.h"
#include "physics/shapes/orthoplex_shape_4d.h"
#include "physics/shapes/sphere_shape_4d.h"

// Render.
#include "render/rendering_engine_4d.h"
#include "render/rendering_server_4d.h"

#ifdef TOOLS_ENABLED
#include "editor/godot_4d_editor_plugin.h"
#endif // TOOLS_ENABLED

inline void add_godot_singleton(const StringName &p_singleton_name, Object *p_object) {
#if GDEXTENSION
	Engine::get_singleton()->register_singleton(p_singleton_name, p_object);
#elif GODOT_MODULE
	Engine::get_singleton()->add_singleton(Engine::Singleton(p_singleton_name, p_object));
#endif
}

inline void remove_godot_singleton(const StringName &p_singleton_name) {
#if GDEXTENSION
	Engine::get_singleton()->unregister_singleton(p_singleton_name);
#elif GODOT_MODULE
	Engine::get_singleton()->remove_singleton(p_singleton_name);
#endif
}

void initialize_4d_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE) {
		// General.
		GDREGISTER_CLASS(godot_4d_bind::Basis4D);
		GDREGISTER_CLASS(godot_4d_bind::Euler4D);
		GDREGISTER_CLASS(godot_4d_bind::Rotor4D);
		GDREGISTER_CLASS(godot_4d_bind::Transform4D);
		GDREGISTER_CLASS(Geometry4D);
		GDREGISTER_CLASS(Vector4D);
		// Physics.
		GDREGISTER_VIRTUAL_CLASS(PhysicsEngine4D);
		GDREGISTER_CLASS(PhysicsServer4D);
		// Render.
		GDREGISTER_VIRTUAL_CLASS(RenderingEngine4D);
		GDREGISTER_CLASS(RenderingServer4D);
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// General.
		GDREGISTER_CLASS(Node4D);
		GDREGISTER_CLASS(Camera4D);
		add_godot_singleton("Basis4D", memnew(godot_4d_bind::Basis4D));
		add_godot_singleton("Geometry4D", memnew(Geometry4D));
		add_godot_singleton("Vector4D", memnew(Vector4D));
		// Virtual classes.
		GDREGISTER_VIRTUAL_CLASS(Material4D);
		GDREGISTER_VIRTUAL_CLASS(Mesh4D);
		GDREGISTER_VIRTUAL_CLASS(Shape4D);
		GDREGISTER_VIRTUAL_CLASS(TetraMesh4D);
		GDREGISTER_VIRTUAL_CLASS(WireMesh4D);
		// Mesh.
		GDREGISTER_CLASS(ArrayTetraMesh4D);
		GDREGISTER_CLASS(ArrayWireMesh4D);
		GDREGISTER_CLASS(BoxTetraMesh4D);
		GDREGISTER_CLASS(BoxWireMesh4D);
		GDREGISTER_CLASS(MeshInstance4D);
		GDREGISTER_CLASS(OFFDocument);
		GDREGISTER_CLASS(OrthoplexTetraMesh4D);
		GDREGISTER_CLASS(OrthoplexWireMesh4D);
		GDREGISTER_CLASS(TetraMaterial4D);
		GDREGISTER_CLASS(WireMaterial4D);
		// Physics.
		GDREGISTER_CLASS(Area4D);
		GDREGISTER_CLASS(BoxShape4D);
		GDREGISTER_CLASS(CapsuleShape4D);
		GDREGISTER_CLASS(CharacterBody4D);
		GDREGISTER_CLASS(CollisionShape4D);
		GDREGISTER_CLASS(CubinderShape4D);
		GDREGISTER_CLASS(CylinderShape4D);
		GDREGISTER_CLASS(DuocylinderShape4D);
		GDREGISTER_CLASS(OrthoplexShape4D);
		GDREGISTER_CLASS(RigidBody4D);
		GDREGISTER_CLASS(SphereShape4D);
		GDREGISTER_CLASS(StaticBody4D);
		GDREGISTER_VIRTUAL_CLASS(CollisionObject4D);
		GDREGISTER_VIRTUAL_CLASS(PhysicsBody4D);
		PhysicsServer4D *physics_server = memnew(PhysicsServer4D);
#ifdef TOOLS_ENABLED
		physics_server->set_active(!Engine::get_singleton()->is_editor_hint());
#endif // TOOLS_ENABLED
		physics_server->register_physics_engine("GhostPhysicsEngine4D", memnew(GhostPhysicsEngine4D));
		add_godot_singleton("PhysicsServer4D", physics_server);
		// Render.
		add_godot_singleton("RenderingServer4D", memnew(RenderingServer4D));
#ifdef TOOLS_ENABLED
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		EditorPlugins::add_by_type<Godot4DEditorPlugin>();
#endif // TOOLS_ENABLED
	}
}

void uninitialize_4d_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		remove_godot_singleton("Basis4D");
		remove_godot_singleton("Geometry4D");
		remove_godot_singleton("Vector4D");
	}
}
