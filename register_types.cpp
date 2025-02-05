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
#include "nodes/quad_split_container.h"

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
#include "mesh/wire/wire_mesh_builder_4d.h"

// Physics.
#include "physics/bodies/area_4d.h"
#include "physics/bodies/character_body_4d.h"
#include "physics/bodies/rigid_body_4d.h"
#include "physics/bodies/static_body_4d.h"
#include "physics/collision_shape_4d.h"
#include "physics/kinematic_collision_4d.h"
#include "physics/server/axis_aligned_box_physics_engine_4d.h"
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
#include "render/wireframe_canvas/wireframe_canvas_rendering_engine_4d.h"

#if GDEXTENSION
// GDExtension has a nervous breakdown whenever singleton or casted classes are not registered.
// We don't need to register these in principle, and we don't need it for a module, just for GDExtension.
#include "physics/server/ghost_physics_engine_4d.h"
#include "render/wireframe_canvas/wireframe_canvas_rendering_engine_4d.h"
#include "render/wireframe_canvas/wireframe_render_canvas.h"
#ifdef TOOLS_ENABLED
#include "editor/godot_4d_editor_plugin.h"
#include "editor/off/editor_import_plugin_off_base.h"
#include "editor/off/editor_import_plugin_off_mesh_3d.h"
#include "editor/off/editor_import_plugin_off_scene.h"
#include "editor/off/editor_import_plugin_off_tetra_4d.h"
#include "editor/off/editor_import_plugin_off_wire_4d.h"
#include "editor/viewport/editor_camera_4d.h"
#include "editor/viewport/editor_input_surface_4d.h"
#include "editor/viewport/editor_main_screen_4d.h"
#include "editor/viewport/editor_main_viewport_4d.h"
#include "editor/viewport/editor_transform_gizmo_4d.h"
#include "editor/viewport/editor_viewport_rotation_4d.h"
#endif // TOOLS_ENABLED
#endif // GDEXTENSION

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
	// Note: Classes MUST be registered in inheritance order.
	// When the inheritance doesn't matter, alphabetical order is used.
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
		GDREGISTER_CLASS(QuadSplitContainer);
		add_godot_singleton("Basis4D", memnew(godot_4d_bind::Basis4D));
		add_godot_singleton("Geometry4D", memnew(Geometry4D));
		add_godot_singleton("Vector4D", memnew(Vector4D));
		// Virtual classes.
		GDREGISTER_VIRTUAL_CLASS(CollisionObject4D);
		GDREGISTER_VIRTUAL_CLASS(Material4D);
		GDREGISTER_VIRTUAL_CLASS(Mesh4D);
		GDREGISTER_VIRTUAL_CLASS(PhysicsBody4D);
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
		GDREGISTER_CLASS(WireMeshBuilder4D);
		add_godot_singleton("WireMeshBuilder4D", memnew(WireMeshBuilder4D));
		// Physics.
		GDREGISTER_CLASS(Area4D);
		GDREGISTER_CLASS(BoxShape4D);
		GDREGISTER_CLASS(CapsuleShape4D);
		GDREGISTER_CLASS(CharacterBody4D);
		GDREGISTER_CLASS(CollisionShape4D);
		GDREGISTER_CLASS(CubinderShape4D);
		GDREGISTER_CLASS(CylinderShape4D);
		GDREGISTER_CLASS(DuocylinderShape4D);
		GDREGISTER_CLASS(KinematicCollision4D);
		GDREGISTER_CLASS(OrthoplexShape4D);
		GDREGISTER_CLASS(RigidBody4D);
		GDREGISTER_CLASS(SphereShape4D);
		GDREGISTER_CLASS(StaticBody4D);
#if GDEXTENSION
		GDREGISTER_CLASS(AxisAlignedBoxPhysicsEngine4D);
		GDREGISTER_CLASS(GhostPhysicsEngine4D);
		GDREGISTER_CLASS(WireframeRenderCanvas);
		GDREGISTER_CLASS(WireframeCanvasRenderingEngine4D);
#endif // GDEXTENSION
		PhysicsServer4D *physics_server = memnew(PhysicsServer4D);
#ifdef TOOLS_ENABLED
		physics_server->set_active(!Engine::get_singleton()->is_editor_hint());
#endif // TOOLS_ENABLED
		physics_server->register_physics_engine("AxisAlignedBoxPhysicsEngine4D", memnew(AxisAlignedBoxPhysicsEngine4D));
		physics_server->register_physics_engine("GhostPhysicsEngine4D", memnew(GhostPhysicsEngine4D));
		add_godot_singleton("PhysicsServer4D", physics_server);
		// Render.
		RenderingServer4D *rendering_server = memnew(RenderingServer4D);
		rendering_server->register_rendering_engine("Wireframe Canvas", memnew(WireframeCanvasRenderingEngine4D));
		add_godot_singleton("RenderingServer4D", rendering_server);
#ifdef TOOLS_ENABLED
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
#ifdef GDEXTENSION
		GDREGISTER_CLASS(EditorCamera4D);
		GDREGISTER_CLASS(EditorImportPluginOFFBase);
		GDREGISTER_CLASS(EditorImportPluginOFFMesh3D);
		GDREGISTER_CLASS(EditorImportPluginOFFScene);
		GDREGISTER_CLASS(EditorImportPluginOFFTetra4D);
		GDREGISTER_CLASS(EditorImportPluginOFFWire4D);
		GDREGISTER_CLASS(EditorInputSurface4D);
		GDREGISTER_CLASS(EditorMainScreen4D);
		GDREGISTER_CLASS(EditorMainViewport4D);
		GDREGISTER_CLASS(EditorTransformGizmo4D);
		GDREGISTER_CLASS(EditorViewportRotation4D);
		GDREGISTER_CLASS(Godot4DEditorPlugin);
#endif // GDEXTENSION
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
