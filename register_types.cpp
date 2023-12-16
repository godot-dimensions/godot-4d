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
		add_godot_singleton("Basis4D", memnew(godot_4d_bind::Basis4D));
		add_godot_singleton("Euler4D", memnew(godot_4d_bind::Euler4D));
		add_godot_singleton("Transform4D", memnew(godot_4d_bind::Transform4D));
		add_godot_singleton("Vector4D", memnew(Vector4D));
	}
}

void uninitialize_4d_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		remove_godot_singleton("Basis4D");
		remove_godot_singleton("Euler4D");
		remove_godot_singleton("Transform4D");
		remove_godot_singleton("Vector4D");
	}
}
