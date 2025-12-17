#pragma once

#include "../../../model/g4mf/g4mf_document_4d.h"

class EditorExportSettingsG4MF4D : public RefCounted {
	GDCLASS(EditorExportSettingsG4MF4D, RefCounted);

	Ref<G4MFDocument4D> _g4mf_document;
	G4MFState4D::ExternalDataMode _external_data_mode = G4MFState4D::ExternalDataMode::EXTERNAL_DATA_MODE_AUTOMATIC;

protected:
	static void _bind_methods();

public:
	G4MFDocument4D::CompressionFormat get_compression_format() const;
	void set_compression_format(const G4MFDocument4D::CompressionFormat p_compression_format);

	G4MFState4D::ExternalDataMode get_external_data_mode() const { return _external_data_mode; }
	void set_external_data_mode(const G4MFState4D::ExternalDataMode p_external_data_mode) { _external_data_mode = p_external_data_mode; }

	int get_max_nested_scene_depth() const;
	void set_max_nested_scene_depth(const int p_max_nested_scene_depth);

	void setup(Ref<G4MFDocument4D> p_g4mf_document);
};
