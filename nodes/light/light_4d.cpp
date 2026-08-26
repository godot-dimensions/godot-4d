#include "light_4d.h"

#include "../../render/rendering_server_4d.h"

void Light4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			RenderingServer4D::get_singleton()->register_light(this);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			// The singleton is already gone if the module was uninitialized first.
			RenderingServer4D *rendering_server = RenderingServer4D::get_singleton();
			if (rendering_server != nullptr) {
				rendering_server->unregister_light(this);
			}
		} break;
	}
}

void Light4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_light_color"), &Light4D::get_light_color);
	ClassDB::bind_method(D_METHOD("set_light_color", "light_color"), &Light4D::set_light_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "light_color"), "set_light_color", "get_light_color");

	ClassDB::bind_method(D_METHOD("get_light_energy"), &Light4D::get_light_energy);
	ClassDB::bind_method(D_METHOD("set_light_energy", "light_energy"), &Light4D::set_light_energy);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "light_energy", PROPERTY_HINT_RANGE, "0,16,0.001,or_greater"), "set_light_energy", "get_light_energy");
}
