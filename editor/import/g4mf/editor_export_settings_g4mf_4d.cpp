#include "editor_export_settings_g4mf_4d.h"

G4MFDocument4D::CompressionFormat EditorExportSettingsG4MF4D::get_compression_format() const {
	if (_g4mf_document.is_null()) {
		return G4MFDocument4D::COMPRESSION_FORMAT_NONE;
	}
	return _g4mf_document->get_compression_format();
}

void EditorExportSettingsG4MF4D::set_compression_format(const G4MFDocument4D::CompressionFormat p_compression_format) {
	ERR_FAIL_COND(_g4mf_document.is_null());
	_g4mf_document->set_compression_format(p_compression_format);
}

int EditorExportSettingsG4MF4D::get_max_nested_scene_depth() const {
	if (_g4mf_document.is_null()) {
		return -1;
	}
	return _g4mf_document->get_max_nested_scene_depth();
}

void EditorExportSettingsG4MF4D::set_max_nested_scene_depth(const int p_max_nested_scene_depth) {
	ERR_FAIL_COND(_g4mf_document.is_null());
	_g4mf_document->set_max_nested_scene_depth(p_max_nested_scene_depth);
}

void EditorExportSettingsG4MF4D::setup(Ref<G4MFDocument4D> p_g4mf_document) {
	_g4mf_document = p_g4mf_document;
}

void EditorExportSettingsG4MF4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_compression_format"), &EditorExportSettingsG4MF4D::get_compression_format);
	ClassDB::bind_method(D_METHOD("set_compression_format", "compression_format"), &EditorExportSettingsG4MF4D::set_compression_format);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "compression_format", PROPERTY_HINT_ENUM, "None,Zstd"), "set_compression_format", "get_compression_format");

	ClassDB::bind_method(D_METHOD("get_external_data_mode"), &EditorExportSettingsG4MF4D::get_external_data_mode);
	ClassDB::bind_method(D_METHOD("set_external_data_mode", "external_data_mode"), &EditorExportSettingsG4MF4D::set_external_data_mode);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "external_data_mode", PROPERTY_HINT_ENUM, "Automatic,Embed Everything,Separate All Files,Separate Binary Blobs,Separate Resource Files"), "set_external_data_mode", "get_external_data_mode");

	ClassDB::bind_method(D_METHOD("get_max_nested_scene_depth"), &EditorExportSettingsG4MF4D::get_max_nested_scene_depth);
	ClassDB::bind_method(D_METHOD("set_max_nested_scene_depth", "max_nested_scene_depth"), &EditorExportSettingsG4MF4D::set_max_nested_scene_depth);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "nested_scene_handling", PROPERTY_HINT_ENUM, "Allow Nested:-1,Merge into Single File:0,Merge into Flat Hierarchy:1"), "set_max_nested_scene_depth", "get_max_nested_scene_depth");

	ClassDB::bind_method(D_METHOD("setup", "g4mf_document"), &EditorExportSettingsG4MF4D::setup);
}
