#include "g4mf_texture_4d.h"

Ref<G4MFTexture4D> G4MFTexture4D::from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFTexture4D> texture;
	texture.instantiate();
	texture->read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("images")) {
		Array images_array = p_dict["images"];
		TypedArray<G4MFFileReference4D> images_parsed;
		images_parsed.resize(images_array.size());
		for (int i = 0; i < images_array.size(); i++) {
			Ref<G4MFFileReference4D> image_ref;
			image_ref->read_file_reference_entries_from_dictionary(images_array[i]);
			images_parsed[i] = image_ref;
		}
		texture->set_images(images_parsed);
	}
	if (p_dict.has("placeholder")) {
		texture->set_placeholder_color(G4MFItem4D::json_array_to_color(p_dict["placeholder"]));
	}
	if (p_dict.has("sampler")) {
		Dictionary sampler_dict = p_dict["sampler"];
		if (sampler_dict.has("magFilter")) {
			texture->set_sampler_mag_filter(sampler_dict["magFilter"]);
		}
		if (sampler_dict.has("minFilter")) {
			texture->set_sampler_min_filter(sampler_dict["minFilter"]);
		}
		if (sampler_dict.has("mipmapFilter")) {
			texture->set_sampler_mipmap_filter(sampler_dict["mipmapFilter"]);
		}
		PackedStringArray wrap_modes_parsed;
		if (sampler_dict.has("wrap")) {
			Array wrap_array = sampler_dict["wrap"];
			wrap_modes_parsed.resize(wrap_array.size());
			for (int i = 0; i < wrap_array.size(); i++) {
				wrap_modes_parsed.set(i, wrap_array[i]);
			}
		} else {
			// Default to repeat if no wrap modes are specified.
			wrap_modes_parsed.append("repeat");
		}
		texture->set_sampler_wrap_modes(wrap_modes_parsed);
	}
	if (p_dict.has("size")) {
		Array size_array = p_dict["size"];
		PackedInt32Array size_parsed;
		size_parsed.resize(size_array.size());
		for (int i = 0; i < size_array.size(); i++) {
			size_parsed.set(i, (int32_t)size_array[i]);
		}
		texture->set_texture_size(size_parsed);
	}
	return texture;
}

Dictionary G4MFTexture4D::to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (!_images.is_empty()) {
		Array images_array;
		images_array.resize(_images.size());
		for (int i = 0; i < _images.size(); i++) {
			Ref<G4MFFileReference4D> image_ref = _images[i];
			images_array[i] = image_ref->write_file_reference_entries_to_dictionary();
		}
		dict["images"] = images_array;
	}
	dict["placeholder"] = G4MFItem4D::color_to_json_array(_placeholder_color, false); // Required property, always write.
	{
		Dictionary sampler_dict;
		// Only write sampler properties if they are not empty and are not the default value.
		if (!_sampler_mag_filter.is_empty() && _sampler_mag_filter != "linear") {
			sampler_dict["magFilter"] = _sampler_mag_filter;
		}
		if (!_sampler_min_filter.is_empty() && _sampler_min_filter != "linear") {
			sampler_dict["minFilter"] = _sampler_min_filter;
		}
		if (!_sampler_mipmap_filter.is_empty() && _sampler_mipmap_filter != "linear") {
			sampler_dict["mipmapFilter"] = _sampler_mipmap_filter;
		}
		if (!_sampler_wrap_modes.is_empty() && !(_sampler_wrap_modes.size() == 1 && _sampler_wrap_modes[0] == "repeat")) {
			Array wrap_array;
			wrap_array.resize(_sampler_wrap_modes.size());
			for (int i = 0; i < _sampler_wrap_modes.size(); i++) {
				wrap_array[i] = _sampler_wrap_modes[i];
			}
			sampler_dict["wrap"] = wrap_array;
		}
		if (!sampler_dict.is_empty()) {
			dict["sampler"] = sampler_dict;
		}
	}
	dict["size"] = _texture_size; // Required property, always write.
#if GODOT_MODULE && (GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 4))
	dict.sort();
#endif
	return dict;
}

void G4MFTexture4D::_bind_methods() {
	// Texture pixel data properties.
	ClassDB::bind_method(D_METHOD("get_images"), &G4MFTexture4D::get_images);
	ClassDB::bind_method(D_METHOD("set_images", "images"), &G4MFTexture4D::set_images);
	ClassDB::bind_method(D_METHOD("get_placeholder_color"), &G4MFTexture4D::get_placeholder_color);
	ClassDB::bind_method(D_METHOD("set_placeholder_color", "placeholder_color"), &G4MFTexture4D::set_placeholder_color);
	ClassDB::bind_method(D_METHOD("get_texture_size"), &G4MFTexture4D::get_texture_size);
	ClassDB::bind_method(D_METHOD("set_texture_size", "texture_size"), &G4MFTexture4D::set_texture_size);

	// Texture sampler properties.
	ClassDB::bind_method(D_METHOD("get_sampler_mag_filter"), &G4MFTexture4D::get_sampler_mag_filter);
	ClassDB::bind_method(D_METHOD("set_sampler_mag_filter", "sampler_mag_filter"), &G4MFTexture4D::set_sampler_mag_filter);
	ClassDB::bind_method(D_METHOD("get_sampler_min_filter"), &G4MFTexture4D::get_sampler_min_filter);
	ClassDB::bind_method(D_METHOD("set_sampler_min_filter", "sampler_min_filter"), &G4MFTexture4D::set_sampler_min_filter);
	ClassDB::bind_method(D_METHOD("get_sampler_mipmap_filter"), &G4MFTexture4D::get_sampler_mipmap_filter);
	ClassDB::bind_method(D_METHOD("set_sampler_mipmap_filter", "sampler_mipmap_filter"), &G4MFTexture4D::set_sampler_mipmap_filter);
	ClassDB::bind_method(D_METHOD("get_sampler_wrap_modes"), &G4MFTexture4D::get_sampler_wrap_modes);
	ClassDB::bind_method(D_METHOD("set_sampler_wrap_modes", "sampler_wrap_modes"), &G4MFTexture4D::set_sampler_wrap_modes);

	ClassDB::bind_static_method("G4MFTexture4D", D_METHOD("from_dictionary", "dict"), &G4MFTexture4D::from_dictionary);
	ClassDB::bind_method(D_METHOD("to_dictionary"), &G4MFTexture4D::to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "images", PROPERTY_HINT_ARRAY_TYPE, "G4MFFileReference4D"), "set_images", "get_images");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "placeholder_color"), "set_placeholder_color", "get_placeholder_color");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "texture_size"), "set_texture_size", "get_texture_size");

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "mag_filter"), "set_sampler_mag_filter", "get_sampler_mag_filter");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "min_filter"), "set_sampler_min_filter", "get_sampler_min_filter");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "mipmap_filter"), "set_sampler_mipmap_filter", "get_sampler_mipmap_filter");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "wrap_modes"), "set_sampler_wrap_modes", "get_sampler_wrap_modes");
}
