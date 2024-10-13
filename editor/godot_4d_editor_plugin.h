#pragma once

#if GDEXTENSION
#include <godot_cpp/classes/editor_plugin.hpp>

#define GDEXTMOD_GET_PLUGIN_NAME _get_plugin_name
#define GDEXTMOD_HANDLES _handles
#define GDEXTMOD_HAS_MAIN_SCREEN _has_main_screen
#define GDEXTMOD_MAKE_VISIBLE _make_visible
#elif GODOT_MODULE
#include "editor/plugins/editor_plugin.h"

#define GDEXTMOD_GET_PLUGIN_NAME get_name
#define GDEXTMOD_HANDLES handles
#define GDEXTMOD_HAS_MAIN_SCREEN has_main_screen
#define GDEXTMOD_MAKE_VISIBLE make_visible
#endif

#include "off/editor_import_plugin_off_mesh_3d.h"
#include "off/editor_import_plugin_off_scene.h"
#include "off/editor_import_plugin_off_tetra_4d.h"
#include "off/editor_import_plugin_off_wire_4d.h"
#include "viewport/editor_main_screen_4d.h"

class Godot4DEditorPlugin : public EditorPlugin {
	GDCLASS(Godot4DEditorPlugin, EditorPlugin);
	Ref<EditorImportPluginOFFMesh3D> _off_mesh_3d_importer;
	Ref<EditorImportPluginOFFScene> _off_scene_importer;
	Ref<EditorImportPluginOFFTetra4D> _off_tetra_4d_importer;
	Ref<EditorImportPluginOFFWire4D> _off_wire_4d_importer;
	EditorMainScreen4D *_main_screen = nullptr;

protected:
	static void _bind_methods() {}
	void _notification(int p_what);

public:
	virtual String GDEXTMOD_GET_PLUGIN_NAME() const override { return "4D"; }
	virtual bool GDEXTMOD_HANDLES(Object *p_object) const override;
	virtual bool GDEXTMOD_HAS_MAIN_SCREEN() const override { return true; }
	virtual void GDEXTMOD_MAKE_VISIBLE(bool p_visible) override;

	Godot4DEditorPlugin();
};
