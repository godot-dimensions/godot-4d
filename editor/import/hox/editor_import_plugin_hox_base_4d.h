#pragma once

#include "../editor_import_plugin_base_4d.h"

class EditorImportPluginHoxBase4D : public EditorImportPluginBase4D {
	GDCLASS(EditorImportPluginHoxBase4D, EditorImportPluginBase4D);

protected:
	static void _bind_methods() {}

public:
	virtual int GDEXTMOD_GET_IMPORT_ORDER() const override { return 0; }
#if GDEXTENSION
	virtual PackedStringArray _get_recognized_extensions() const override {
		return PackedStringArray{ "hox" };
	}
#elif GODOT_MODULE
	virtual void get_recognized_extensions(List<String> *p_extensions) const override {
		p_extensions->push_back("hox");
	}
#endif
};
