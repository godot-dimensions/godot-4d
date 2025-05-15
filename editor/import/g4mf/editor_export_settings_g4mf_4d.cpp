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

void EditorExportSettingsG4MF4D::setup(Ref<G4MFDocument4D> p_g4mf_document) {
	_g4mf_document = p_g4mf_document;
}

void EditorExportSettingsG4MF4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_compression_format"), &EditorExportSettingsG4MF4D::get_compression_format);
	ClassDB::bind_method(D_METHOD("set_compression_format", "compression_format"), &EditorExportSettingsG4MF4D::set_compression_format);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "compression_format", PROPERTY_HINT_ENUM, "None,Zstd"), "set_compression_format", "get_compression_format");

	ClassDB::bind_method(D_METHOD("setup", "g4mf_document"), &EditorExportSettingsG4MF4D::setup);
}
