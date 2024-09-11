#pragma once

#include "../../mesh/off/off_document.h"

#if GDEXTENSION
#include <godot_cpp/classes/editor_import_plugin.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#define GDEXTMOD_GET_IMPORT_ORDER _get_import_order
#define GDEXTMOD_GET_IMPORTER_NAME _get_importer_name
#define GDEXTMOD_GET_PRESET_COUNT _get_preset_count
#define GDEXTMOD_GET_PRESET_NAME _get_preset_name
#define GDEXTMOD_GET_PRIORITY _get_priority
#define GDEXTMOD_GET_RESOURCE_TYPE _get_resource_type
#define GDEXTMOD_GET_SAVE_EXTENSION _get_save_extension
#define GDEXTMOD_GET_VISIBLE_NAME _get_visible_name
#elif GODOT_MODULE
#include "editor/import/editor_import_plugin.h"

#define GDEXTMOD_GET_IMPORT_ORDER get_import_order
#define GDEXTMOD_GET_IMPORTER_NAME get_importer_name
#define GDEXTMOD_GET_PRESET_COUNT get_preset_count
#define GDEXTMOD_GET_PRESET_NAME get_preset_name
#define GDEXTMOD_GET_PRIORITY get_priority
#define GDEXTMOD_GET_RESOURCE_TYPE get_resource_type
#define GDEXTMOD_GET_SAVE_EXTENSION get_save_extension
#define GDEXTMOD_GET_VISIBLE_NAME get_visible_name
#endif

class EditorImportPluginOFFBase : public EditorImportPlugin {
	GDCLASS(EditorImportPluginOFFBase, EditorImportPlugin);

public:
	virtual int GDEXTMOD_GET_IMPORT_ORDER() const override { return 0; }
	virtual int GDEXTMOD_GET_PRESET_COUNT() const override { return 0; }
	virtual String GDEXTMOD_GET_PRESET_NAME(int p_idx) const override { return String("Default"); }
	virtual String GDEXTMOD_GET_SAVE_EXTENSION() const override { return "res"; }
#if GDEXTENSION
	virtual bool _get_option_visibility(const String &p_path, const StringName &p_option_name, const Dictionary &p_options) const override {
		return true;
	}
	virtual PackedStringArray _get_recognized_extensions() const override {
		return PackedStringArray{ "off" };
	}
#elif GODOT_MODULE
	virtual bool get_option_visibility(const String &p_path, const String &p_option, const HashMap<StringName, Variant> &p_options) const override {
		return true;
	}
	virtual void get_recognized_extensions(List<String> *p_extensions) const override {
		p_extensions->push_back("off");
	}
#endif
};
