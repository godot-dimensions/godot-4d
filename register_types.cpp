#include "register_types.h"

#if GDEXTENSION
#include <godot_cpp/classes/engine.hpp>
#elif GODOT_MODULE
#include "core/config/engine.h"
#endif

#include "math/basis_4d_bind.h"
#include "math/euler_4d_bind.h"
#include "math/transform_4d_bind.h"
#include "math/vector_4d.h"
#include "nodes/node_4d.h"

#include "mesh/mesh_instance_4d.h"
#include "mesh/tetra/array_tetra_mesh_4d.h"
#include "mesh/tetra/box_tetra_mesh_4d.h"
#include "mesh/tetra/tetra_material_4d.h"
#include "mesh/wire/array_wire_mesh_4d.h"
#include "mesh/wire/box_wire_mesh_4d.h"
#include "mesh/wire/wire_material_4d.h"

#include "physics/collision_shape_4d.h"
#include "physics/shapes/box_shape_4d.h"
#include "physics/shapes/capsule_shape_4d.h"
#include "physics/shapes/cubinder_shape_4d.h"
#include "physics/shapes/cylinder_shape_4d.h"
#include "physics/shapes/duocylinder_shape_4d.h"
#include "physics/shapes/shape_4d.h"
#include "physics/shapes/sphere_shape_4d.h"

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
		GDREGISTER_CLASS(godot_4d_bind::Basis4D);
		GDREGISTER_CLASS(godot_4d_bind::Euler4D);
		GDREGISTER_CLASS(godot_4d_bind::Transform4D);
		GDREGISTER_CLASS(Vector4D);
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// General.
		GDREGISTER_CLASS(Node4D);
		add_godot_singleton("Basis4D", memnew(godot_4d_bind::Basis4D));
		add_godot_singleton("Vector4D", memnew(Vector4D));
		// Mesh.
		GDREGISTER_CLASS(ArrayTetraMesh4D);
		GDREGISTER_CLASS(ArrayWireMesh4D);
		GDREGISTER_CLASS(BoxTetraMesh4D);
		GDREGISTER_CLASS(BoxWireMesh4D);
		GDREGISTER_CLASS(MeshInstance4D);
		GDREGISTER_CLASS(TetraMaterial4D);
		GDREGISTER_CLASS(WireMaterial4D);
		GDREGISTER_VIRTUAL_CLASS(Material4D);
		GDREGISTER_VIRTUAL_CLASS(Mesh4D);
		GDREGISTER_VIRTUAL_CLASS(TetraMesh4D);
		GDREGISTER_VIRTUAL_CLASS(WireMesh4D);
		// Physics.
		GDREGISTER_CLASS(BoxShape4D);
		GDREGISTER_CLASS(CapsuleShape4D);
		GDREGISTER_CLASS(CollisionShape4D);
		GDREGISTER_CLASS(CubinderShape4D);
		GDREGISTER_CLASS(CylinderShape4D);
		GDREGISTER_CLASS(DuocylinderShape4D);
		GDREGISTER_CLASS(SphereShape4D);
		GDREGISTER_VIRTUAL_CLASS(Shape4D);
	}
}

void uninitialize_4d_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		remove_godot_singleton("Basis4D");
		remove_godot_singleton("Vector4D");
	}
}
