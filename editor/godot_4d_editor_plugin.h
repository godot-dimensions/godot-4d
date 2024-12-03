#pragma once

#include "off/editor_import_plugin_off_mesh_3d.h"
#include "off/editor_import_plugin_off_scene.h"
#include "off/editor_import_plugin_off_tetra_4d.h"
#include "off/editor_import_plugin_off_wire_4d.h"
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

#define GDEXTMOD_GET_PLUGIN_ICON get_icon
#define GDEXTMOD_GET_PLUGIN_NAME get_name
#define GDEXTMOD_HANDLES handles
#define GDEXTMOD_HAS_MAIN_SCREEN has_main_screen
#define GDEXTMOD_MAKE_VISIBLE make_visible
#endif

class Godot4DEditorPlugin : public EditorPlugin {
	GDCLASS(Godot4DEditorPlugin, EditorPlugin);
	Ref<EditorImportPluginOFFMesh3D> _off_mesh_3d_importer;
	Ref<EditorImportPluginOFFScene> _off_scene_importer;
	Ref<EditorImportPluginOFFTetra4D> _off_tetra_4d_importer;
	Ref<EditorImportPluginOFFWire4D> _off_wire_4d_importer;
	EditorMainScreen4D *_main_screen = nullptr;

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
