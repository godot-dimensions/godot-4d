#pragma once

#include "../../../godot_4d_defines.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_file_dialog.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#elif GODOT_MODULE
#include "editor/gui/editor_file_dialog.h"
#include "scene/gui/popup_menu.h"
#endif

class EditorExportDialogG4MF4D : public EditorFileDialog {
	GDCLASS(EditorExportDialogG4MF4D, EditorFileDialog);

	void _popup_g4mf_export_dialog();
	void _export_scene_as_g4mf(const String &p_path);

protected:
	static void _bind_methods() {}

public:
	void setup(PopupMenu *p_export_menu);
};
