#pragma once

#include "../../../godot_4d_defines.h"
#include "editor_export_settings_g4mf_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_file_dialog.hpp>
#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#elif GODOT_MODULE
#if GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR < 5
#include "editor/editor_inspector.h"
#else
#include "editor/inspector/editor_inspector.h"
#endif
#include "editor/gui/editor_file_dialog.h"
#include "scene/gui/popup_menu.h"
#endif

#if GODOT_MODULE && (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 2 && GODOT_VERSION_MINOR <= 5)
// Added in https://github.com/godotengine/godot/pull/79313 for Godot 4.2.
// Removed in https://github.com/godotengine/godot/pull/111162 for Godot 4.6.
// Only enable in modules to avoid issues when loading old GDExtensions in new Godot versions.
#define USE_EDITOR_FILE_DIALOG_SIDE_MENU 1
#endif

// Not actually a dialog, but manages two of them: the settings dialog and the file dialog.
class EditorExportDialogG4MF4D : public Object {
	GDCLASS(EditorExportDialogG4MF4D, Object);

	Ref<G4MFDocument4D> _g4mf_document;
	Ref<EditorExportSettingsG4MF4D> _export_settings;
	EditorInspector *_settings_inspector = nullptr;
#if USE_EDITOR_FILE_DIALOG_SIDE_MENU
	EditorInspector *_settings_inspector_side_menu = nullptr;
#endif

	ConfirmationDialog *_settings_dialog = nullptr;
	EditorFileDialog *_file_dialog = nullptr;
	PopupMenu *_export_menu = nullptr;

	void _popup_g4mf_export_settings_dialog();
	void _popup_g4mf_export_file_dialog();
	void _export_scene_as_g4mf(const String &p_path);

protected:
	static void _bind_methods() {}

public:
	void setup(PopupMenu *p_export_menu);
	void cleanup_export_dialog();
};
