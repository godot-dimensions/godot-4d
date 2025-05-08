#include "editor_export_dialog_g4mf_4d.h"

#include "../../../model/g4mf/g4mf_document_4d.h"
#include "../../../model/g4mf/g4mf_state_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_interface.hpp>
#elif GODOT_MODULE
#include "editor/editor_interface.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"
#endif

#if GDEXTENSION
#define ERR_FAIL_COND_SHOW_DIALOG(cond, msg) \
	if (cond) {                              \
		print_error(msg);                    \
		return;                              \
	}
#elif GODOT_MODULE
#define ERR_FAIL_COND_SHOW_DIALOG(cond, msg)                           \
	if (cond) {                                                        \
		print_error(TTR(msg));                                         \
		EditorNode::get_singleton()->show_accept(TTR(msg), TTR("OK")); \
		return;                                                        \
	}
#endif

void EditorExportDialogG4MF4D::_popup_g4mf_export_dialog() {
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);
	Node *scene_root = editor_interface->get_edited_scene_root();
	ERR_FAIL_COND_SHOW_DIALOG(scene_root == nullptr, "G4MF error: Cannot export scene without a root node.");
	// Set the file dialog's file name to the scene name.
	String filename = scene_root->get_scene_file_path().get_file().get_basename();
	if (filename.is_empty()) {
		filename = scene_root->get_name();
	}
	set_current_file(filename + ".g4tf");
#if GODOT_MODULE
	_settings_inspector->edit(nullptr);
	_settings_inspector->edit(_export_settings.ptr());
#endif // GODOT_MODULE
	// Show the file dialog.
	popup_centered_ratio();
}

void EditorExportDialogG4MF4D::_export_scene_as_g4mf(const String &p_path) {
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);
	Node *scene_root = editor_interface->get_edited_scene_root();
	ERR_FAIL_COND_SHOW_DIALOG(scene_root == nullptr, "G4MF error: Cannot export scene without a root node.");
	Ref<G4MFState4D> g4mf_state;
	g4mf_state.instantiate();
	Error err = _g4mf_document->export_append_from_godot_scene(g4mf_state, scene_root);
	ERR_FAIL_COND_SHOW_DIALOG(err != OK, "G4MF editor export: Error while running export_append_from_godot_scene.");
	err = _g4mf_document->export_write_to_file(g4mf_state, p_path);
	ERR_FAIL_COND_SHOW_DIALOG(err != OK, "G4MF editor export: Error while running export_write_to_file.");
}

void EditorExportDialogG4MF4D::setup(PopupMenu *p_export_menu) {
	_g4mf_document.instantiate();
	set_file_mode(EditorFileDialog::FILE_MODE_SAVE_FILE);
	set_access(EditorFileDialog::ACCESS_FILESYSTEM);
	clear_filters();
	add_filter("*.g4tf", "G4MF Text File");
	add_filter("*.g4b", "G4MF Binary File");
	set_title("Export 4D Scene as G4MF File");
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);
	connect("file_selected", callable_mp(this, &EditorExportDialogG4MF4D::_export_scene_as_g4mf));
	editor_interface->get_base_control()->add_child(this);
	// Set up the export settings menu.
	_export_settings.instantiate();
	_export_settings->setup(_g4mf_document);
	_settings_inspector = memnew(EditorInspector);
	_settings_inspector->set_custom_minimum_size(Size2(350, 300) * EDSCALE);
	add_side_menu(_settings_inspector, TTR("Export Settings:"));
	// Add a button to the Scene -> Export menu to pop up the settings dialog.
	const int index = p_export_menu->get_item_count();
	p_export_menu->add_item("4D Scene as G4MF...");
	p_export_menu->set_item_metadata(index, callable_mp(this, &EditorExportDialogG4MF4D::_popup_g4mf_export_dialog));
}
