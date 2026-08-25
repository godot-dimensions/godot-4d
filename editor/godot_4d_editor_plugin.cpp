#include "godot_4d_editor_plugin.h"

#include "../nodes/node_4d.h"
#include "icons/editor_4d_icons.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/window.hpp>

#if GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 8)
// In Godot 4.8 and later, main screens are docks, so we need to interact with the dock system.
#include <godot_cpp/classes/editor_dock.hpp>
#endif
#elif GODOT_MODULE
#include "editor/editor_data.h"
#include "editor/editor_interface.h"
#include "scene/main/window.h"

#if GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 8)
// In Godot 4.8 and later, main screens are docks, so we need to interact with the dock system.
#include "editor/docks/editor_dock.h"
#endif

#if VERSION_HEX < 0x040400
#define set_button_icon set_icon
#endif
#endif

Button *Godot4DEditorPlugin::_find_button_by_text(Node *p_start, const String &p_text) {
	Button *start_button = Object::cast_to<Button>(p_start);
	if (start_button != nullptr) {
		if (start_button->get_text() == p_text) {
			return start_button;
		}
	}
	for (int i = 0; i < p_start->get_child_count(); i++) {
		Button *button = _find_button_by_text(p_start->get_child(i), p_text);
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

void Godot4DEditorPlugin::_remove_4d_main_screen() {
	if (_main_screen == nullptr) {
		return;
	}
	// The main screen is a child of Godot's editor main screen, not of this plugin,
	// so Godot will not free it for us, and we have to do so explicitly.
	Node *parent = _main_screen->get_parent();
	if (parent != nullptr) {
		parent->remove_child(_main_screen);
	}
	memdelete(_main_screen);
	_main_screen = nullptr;
}

void Godot4DEditorPlugin::_move_4d_main_screen_tab_button() const {
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 8
	// Prior to Godot 4.8, each main screen had a Button in an HBoxContainer in the title bar.
	Control *editor = EditorInterface::get_singleton()->get_base_control();
	ERR_FAIL_NULL(editor);
	// Move 4D button to the left of the "Script" button, to the right of the "3D" button.
	// The asset library / asset store button is probably the most unique node name to search for.
	Node *button_asset_lib_store_tab = editor->find_child("Asset Store", true, false);
	// The name changed from "AssetLib" to "Asset Store" in Godot 4.7.
	// However, a version check is not appropriate here, because a GDExtension may be
	// compiled for Godot 4.3-4.6 and used in 4.7, so we need to check for both names.
	if (button_asset_lib_store_tab == nullptr) {
		button_asset_lib_store_tab = editor->find_child("AssetLib", true, false);
	}
	ERR_FAIL_NULL(button_asset_lib_store_tab);
	Node *main_editor_button_hbox = button_asset_lib_store_tab->get_parent();
	// Check that the buttons exist before getting them, and bail out without printing
	// any errors if they don't. A GDExtension compiled for Godot 4.3-4.7 may be used in
	// Godot 4.8 or later, where main screens are docks instead of buttons in an HBoxContainer.
	if (!main_editor_button_hbox->has_node(NodePath("4D")) || !main_editor_button_hbox->has_node(NodePath("Script"))) {
		return;
	}
	Button *button_4d_tab = GET_NODE_TYPE(main_editor_button_hbox, Button, "4D");
	Button *button_script_tab = GET_NODE_TYPE(main_editor_button_hbox, Button, "Script");
	if (button_4d_tab == nullptr || button_script_tab == nullptr) {
		return;
	}
	main_editor_button_hbox->move_child(button_4d_tab, button_script_tab->get_index());
#else
	// In Godot 4.8 and later, each main screen is an EditorDock in the main screen's
	// DockTabContainer, and the tabs are ordered by the order of the dock nodes.
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);
	// Move 4D tab to the left of the "Script" tab, to the right of the "3D" tab.
	EditorDock *dock_4d = editor_interface->get_dock_by_name("4D");
	EditorDock *dock_script = editor_interface->get_dock_by_name("Script");
	ERR_FAIL_NULL(dock_4d);
	ERR_FAIL_NULL(dock_script);
	Node *main_screen_docks = dock_4d->get_parent();
	if (main_screen_docks == nullptr || main_screen_docks != dock_script->get_parent()) {
		// The docks are not side by side, perhaps because the user moved one of them
		// somewhere else or closed it, so there is no sensible way to reorder them.
		return;
	}
	// Block signals to avoid changing the selected main screen while the editor is still starting up.
	main_screen_docks->set_block_signals(true);
	main_screen_docks->move_child(dock_4d, dock_script->get_index(false));
	main_screen_docks->set_block_signals(false);
#endif
}

void Godot4DEditorPlugin::_inject_4d_scene_button() {
	if (_create_4d_scene_button != nullptr || !is_inside_tree()) {
		// Already injected, or this deferred call outlived the plugin being in the tree.
		return;
	}
	Control *editor = EditorInterface::get_singleton()->get_base_control();
	ERR_FAIL_NULL(editor);
	// Add a "4D Scene" button above the "User Interface" button, below the "3D Scene" button.
	Button *button_other_node_scene = _find_button_by_text(editor, "Other Node");
	ERR_FAIL_NULL(button_other_node_scene);
	Control *beginner_node_shortcuts = Object::cast_to<Control>(button_other_node_scene->get_parent()->get_child(0));
	Button *user_interface_scene = _find_button_by_text(beginner_node_shortcuts, "User Interface");
	ERR_FAIL_NULL(user_interface_scene);
	// Now that we know where to insert the button, create it.
	EditorCreate4DSceneButton *button_4d = memnew(EditorCreate4DSceneButton);
	button_4d->set_text(TTR("4D Scene"));
	button_4d->set_button_icon(_generate_editor_4d_icon("Node4D"));
	button_4d->connect(StringName("pressed"), callable_mp(this, &Godot4DEditorPlugin::_create_4d_scene));
	beginner_node_shortcuts->add_child(button_4d);
	beginner_node_shortcuts->move_child(button_4d, user_interface_scene->get_index());
	_create_4d_scene_button = button_4d;
}

void Godot4DEditorPlugin::_remove_4d_scene_button() {
	if (_create_4d_scene_button == nullptr) {
		return;
	}
	// Same as the main screen: this button was injected into a part of the editor
	// that Godot owns, so this plugin is responsible for taking it back out again.
	Node *parent = _create_4d_scene_button->get_parent();
	if (parent != nullptr) {
		parent->remove_child(_create_4d_scene_button);
	}
	memdelete(_create_4d_scene_button);
	_create_4d_scene_button = nullptr;
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
			_g4mf_mesh_4d_importer.instantiate();
			_g4mf_scene_4d_importer.instantiate();
			_off_mesh_3d_importer.instantiate();
			_off_scene_importer.instantiate();
			_off_poly_4d_importer.instantiate();
			_off_tetra_4d_importer.instantiate();
			_off_wire_4d_importer.instantiate();
			add_import_plugin(_g4mf_mesh_4d_importer);
			add_import_plugin(_g4mf_scene_4d_importer);
			add_import_plugin(_off_mesh_3d_importer);
			add_import_plugin(_off_scene_importer);
			add_import_plugin(_off_poly_4d_importer);
			add_import_plugin(_off_tetra_4d_importer);
			add_import_plugin(_off_wire_4d_importer);
			_add_4d_main_screen();
			_move_4d_main_screen_tab_button();
			_g4mf_export_dialog = memnew(EditorExportDialogG4MF4D);
			_g4mf_export_dialog->setup(get_export_as_menu());
			call_deferred(StringName("_inject_4d_scene_button"));
		} break;
		case NOTIFICATION_EXIT_TREE: {
			// Clean up in the opposite order of NOTIFICATION_ENTER_TREE.
			_remove_4d_scene_button();
			if (_g4mf_export_dialog != nullptr) {
				_g4mf_export_dialog->cleanup_export_dialog();
				memdelete(_g4mf_export_dialog);
				_g4mf_export_dialog = nullptr;
			}
			_remove_4d_main_screen();
			remove_import_plugin(_off_wire_4d_importer);
			remove_import_plugin(_off_tetra_4d_importer);
			remove_import_plugin(_off_poly_4d_importer);
			remove_import_plugin(_off_scene_importer);
			remove_import_plugin(_off_mesh_3d_importer);
			remove_import_plugin(_g4mf_scene_4d_importer);
			remove_import_plugin(_g4mf_mesh_4d_importer);
			_off_wire_4d_importer.unref();
			_off_tetra_4d_importer.unref();
			_off_poly_4d_importer.unref();
			_off_scene_importer.unref();
			_off_mesh_3d_importer.unref();
			_g4mf_scene_4d_importer.unref();
			_g4mf_mesh_4d_importer.unref();
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
	if (_main_screen != nullptr) {
		_main_screen->set_visible(p_visible);
	}
}

void Godot4DEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_inject_4d_scene_button"), &Godot4DEditorPlugin::_inject_4d_scene_button);
}

Godot4DEditorPlugin::Godot4DEditorPlugin() {
	set_name(StringName("Godot4DEditorPlugin"));
}
