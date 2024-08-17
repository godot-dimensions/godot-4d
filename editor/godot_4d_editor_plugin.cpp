#include "godot_4d_editor_plugin.h"

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

void Godot4DEditorPlugin::GDEXTMOD_MAKE_VISIBLE(bool p_visible) {
	if (p_visible) {
		//print_line("Godot4DEditorPlugin: Make visible");
	} else {
		//print_line("Godot4DEditorPlugin: Make invisible");
	}
}

Godot4DEditorPlugin::Godot4DEditorPlugin() {
	_off_mesh_3d_importer.instantiate();
	_off_scene_importer.instantiate();
	_off_tetra_4d_importer.instantiate();
	_off_wire_4d_importer.instantiate();
}
