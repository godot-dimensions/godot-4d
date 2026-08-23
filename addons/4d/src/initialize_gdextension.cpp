#include "../../../register_types.h"

#include <gdextension_interface.h>

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT godot_4d_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_4d_module);
	init_obj.register_terminator(uninitialize_4d_module);
	// Setting this to SCENE enables reloading, but prevents using CORE or SERVERS initialization levels.
	// Reloading won't actually happen unless the user sets `reloadable = true` in the .gdextension file,
	// but still, the extension's code fully supports reloading if the user wants to enable it.
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
} // extern "C"
