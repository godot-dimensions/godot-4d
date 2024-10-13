#include "godot_4d_editor_plugin.h"

#include "../nodes/node_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/window.hpp>
#elif GODOT_MODULE
#include "editor/editor_interface.h"
#include "scene/main/window.h"
#endif

void Godot4DEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			add_import_plugin(_off_mesh_3d_importer);
			add_import_plugin(_off_scene_importer);
			add_import_plugin(_off_tetra_4d_importer);
			add_import_plugin(_off_wire_4d_importer);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			remove_import_plugin(_off_mesh_3d_importer);
			remove_import_plugin(_off_scene_importer);
			remove_import_plugin(_off_tetra_4d_importer);
			remove_import_plugin(_off_wire_4d_importer);
		} break;
	}
}

const Ref<Texture2D> Godot4DEditorPlugin::GDEXTMOD_GET_PLUGIN_ICON() const {
	Window *window = Object::cast_to<Window>(get_viewport());
	ERR_FAIL_COND_V_MSG(window == nullptr, Ref<Texture2D>(), "Window is null.");
	return window->get_editor_theme_icon(StringName("4D"));
}

bool Godot4DEditorPlugin::GDEXTMOD_HANDLES(Object *p_object) const {
	return Object::cast_to<Node4D>(p_object) != nullptr;
}

void Godot4DEditorPlugin::GDEXTMOD_MAKE_VISIBLE(bool p_visible) {
	_main_screen->set_visible(p_visible);
}

Godot4DEditorPlugin::Godot4DEditorPlugin() {
	set_name(StringName("Godot4DEditorPlugin"));
	_off_mesh_3d_importer.instantiate();
	_off_scene_importer.instantiate();
	_off_tetra_4d_importer.instantiate();
	_off_wire_4d_importer.instantiate();
	_main_screen = memnew(EditorMainScreen4D);
	EditorInterface::get_singleton()->get_editor_main_screen()->add_child(_main_screen);
}
