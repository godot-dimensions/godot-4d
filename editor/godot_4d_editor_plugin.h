#pragma once

#include "import/g4mf/editor_export_dialog_g4mf_4d.h"
#include "import/g4mf/editor_import_plugin_g4mf_mesh_4d.h"
#include "import/g4mf/editor_import_plugin_g4mf_scene_4d.h"
#include "import/off/editor_import_plugin_off_mesh_3d.h"
#include "import/off/editor_import_plugin_off_scene.h"
#include "import/off/editor_import_plugin_off_tetra_4d.h"
#include "import/off/editor_import_plugin_off_wire_4d.h"
#include "viewport/editor_main_screen_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_plugin.hpp>

#define GDEXTMOD_GET_PLUGIN_ICON _get_plugin_icon
#define GDEXTMOD_GET_PLUGIN_NAME _get_plugin_name
#define GDEXTMOD_HANDLES _handles
#define GDEXTMOD_HAS_MAIN_SCREEN _has_main_screen
#define GDEXTMOD_MAKE_VISIBLE _make_visible
#elif GODOT_MODULE
#include "editor/plugins/editor_plugin.h"

#if VERSION_HEX < 0x040400
#define GDEXTMOD_GET_PLUGIN_ICON get_icon
#define GDEXTMOD_GET_PLUGIN_NAME get_name
#else
#define GDEXTMOD_GET_PLUGIN_ICON get_plugin_icon
#define GDEXTMOD_GET_PLUGIN_NAME get_plugin_name
#endif
#define GDEXTMOD_HANDLES handles
#define GDEXTMOD_HAS_MAIN_SCREEN has_main_screen
#define GDEXTMOD_MAKE_VISIBLE make_visible
#endif

class EditorCreate4DSceneButton : public Button {
	GDCLASS(EditorCreate4DSceneButton, Button);

protected:
	static void _bind_methods() {}
	void _notification(int p_what);
};

class Godot4DEditorPlugin : public EditorPlugin {
	GDCLASS(Godot4DEditorPlugin, EditorPlugin);

	Ref<EditorImportPluginG4MFMesh4D> _g4mf_mesh_4d_importer;
	Ref<EditorImportPluginG4MFScene4D> _g4mf_scene_4d_importer;
	Ref<EditorImportPluginOFFMesh3D> _off_mesh_3d_importer;
	Ref<EditorImportPluginOFFScene> _off_scene_importer;
	Ref<EditorImportPluginOFFTetra4D> _off_tetra_4d_importer;
	Ref<EditorImportPluginOFFWire4D> _off_wire_4d_importer;
	EditorExportDialogG4MF4D *_g4mf_export_dialog = nullptr;
	EditorMainScreen4D *_main_screen = nullptr;

	void _add_4d_main_screen();
	void _move_4d_main_screen_tab_button() const;
	void _inject_4d_scene_button();
	void _create_4d_scene();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
#if GDEXTENSION
	virtual Ref<Texture2D> GDEXTMOD_GET_PLUGIN_ICON() const override;
#elif GODOT_MODULE
	virtual const Ref<Texture2D> GDEXTMOD_GET_PLUGIN_ICON() const override;
#endif
	virtual String GDEXTMOD_GET_PLUGIN_NAME() const override { return "4D"; }
	virtual bool GDEXTMOD_HANDLES(Object *p_object) const override;
	virtual bool GDEXTMOD_HAS_MAIN_SCREEN() const override { return true; }
	virtual void GDEXTMOD_MAKE_VISIBLE(bool p_visible) override;

	Godot4DEditorPlugin();
};
