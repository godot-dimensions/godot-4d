#include "register_types.h"

#if GDEXTENSION
#include <godot_cpp/classes/engine.hpp>
#ifdef TOOLS_ENABLED
#include <godot_cpp/classes/editor_plugin_registration.hpp>
#endif // TOOLS_ENABLED
#elif GODOT_MODULE
#include "core/config/engine.h"
#ifdef TOOLS_ENABLED
#include "editor/plugins/editor_plugin.h"
#include "editor/themes/editor_color_map.h"
#endif // TOOLS_ENABLED
#endif

// General.
#include "math/basis_4d_bind.h"
#include "math/euler_4d_bind.h"
#include "math/geometric_algebra/rotor_4d_bind.h"
#include "math/geometry_4d.h"
#include "math/math_4d.h"
#include "math/transform_4d_bind.h"
#include "math/vector_4d.h"
#include "nodes/camera_4d.h"
#include "nodes/marker_4d.h"
#include "nodes/node_4d.h"
#include "nodes/quad_split_container.h"

// Virtual classes.
#include "model/material_4d.h"
#include "model/mesh_4d.h"
#include "model/tetra/tetra_mesh_4d.h"
#include "model/wire/wire_mesh_4d.h"
#include "physics/shapes/shape_4d.h"

// Model.
#include "model/g4mf/g4mf_document_4d.h"
#include "model/mesh_instance_4d.h"
#include "model/off/off_document_4d.h"
#include "model/tetra/array_tetra_mesh_4d.h"
#include "model/tetra/box_tetra_mesh_4d.h"
#include "model/tetra/orthoplex_tetra_mesh_4d.h"
#include "model/tetra/tetra_material_4d.h"
#include "model/wire/array_wire_mesh_4d.h"
#include "model/wire/box_wire_mesh_4d.h"
#include "model/wire/orthoplex_wire_mesh_4d.h"
#include "model/wire/wire_material_4d.h"
#include "model/wire/wire_mesh_builder_4d.h"

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
#include "physics/shapes/concave_mesh_shape_4d.h"
#include "physics/shapes/convex_hull_shape_4d.h"
#include "physics/shapes/cubinder_shape_4d.h"
#include "physics/shapes/cylinder_shape_4d.h"
#include "physics/shapes/duocylinder_shape_4d.h"
#include "physics/shapes/general_shape_4d.h"
#include "physics/shapes/height_map_shape_4d.h"
#include "physics/shapes/orthoplex_shape_4d.h"
#include "physics/shapes/plane_shape_4d.h"
#include "physics/shapes/ray_shape_4d.h"
#include "physics/shapes/sphere_shape_4d.h"

// Render.
#include "render/cross_section/cross_section_rendering_engine_4d.h"
#include "render/rendering_engine_4d.h"
#include "render/rendering_server_4d.h"
#include "render/wireframe_canvas/wireframe_canvas_rendering_engine_4d.h"

#if GDEXTENSION
// GDExtension has a nervous breakdown whenever singleton or casted classes are not registered.
// We don't need to register these in principle, and we don't need it for a module, just for GDExtension.
#include "physics/server/ghost_physics_engine_4d.h"
#include "render/wireframe_canvas/wireframe_canvas_rendering_engine_4d.h"
#include "render/wireframe_canvas/wireframe_render_canvas_4d.h"
#ifdef TOOLS_ENABLED
#include "editor/godot_4d_editor_plugin.h"
#include "editor/import/off/editor_import_plugin_off_base.h"
#include "editor/import/off/editor_import_plugin_off_mesh_3d.h"
#include "editor/import/off/editor_import_plugin_off_scene.h"
#include "editor/import/off/editor_import_plugin_off_tetra_4d.h"
#include "editor/import/off/editor_import_plugin_off_wire_4d.h"
#include "editor/viewport/editor_camera_4d.h"
#include "editor/viewport/editor_camera_settings_4d.h"
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
		GDREGISTER_CLASS(Math4D);
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
		GDREGISTER_CLASS(OFFDocument4D);
		GDREGISTER_CLASS(OrthoplexTetraMesh4D);
		GDREGISTER_CLASS(OrthoplexWireMesh4D);
		GDREGISTER_CLASS(TetraMaterial4D);
		GDREGISTER_CLASS(WireMaterial4D);
		GDREGISTER_CLASS(WireMeshBuilder4D);
		add_godot_singleton("WireMeshBuilder4D", memnew(WireMeshBuilder4D));
		// Depends on mesh.
		GDREGISTER_CLASS(Marker4D);
		// Physics.
		GDREGISTER_CLASS(Area4D);
		GDREGISTER_CLASS(BoxShape4D);
		GDREGISTER_CLASS(CapsuleShape4D);
		GDREGISTER_CLASS(CharacterBody4D);
		GDREGISTER_CLASS(CollisionShape4D);
		GDREGISTER_CLASS(ConcaveMeshShape4D);
		GDREGISTER_CLASS(ConvexHullShape4D);
		GDREGISTER_CLASS(CubinderShape4D);
		GDREGISTER_CLASS(CylinderShape4D);
		GDREGISTER_CLASS(DuocylinderShape4D);
		GDREGISTER_CLASS(GeneralShape4D);
		GDREGISTER_CLASS(GeneralShapeCurve4D);
		GDREGISTER_CLASS(HeightMapShape4D);
		GDREGISTER_CLASS(KinematicCollision4D);
		GDREGISTER_CLASS(OrthoplexShape4D);
		GDREGISTER_CLASS(PlaneShape4D);
		GDREGISTER_CLASS(RayShape4D);
		GDREGISTER_CLASS(RigidBody4D);
		GDREGISTER_CLASS(SphereShape4D);
		GDREGISTER_CLASS(StaticBody4D);
		// G4MF (register in dependency order).
		GDREGISTER_CLASS(G4MFItem4D);
		GDREGISTER_CLASS(G4MFBufferView4D);
		GDREGISTER_CLASS(G4MFAccessor4D);
		GDREGISTER_CLASS(G4MFFileReference4D);
		GDREGISTER_CLASS(G4MFTexture4D);
		GDREGISTER_CLASS(G4MFMaterialChannel4D);
		GDREGISTER_CLASS(G4MFMaterial4D);
		GDREGISTER_CLASS(G4MFMeshSurface4D);
		GDREGISTER_CLASS(G4MFMesh4D);
		GDREGISTER_CLASS(G4MFModel4D);
		GDREGISTER_CLASS(G4MFShape4D);
		GDREGISTER_CLASS(G4MFNodePhysics4D);
		GDREGISTER_CLASS(G4MFNodePhysicsMotion4D);
		GDREGISTER_CLASS(G4MFLight4D);
		GDREGISTER_CLASS(G4MFCamera4D);
		GDREGISTER_CLASS(G4MFNode4D);
		GDREGISTER_CLASS(G4MFState4D);
		GDREGISTER_CLASS(G4MFDocument4D);
#if GDEXTENSION
		GDREGISTER_CLASS(AxisAlignedBoxPhysicsEngine4D);
		GDREGISTER_CLASS(GhostPhysicsEngine4D);
		GDREGISTER_CLASS(WireframeRenderCanvas4D);
		GDREGISTER_CLASS(WireframeCanvasRenderingEngine4D);
		GDREGISTER_CLASS(CrossSectionRenderingEngine4D);
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
		rendering_server->register_rendering_engine(memnew(WireframeCanvasRenderingEngine4D));
		rendering_server->register_rendering_engine(memnew(CrossSectionRenderingEngine4D));
		add_godot_singleton("RenderingServer4D", rendering_server);
#ifdef TOOLS_ENABLED
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
#ifdef GDEXTENSION
		GDREGISTER_CLASS(EditorCamera4D);
		GDREGISTER_CLASS(EditorCameraSettings4D);
		GDREGISTER_CLASS(EditorCreate4DSceneButton);
		GDREGISTER_CLASS(EditorExportDialogG4MF4D);
		GDREGISTER_CLASS(EditorExportSettingsG4MF4D);
		GDREGISTER_CLASS(EditorImportPluginBase4D);
		GDREGISTER_CLASS(EditorImportPluginG4MFMesh4D);
		GDREGISTER_CLASS(EditorImportPluginG4MFScene4D);
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
#elif GODOT_MODULE
		EditorColorMap::add_conversion_color_pair("fff6a2", "ccc055");
		EditorColorMap::add_conversion_color_pair("fe5", "ba0");
		EditorColorMap::add_conversion_color_pair("fe7", "ba2");
		EditorColorMap::add_conversion_color_pair("fe9", "ba4");
		EditorColorMap::add_conversion_color_pair("fd0", "a90");
		EditorColorMap::add_conversion_color_pair("fd3", "a93");
		EditorColorMap::add_conversion_color_pair("dc3", "870");
		EditorColorMap::add_conversion_color_pair("ba3", "665d11");
#endif // GDEXTENSION or GODOT_MODULE
		EditorPlugins::add_by_type<Godot4DEditorPlugin>();
#endif // TOOLS_ENABLED
	}
}

void uninitialize_4d_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		remove_godot_singleton("Basis4D");
		remove_godot_singleton("Geometry4D");
		remove_godot_singleton("PhysicsServer4D");
		remove_godot_singleton("RenderingServer4D");
		remove_godot_singleton("Vector4D");
		remove_godot_singleton("WireMeshBuilder4D");
		memdelete(godot_4d_bind::Basis4D::get_singleton());
		memdelete(Geometry4D::get_singleton());
		memdelete(PhysicsServer4D::get_singleton());
		memdelete(RenderingServer4D::get_singleton());
		memdelete(Vector4D::get_singleton());
		memdelete(WireMeshBuilder4D::get_singleton());
	}
}
