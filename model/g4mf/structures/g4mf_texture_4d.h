#pragma once

#include "g4mf_file_reference_4d.h"

#if GDEXTENSION
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

class G4MFTexture4D : public G4MFItem4D {
	GDCLASS(G4MFTexture4D, G4MFItem4D);

	// Texture pixel data properties.
	TypedArray<G4MFFileReference4D> _images;
	PackedInt32Array _texture_size;
	Color _placeholder_color;

	// Texture sampler properties.
	String _sampler_mag_filter = "linear";
	String _sampler_min_filter = "linear";
	String _sampler_mipmap_filter = "linear";
	PackedStringArray _sampler_wrap_modes;

protected:
	static void _bind_methods();

public:
	// Texture pixel data properties.
	TypedArray<G4MFFileReference4D> get_images() const { return _images; }
	void set_images(const TypedArray<G4MFFileReference4D> &p_images) { _images = p_images; }

	PackedInt32Array get_texture_size() const { return _texture_size; }
	void set_texture_size(const PackedInt32Array &p_texture_size) { _texture_size = p_texture_size; }

	Color get_placeholder_color() const { return _placeholder_color; }
	void set_placeholder_color(const Color &p_placeholder_color) { _placeholder_color = p_placeholder_color; }

	// Texture sampler properties.
	String get_sampler_mag_filter() const { return _sampler_mag_filter; }
	void set_sampler_mag_filter(const String &p_sampler_mag_filter) { _sampler_mag_filter = p_sampler_mag_filter; }

	String get_sampler_min_filter() const { return _sampler_min_filter; }
	void set_sampler_min_filter(const String &p_sampler_min_filter) { _sampler_min_filter = p_sampler_min_filter; }

	String get_sampler_mipmap_filter() const { return _sampler_mipmap_filter; }
	void set_sampler_mipmap_filter(const String &p_sampler_mipmap_filter) { _sampler_mipmap_filter = p_sampler_mipmap_filter; }

	PackedStringArray get_sampler_wrap_modes() const { return _sampler_wrap_modes; }
	void set_sampler_wrap_modes(const PackedStringArray &p_sampler_wrap_modes) { _sampler_wrap_modes = p_sampler_wrap_modes; }

	static Ref<G4MFTexture4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
