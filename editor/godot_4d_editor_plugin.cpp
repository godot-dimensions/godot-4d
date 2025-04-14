#include "godot_4d_editor_plugin.h"

#include "../nodes/node_4d.h"
#include "icons/editor_4d_icons.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/window.hpp>
#elif GODOT_MODULE
#include "editor/editor_data.h"
#include "editor/editor_interface.h"
#include "scene/main/window.h"

#if VERSION_HEX < 0x040400
#define set_button_icon set_icon
#endif
#endif

Button *_find_button_by_text_4d(Node *p_start, const String &p_text) {
	Button *start_button = Object::cast_to<Button>(p_start);
	if (start_button != nullptr) {
		if (start_button->get_text() == p_text) {
			return start_button;
		}
	}
	for (int i = 0; i < p_start->get_child_count(); i++) {
		Button *button = _find_button_by_text_4d(p_start->get_child(i), p_text);
		if (button != nullptr) {
			return button;
		}
	}
	return nullptr;
}

void EditorCreate4DSceneButton::_notification(int p_what) {
	if (p_what == NOTIFICATION_THEME_CHANGED) {
		set_button_icon(_generate_editor_4d_icon("Node4D"));
	}
}

void Godot4DEditorPlugin::_add_4d_main_screen() {
	Control *godot_editor_main_screen = EditorInterface::get_singleton()->get_editor_main_screen();
	if (godot_editor_main_screen->has_node(NodePath("EditorMainScreen4D"))) {
		_main_screen = GET_NODE_TYPE(godot_editor_main_screen, EditorMainScreen4D, "EditorMainScreen4D");
		ERR_PRINT("EditorMainScreen4D already exists.");
	} else {
		_main_screen = memnew(EditorMainScreen4D);
		_main_screen->setup(get_undo_redo());
		godot_editor_main_screen->add_child(_main_screen);
	}
}

void Godot4DEditorPlugin::_move_4d_main_screen_tab_button() const {
	Control *editor = EditorInterface::get_singleton()->get_base_control();
	ERR_FAIL_NULL(editor);
	// Move 4D button to the left of the "Script" button, to the right of the "3D" button.
	Node *button_asset_lib_tab = editor->find_child("AssetLib", true, false);
	ERR_FAIL_NULL(button_asset_lib_tab);
	Node *main_editor_button_hbox = button_asset_lib_tab->get_parent();
	Button *button_4d_tab = GET_NODE_TYPE(main_editor_button_hbox, Button, "4D");
	Button *button_script_tab = GET_NODE_TYPE(main_editor_button_hbox, Button, "Script");
	ERR_FAIL_NULL(button_4d_tab);
	ERR_FAIL_NULL(button_script_tab);
	main_editor_button_hbox->move_child(button_4d_tab, button_script_tab->get_index());
}

void Godot4DEditorPlugin::_inject_4d_scene_button() {
	Control *editor = EditorInterface::get_singleton()->get_base_control();
	ERR_FAIL_NULL(editor);
	// Add a "4D Scene" button above the "User Interface" button, below the "3D Scene" button.
	Button *button_other_node_scene = _find_button_by_text_4d(editor, "Other Node");
	ERR_FAIL_NULL(button_other_node_scene);
	Control *beginner_node_shortcuts = Object::cast_to<Control>(button_other_node_scene->get_parent()->get_child(0));
	Button *user_interface_scene = _find_button_by_text_4d(beginner_node_shortcuts, "User Interface");
	ERR_FAIL_NULL(user_interface_scene);
	// Now that we know where to insert the button, create it.
	EditorCreate4DSceneButton *button_4d = memnew(EditorCreate4DSceneButton);
	button_4d->set_text(TTR("4D Scene"));
	button_4d->set_button_icon(_generate_editor_4d_icon("Node4D"));
	button_4d->connect(StringName("pressed"), callable_mp(this, &Godot4DEditorPlugin::_create_4d_scene));
	beginner_node_shortcuts->add_child(button_4d);
	beginner_node_shortcuts->move_child(button_4d, user_interface_scene->get_index());
}

void Godot4DEditorPlugin::_create_4d_scene() {
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	Node4D *new_node = memnew(Node4D);
	Node *editor_node = editor_interface->get_base_control()->get_parent();
	ERR_FAIL_NULL(editor_node);
	editor_node->call(StringName("set_edited_scene"), new_node);
	editor_interface->edit_node(new_node);
	EditorSelection *editor_selection = editor_interface->get_selection();
	editor_selection->clear();
	editor_selection->add_node(new_node);
}

void Godot4DEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_off_mesh_3d_importer.instantiate();
			_off_scene_importer.instantiate();
			_off_tetra_4d_importer.instantiate();
			_off_wire_4d_importer.instantiate();
			add_import_plugin(_off_mesh_3d_importer);
			add_import_plugin(_off_scene_importer);
			add_import_plugin(_off_tetra_4d_importer);
			add_import_plugin(_off_wire_4d_importer);
			_add_4d_main_screen();
			_move_4d_main_screen_tab_button();
			call_deferred(StringName("_inject_4d_scene_button"));
		} break;
		case NOTIFICATION_EXIT_TREE: {
			remove_import_plugin(_off_mesh_3d_importer);
			remove_import_plugin(_off_scene_importer);
			remove_import_plugin(_off_tetra_4d_importer);
			remove_import_plugin(_off_wire_4d_importer);
		} break;
	}
}

#if GDEXTENSION
Ref<Texture2D> Godot4DEditorPlugin::GDEXTMOD_GET_PLUGIN_ICON() const
#elif GODOT_MODULE
const Ref<Texture2D> Godot4DEditorPlugin::GDEXTMOD_GET_PLUGIN_ICON() const
#endif
{
	return _generate_editor_4d_icon("4D");
}

bool Godot4DEditorPlugin::GDEXTMOD_HANDLES(Object *p_object) const {
	return Object::cast_to<Node4D>(p_object) != nullptr;
}

void Godot4DEditorPlugin::GDEXTMOD_MAKE_VISIBLE(bool p_visible) {
	_main_screen->set_visible(p_visible);
}

void Godot4DEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_inject_4d_scene_button"), &Godot4DEditorPlugin::_inject_4d_scene_button);
}

Godot4DEditorPlugin::Godot4DEditorPlugin() {
	set_name(StringName("Godot4DEditorPlugin"));
}
