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

class EditorExportDialogG4MF4D : public EditorFileDialog {
	GDCLASS(EditorExportDialogG4MF4D, EditorFileDialog);

	Ref<G4MFDocument4D> _g4mf_document;
	Ref<EditorExportSettingsG4MF4D> _export_settings;
	EditorInspector *_settings_inspector = nullptr;

	void _popup_g4mf_export_dialog();
	void _export_scene_as_g4mf(const String &p_path);

protected:
	static void _bind_methods() {}

public:
	void setup(PopupMenu *p_export_menu);
};
