#pragma once

#include "../../../model/g4mf/g4mf_document_4d.h"

class EditorExportSettingsG4MF4D : public RefCounted {
	GDCLASS(EditorExportSettingsG4MF4D, RefCounted);

	Ref<G4MFDocument4D> _g4mf_document;

protected:
	static void _bind_methods();

public:
	G4MFDocument4D::CompressionFormat get_compression_format() const;
	void set_compression_format(const G4MFDocument4D::CompressionFormat p_compression_format);

	void setup(Ref<G4MFDocument4D> p_g4mf_document);
};
