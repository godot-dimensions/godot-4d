#include "register_types.h"

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
#include "model/mesh/material_4d.h"
#include "model/mesh/mesh_4d.h"
#include "model/mesh/poly/poly_mesh_4d.h"
#include "model/mesh/tetra/tetra_mesh_4d.h"
#include "model/mesh/wire/wire_mesh_4d.h"
#include "physics/shapes/shape_4d.h"

// Light.
#include "nodes/light/directional_light_4d.h"
#include "nodes/light/omni_light_4d.h"
#include "nodes/light/spot_light_4d.h"

// Model.
#include "model/g4mf/g4mf_document_4d.h"
#include "model/g4mf/structures/g4mf_model_4d.h"
#include "model/mesh/mesh_instance_4d.h"
#include "model/mesh/poly/array_poly_mesh_4d.h"
#include "model/mesh/poly/box_poly_mesh_4d.h"
#include "model/mesh/poly/orthoplex_poly_mesh_4d.h"
#include "model/mesh/poly/poly_material_4d.h"
#include "model/mesh/poly/poly_mesh_builder_4d.h"
#include "model/mesh/tetra/array_tetra_mesh_4d.h"
#include "model/mesh/tetra/box_tetra_mesh_4d.h"
#include "model/mesh/tetra/orthoplex_tetra_mesh_4d.h"
#include "model/mesh/tetra/tetra_material_4d.h"
#include "model/mesh/wire/array_wire_mesh_4d.h"
#include "model/mesh/wire/box_wire_mesh_4d.h"
#include "model/mesh/wire/orthoplex_wire_mesh_4d.h"
#include "model/mesh/wire/wire_material_4d.h"
#include "model/mesh/wire/wire_mesh_builder_4d.h"
#include "model/off/off_document_4d.h"

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
#include "render/3d/cross_section/cross_section_rendering_engine_4d.h"
#include "render/3d/godot_3d_rendering_engine_4d.h"
#include "render/rendering_engine_4d.h"
#include "render/rendering_server_4d.h"
#include "render/wireframe_canvas/wireframe_canvas_rendering_engine_4d.h"

// Environment.
#include "render/environment/cloud/volumetric_cloud_material_4d.h"
#include "render/environment/sky/gradient_sky_material_4d.h"
#include "render/environment/sky/physical_sky_material_4d.h"
#include "render/environment/sky/plain_sky_material_4d.h"
#include "render/environment/world_environment_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/engine.hpp>
// GDExtension has a nervous breakdown whenever singleton or casted classes are not registered.
// We don't need to register these in principle, and we don't need it for a module, just for GDExtension.
#include "render/environment/render_bridge_4d_to_3d.h"
#include "render/wireframe_canvas/wireframe_render_canvas_4d.h"
#ifdef TOOLS_ENABLED
#include "editor/import/off/editor_import_plugin_off_base.h"
#include "editor/import/off/editor_import_plugin_off_mesh_3d.h"
#include "editor/import/off/editor_import_plugin_off_poly_4d.h"
#include "editor/import/off/editor_import_plugin_off_scene.h"
#include "editor/import/off/editor_import_plugin_off_tetra_4d.h"
#include "editor/import/off/editor_import_plugin_off_wire_4d.h"
#include "editor/viewport/editor_camera_4d.h"
#include "editor/viewport/editor_camera_settings_4d.h"
#include "editor/viewport/editor_input_surface_4d.h"
#include "editor/viewport/editor_main_screen_4d.h"
#include "editor/viewport/editor_main_viewport_4d.h"
#include "editor/viewport/editor_preview_environment_4d.h"
#include "editor/viewport/editor_transform_gizmo_4d.h"
#include "editor/viewport/editor_transform_snap_settings_4d.h"
#include "editor/viewport/editor_viewport_rotation_4d.h"

#include <godot_cpp/classes/editor_plugin_registration.hpp>
#endif // TOOLS_ENABLED
#elif GODOT_MODULE
#include "core/config/engine.h"
#include "core/core_bind.h"
#ifdef TOOLS_ENABLED
#include "editor/plugins/editor_plugin.h"
#include "editor/themes/editor_color_map.h"
#endif // TOOLS_ENABLED
#endif

#ifdef TOOLS_ENABLED
#include "editor/godot_4d_editor_plugin.h"
#endif // TOOLS_ENABLED

inline void add_godot_singleton(const StringName &p_singleton_name, Object *p_object) {
	CoreBind::Engine::get_singleton()->register_singleton(p_singleton_name, p_object);
}

inline void remove_godot_singleton(const StringName &p_singleton_name, Object *p_object) {
	CoreBind::Engine::get_singleton()->unregister_singleton(p_singleton_name);
	if (p_object != nullptr) {
		memdelete(p_object);
	}
}

#if GDEXTENSION
// The extension declares `set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE)`,
// which is required to support reloading, but prevents using CORE or SERVERS initialization levels.
#define MODULE_INITIALIZATION_LEVEL_CORE_OR_EARLIEST MODULE_INITIALIZATION_LEVEL_SCENE
#elif GODOT_MODULE
// The module can use CORE or SERVERS initialization levels. In modules, we want to
// register as early as possible, so that other modules can depend on this module.
#define MODULE_INITIALIZATION_LEVEL_CORE_OR_EARLIEST MODULE_INITIALIZATION_LEVEL_CORE
#endif

void initialize_4d_module(ModuleInitializationLevel p_level) {
	// Classes MUST be registered in inheritance order, then dependency order.
	// When the inheritance and dependency doesn't matter, then alphabetical order is used.
	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE_OR_EARLIEST) {
		// Core math: must be first.
		GDREGISTER_CLASS(godot_4d_bind::Basis4D);
		GDREGISTER_CLASS(godot_4d_bind::Euler4D);
		GDREGISTER_CLASS(godot_4d_bind::Rotor4D);
		GDREGISTER_CLASS(godot_4d_bind::Transform4D);
		GDREGISTER_CLASS(Geometry4D);
		GDREGISTER_CLASS(Math4D);
		GDREGISTER_CLASS(Vector4D);
		add_godot_singleton("Basis4D", memnew(godot_4d_bind::Basis4D));
		add_godot_singleton("Geometry4D", memnew(Geometry4D));
		add_godot_singleton("Math4D", memnew(Math4D));
		add_godot_singleton("Vector4D", memnew(Vector4D));
		// Core physics.
		GDREGISTER_CLASS(RaycastParameters4D);
		GDREGISTER_VIRTUAL_CLASS(PhysicsEngine4D);
		GDREGISTER_CLASS(PhysicsServer4D);
		// Core render.
		GDREGISTER_VIRTUAL_CLASS(RenderingEngine4D);
		GDREGISTER_CLASS(RenderingServer4D);
		// General.
		GDREGISTER_CLASS(Node4D);
		GDREGISTER_CLASS(Camera4D);
		GDREGISTER_CLASS(QuadSplitContainer);
		// Virtual classes.
		GDREGISTER_VIRTUAL_CLASS(CollisionObject4D);
		GDREGISTER_VIRTUAL_CLASS(Material4D);
		GDREGISTER_VIRTUAL_CLASS(Mesh4D);
		GDREGISTER_VIRTUAL_CLASS(PhysicsBody4D);
		GDREGISTER_VIRTUAL_CLASS(Shape4D);
		GDREGISTER_VIRTUAL_CLASS(TetraMesh4D);
		GDREGISTER_VIRTUAL_CLASS(PolyMesh4D);
		GDREGISTER_VIRTUAL_CLASS(WireMesh4D);
		// Materials.
		GDREGISTER_CLASS(TetraMaterial4D);
		GDREGISTER_CLASS(PolyMaterial4D);
		GDREGISTER_CLASS(WireMaterial4D);
		// Light.
		GDREGISTER_ABSTRACT_CLASS(Light4D);
		GDREGISTER_CLASS(DirectionalLight4D);
		GDREGISTER_CLASS(OmniLight4D);
		GDREGISTER_CLASS(SpotLight4D);
		// Environment.
		GDREGISTER_VIRTUAL_CLASS(SkyMaterial4D);
		GDREGISTER_CLASS(GradientSkyMaterial4D);
		GDREGISTER_CLASS(PhysicalSkyMaterial4D);
		GDREGISTER_CLASS(PlainSkyMaterial4D);
		GDREGISTER_CLASS(VolumetricCloudMaterial4D);
		GDREGISTER_CLASS(WorldEnvironment4D);
		// Mesh.
		GDREGISTER_CLASS(ArrayPolyMesh4D);
		GDREGISTER_CLASS(ArrayTetraMesh4D);
		GDREGISTER_CLASS(ArrayWireMesh4D);
		GDREGISTER_CLASS(BoxPolyMesh4D);
		GDREGISTER_CLASS(BoxTetraMesh4D);
		GDREGISTER_CLASS(BoxWireMesh4D);
		GDREGISTER_CLASS(MeshInstance4D);
		GDREGISTER_CLASS(OrthoplexPolyMesh4D);
		GDREGISTER_CLASS(OrthoplexTetraMesh4D);
		GDREGISTER_CLASS(OrthoplexWireMesh4D);
		GDREGISTER_CLASS(PolyMeshBuilder4D);
		GDREGISTER_CLASS(WireMeshBuilder4D);
		add_godot_singleton("PolyMeshBuilder4D", memnew(PolyMeshBuilder4D));
		add_godot_singleton("WireMeshBuilder4D", memnew(WireMeshBuilder4D));
		// Depends on mesh.
		GDREGISTER_CLASS(Marker4D);
		GDREGISTER_CLASS(OFFDocument4D);
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
		// G4MF.
		GDREGISTER_CLASS(G4MFItem4D);
		GDREGISTER_CLASS(G4MFBufferView4D);
		GDREGISTER_CLASS(G4MFAccessor4D);
		GDREGISTER_CLASS(G4MFFileReference4D);
		GDREGISTER_CLASS(G4MFTexture4D);
		GDREGISTER_CLASS(G4MFMaterialChannel4D);
		GDREGISTER_CLASS(G4MFMaterial4D);
		GDREGISTER_CLASS(G4MFMeshSurface4D);
		GDREGISTER_CLASS(G4MFMeshSurfaceBinding4D);
		GDREGISTER_CLASS(G4MFMeshSurfaceBindingGeometry4D);
		GDREGISTER_CLASS(G4MFMesh4D);
		GDREGISTER_CLASS(G4MFMeshInstance4D);
		GDREGISTER_CLASS(G4MFModel4D);
		GDREGISTER_CLASS(G4MFModelInstance4D);
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
		GDREGISTER_ABSTRACT_CLASS(Godot3DRenderingEngine4D);
		// Must be registered before CrossSectionRenderingEngine4D, which owns and frees it.
		GDREGISTER_CLASS(EnvironmentRenderBridge4DTo3D);
		GDREGISTER_CLASS(CrossSectionRenderingEngine4D);
#endif // GDEXTENSION
		PhysicsServer4D *physics_server = memnew(PhysicsServer4D);
		physics_server->register_physics_engine("AxisAlignedBoxPhysicsEngine4D", memnew(AxisAlignedBoxPhysicsEngine4D));
		physics_server->register_physics_engine("GhostPhysicsEngine4D", memnew(GhostPhysicsEngine4D));
		add_godot_singleton("PhysicsServer4D", physics_server);
	}
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Render. This must be initialized after RenderingServer and RenderingDevice.
		RenderingServer4D *rendering_server = memnew(RenderingServer4D);
		Ref<WireframeCanvasRenderingEngine4D> wireframe_canvas_engine;
		wireframe_canvas_engine.instantiate();
		Ref<CrossSectionRenderingEngine4D> cross_section_engine;
		cross_section_engine.instantiate();
		// The order of registration determines the precedence for the "Automatic" rendering engine selection.
		// Wireframe is the default for now, but this will change after the other engines are feature-complete.
		rendering_server->register_rendering_engine(wireframe_canvas_engine);
		rendering_server->register_rendering_engine(cross_section_engine);
		add_godot_singleton("RenderingServer4D", rendering_server);
		// Material initialization.
#if GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR > 3)
		// In Godot 4.4+, preload the cross-section shaders. In Godot 4.3, lazy-load them when needed.
		WireMaterial4D::init_shaders();
		TetraMaterial4D::init_shaders();
		GradientSkyMaterial4D::init_shader();
		PhysicalSkyMaterial4D::init_shader();
		PlainSkyMaterial4D::init_shader();
		VolumetricCloudMaterial4D::init_shaders();
#endif
		// Initialize fallback materials in the opposite order from when they will later be destroyed.
		WireMesh4D::init_fallback_material();
		TetraMesh4D::init_fallback_material();
#ifdef TOOLS_ENABLED
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		PhysicsServer4D::get_singleton()->set_active(!Engine::get_singleton()->is_editor_hint());
#ifdef GDEXTENSION
		// Export and import.
		GDREGISTER_CLASS(EditorExportSettingsG4MF4D);
		GDREGISTER_CLASS(EditorExportDialogG4MF4D);
		GDREGISTER_CLASS(EditorImportPluginBase4D);
		GDREGISTER_CLASS(EditorImportPluginG4MFMesh4D);
		GDREGISTER_CLASS(EditorImportPluginG4MFScene4D);
		GDREGISTER_CLASS(EditorImportPluginOFFBase);
		GDREGISTER_CLASS(EditorImportPluginOFFMesh3D);
		GDREGISTER_CLASS(EditorImportPluginOFFPoly4D);
		GDREGISTER_CLASS(EditorImportPluginOFFScene);
		GDREGISTER_CLASS(EditorImportPluginOFFTetra4D);
		GDREGISTER_CLASS(EditorImportPluginOFFWire4D);
		// Pieces of the editor viewport.
		GDREGISTER_CLASS(EditorTransformSnapSettings4D);
		GDREGISTER_CLASS(EditorCameraSettings4D);
		GDREGISTER_CLASS(EditorCamera4D);
		GDREGISTER_CLASS(EditorCreate4DSceneButton);
		GDREGISTER_CLASS(EditorInputSurface4D);
		GDREGISTER_CLASS(EditorPreviewEnvironment4D);
		GDREGISTER_CLASS(EditorTransformGizmo4D);
		GDREGISTER_CLASS(EditorViewportRotation4D);
		// Main editor plugin classes.
		GDREGISTER_CLASS(EditorMainViewport4D);
		GDREGISTER_CLASS(EditorMainScreen4D);
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
		EditorColorMap::add_conversion_color_pair("b90", "614a00");
		EditorColorMap::add_conversion_color_pair("ba7", "6b5f3f");
		EditorColorMap::add_conversion_color_pair("982", "4a3f0d");
		EditorColorMap::add_conversion_color_pair("761", "2b2507");
#endif // GDEXTENSION or GODOT_MODULE
		EditorPlugins::add_by_type<Godot4DEditorPlugin>();
#endif // TOOLS_ENABLED
	}
}

void uninitialize_4d_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Clean up fallback materials and shaders in the opposite order of their creation.
		TetraMesh4D::cleanup_fallback_material();
		WireMesh4D::cleanup_fallback_material();
		VolumetricCloudMaterial4D::cleanup_shaders();
		PlainSkyMaterial4D::cleanup_shader();
		PhysicalSkyMaterial4D::cleanup_shader();
		GradientSkyMaterial4D::cleanup_shader();
		TetraMaterial4D::cleanup_shaders();
		WireMaterial4D::cleanup_shaders();
		// Clean up RenderingServer4D and its engines.
		RenderingServer4D *rendering_server = RenderingServer4D::get_singleton();
		rendering_server->unregister_all_rendering_engines();
		remove_godot_singleton("RenderingServer4D", rendering_server);
	}
	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE_OR_EARLIEST) {
		// Unregister and free the singletons in the opposite order of registration.
		remove_godot_singleton("PhysicsServer4D", PhysicsServer4D::get_singleton());
		remove_godot_singleton("WireMeshBuilder4D", WireMeshBuilder4D::get_singleton());
		remove_godot_singleton("PolyMeshBuilder4D", PolyMeshBuilder4D::get_singleton());
		remove_godot_singleton("Vector4D", Vector4D::get_singleton());
		remove_godot_singleton("Math4D", Math4D::get_singleton());
		remove_godot_singleton("Geometry4D", Geometry4D::get_singleton());
		remove_godot_singleton("Basis4D", godot_4d_bind::Basis4D::get_singleton());
	}
}
