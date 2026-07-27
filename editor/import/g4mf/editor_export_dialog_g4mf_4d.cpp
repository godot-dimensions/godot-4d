#include "editor_export_dialog_g4mf_4d.h"

#include "../../../model/g4mf/g4mf_document_4d.h"
#include "../../../model/g4mf/g4mf_state_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_file_system.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#define GDEXTMOD_GET_RESOURCE_FILESYSTEM get_resource_filesystem
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 5
#include "editor/editor_file_system.h"
#else
#include "editor/file_system/editor_file_system.h"
#endif
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 6
#define GDEXTMOD_GET_RESOURCE_FILESYSTEM get_resource_file_system
#else
#define GDEXTMOD_GET_RESOURCE_FILESYSTEM get_resource_filesystem
#endif
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
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 8
#define ERR_FAIL_COND_SHOW_DIALOG(cond, msg)                           \
	if (cond) {                                                        \
		print_error(TTR(msg));                                         \
		EditorNode::get_singleton()->show_accept(TTR(msg), TTR("OK")); \
		return;                                                        \
	}
#else
// Godot 4.8 replaces `show_accept` with `show_warning`.
// https://github.com/godotengine/godot/pull/119281
#define ERR_FAIL_COND_SHOW_DIALOG(cond, msg)                            \
	if (cond) {                                                         \
		print_error(TTR(msg));                                          \
		EditorNode::get_singleton()->show_warning(TTR(msg), TTR("OK")); \
		return;                                                         \
	}
#endif
#endif

void EditorExportDialogG4MF4D::_popup_g4mf_export_settings_dialog() {
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);
	Node *scene_root = editor_interface->get_edited_scene_root();
	ERR_FAIL_COND_SHOW_DIALOG(scene_root == nullptr, "G4MF error: Cannot export scene without a root node.");
	_export_settings->setup_for_scene(_g4mf_document, scene_root);
#if GODOT_MODULE
	// Force the inspector to refresh its display of the export settings by editing null first.
	_settings_inspector->edit(nullptr);
	_settings_inspector->edit(_export_settings.ptr());
#endif // GODOT_MODULE
	// Show the file dialog.
	_settings_dialog->popup_centered(Size2(450, 400) * EDSCALE);
}

void EditorExportDialogG4MF4D::_popup_g4mf_export_file_dialog() {
	_settings_dialog->hide();
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);
	Node *scene_root = editor_interface->get_edited_scene_root();
	ERR_FAIL_COND_SHOW_DIALOG(scene_root == nullptr, "G4MF error: Cannot export scene without a root node.");
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 2 && GODOT_VERSION_MINOR <= 5
	// Force the inspector to refresh its display of the export settings by editing null first.
	_settings_inspector_side_menu->edit(nullptr);
	_settings_inspector_side_menu->edit(_export_settings.ptr());
#endif
	// Set the file dialog's file name to the scene name.
	String filename = scene_root->get_scene_file_path().get_file().get_basename();
	if (filename.is_empty()) {
		filename = scene_root->get_name();
	}
	_file_dialog->set_current_file(filename + ".g4tf");
	_file_dialog->popup_centered_ratio(0.75f);
}

void EditorExportDialogG4MF4D::_export_scene_as_g4mf(const String &p_path) {
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);
	Node *scene_root = editor_interface->get_edited_scene_root();
	ERR_FAIL_COND_SHOW_DIALOG(scene_root == nullptr, "G4MF error: Cannot export scene without a root node.");
	Ref<G4MFState4D> g4mf_state;
	g4mf_state.instantiate();
	g4mf_state->set_external_data_mode(_export_settings->get_external_data_mode());
	Error err = _g4mf_document->export_append_from_godot_scene(g4mf_state, scene_root);
	ERR_FAIL_COND_SHOW_DIALOG(err != OK, "G4MF editor export: Error while running export_append_from_godot_scene.");
	err = _g4mf_document->export_repack_buffer_data(g4mf_state);
	ERR_FAIL_COND_SHOW_DIALOG(err != OK, "G4MF editor export: Error while running export_repack_buffer_data.");
	err = _g4mf_document->export_write_to_file(g4mf_state, p_path);
	ERR_FAIL_COND_SHOW_DIALOG(err != OK, "G4MF editor export: Error while running export_write_to_file.");
	// Refresh the editor file system to inform it of the new file.
	EditorFileSystem *efs = editor_interface->GDEXTMOD_GET_RESOURCE_FILESYSTEM();
	ERR_FAIL_NULL(efs);
	efs->scan();
}

void EditorExportDialogG4MF4D::setup(PopupMenu *p_export_menu) {
	EditorInterface *editor_interface = EditorInterface::get_singleton();
	ERR_FAIL_NULL(editor_interface);
	// Set up the file dialog.
	_file_dialog = memnew(EditorFileDialog);
	_file_dialog->set_min_size(Size2(500, 300) * EDSCALE);
	_file_dialog->set_file_mode(EditorFileDialog::FILE_MODE_SAVE_FILE);
	_file_dialog->set_access(EditorFileDialog::ACCESS_FILESYSTEM);
	_file_dialog->clear_filters();
	_file_dialog->add_filter("*.g4tf", "G4MF Text File");
	_file_dialog->add_filter("*.g4b", "G4MF Binary File");
	_file_dialog->set_title("Export 4D Scene as G4MF File");
	_file_dialog->connect("file_selected", callable_mp(this, &EditorExportDialogG4MF4D::_export_scene_as_g4mf));
	editor_interface->get_base_control()->add_child(_file_dialog);
	// Set up the export settings menu.
	_g4mf_document.instantiate();
	_export_settings.instantiate();
	_export_settings->setup_for_scene(_g4mf_document, nullptr);
	_settings_inspector = memnew(EditorInspector);
	_settings_inspector->set_custom_minimum_size(Size2(350, 200) * EDSCALE);
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 2 && GODOT_VERSION_MINOR <= 5
	_settings_inspector_side_menu = memnew(EditorInspector);
	_settings_inspector_side_menu->set_custom_minimum_size(Size2(350, 200) * EDSCALE);
	_file_dialog->add_side_menu(_settings_inspector_side_menu, TTR("Export Settings:"));
#endif
	// Set up the export settings dialog.
	_settings_dialog = memnew(ConfirmationDialog);
	_settings_dialog->set_min_size(Size2(400, 200) * EDSCALE);
	_settings_dialog->set_title("G4MF Export Settings");
	_settings_dialog->add_child(_settings_inspector);
	_settings_dialog->get_ok_button()->set_text("Select File...");
	_settings_dialog->connect("confirmed", callable_mp(this, &EditorExportDialogG4MF4D::_popup_g4mf_export_file_dialog));
	editor_interface->get_base_control()->add_child(_settings_dialog);
	// Add a button to the Scene -> Export menu to pop up the settings dialog.
	const int index = p_export_menu->get_item_count();
	p_export_menu->add_item("4D Scene as G4MF...");
	p_export_menu->set_item_metadata(index, callable_mp(this, &EditorExportDialogG4MF4D::_popup_g4mf_export_settings_dialog));
}

void EditorExportDialogG4MF4D::cleanup_export_dialog() {
	if (_file_dialog) {
		_file_dialog->queue_free();
		_file_dialog = nullptr;
	}
	if (_settings_dialog) {
		_settings_dialog->queue_free();
		_settings_dialog = nullptr;
	}
	if (_settings_inspector) {
		_settings_inspector->queue_free();
		_settings_inspector = nullptr;
	}
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 2 && GODOT_VERSION_MINOR <= 5
	if (_settings_inspector_side_menu) {
		_settings_inspector_side_menu->queue_free();
		_settings_inspector_side_menu = nullptr;
	}
#endif
	_g4mf_document.unref();
	_export_settings.unref();
}
