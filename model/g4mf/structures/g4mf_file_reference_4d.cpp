#include "g4mf_file_reference_4d.h"

#include "../g4mf_state_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#elif GODOT_MODULE
#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#endif

PackedByteArray G4MFFileReference4D::read_file_data(const Ref<G4MFState4D> &p_g4mf_state) const {
	ERR_FAIL_COND_V(!p_g4mf_state.is_valid(), PackedByteArray());
	if (!_file_uri.is_empty()) {
		const String path = p_g4mf_state->get_g4mf_base_path().path_join(_file_uri);
		Ref<DirAccess> dir = DirAccess::open(p_g4mf_state->get_g4mf_base_path());
		if (dir.is_valid() && dir->file_exists(_file_uri)) {
			return FileAccess::get_file_as_bytes(path);
		} else {
			ERR_FAIL_V_MSG(PackedByteArray(), "G4MF import: Failed to load file data from URI '" + _file_uri + "' resolved to '" + path + "'. File does not exist.");
		}
	}
	if (_buffer_view_index >= 0) {
		TypedArray<G4MFBufferView4D> buffer_views = p_g4mf_state->get_g4mf_buffer_views();
		ERR_FAIL_INDEX_V(_buffer_view_index, buffer_views.size(), PackedByteArray());
		Ref<G4MFBufferView4D> buffer_view = buffer_views[_buffer_view_index];
		ERR_FAIL_COND_V(!buffer_view.is_valid(), PackedByteArray());
		return buffer_view->read_buffer_view_data(p_g4mf_state);
	}
	ERR_FAIL_V_MSG(PackedByteArray(), "G4MF import: Failed to load file data, file reference does not have a valid URI or buffer view.");
}

Error G4MFFileReference4D::create_missing_directories_if_needed(const Ref<G4MFState4D> &p_g4mf_state) const {
	ERR_FAIL_COND_V(!p_g4mf_state.is_valid(), ERR_INVALID_PARAMETER);
	if (_file_uri.is_empty()) {
		return OK; // Nothing to do.
	}
	const String relative_folder = _file_uri.get_base_dir();
	if (!relative_folder.is_empty()) {
		String base_path = p_g4mf_state->get_g4mf_base_path();
		Ref<DirAccess> dir = DirAccess::open(base_path);
		if (dir.is_null()) {
			base_path = ProjectSettings::get_singleton()->globalize_path(base_path);
			DirAccess::make_dir_recursive_absolute(base_path);
			dir = DirAccess::open(base_path);
		}
		ERR_FAIL_COND_V(dir.is_null(), ERR_FILE_CANT_OPEN);
		Error err = dir->make_dir_recursive(relative_folder);
		ERR_FAIL_COND_V(err != OK && err != ERR_ALREADY_EXISTS, err);
	}
	return OK;
}

Error G4MFFileReference4D::write_file_data(const Ref<G4MFState4D> &p_g4mf_state, const PackedByteArray &p_data, const int64_t p_alignment, const bool p_deduplicate, const int p_buffer_index) {
	ERR_FAIL_COND_V(!p_g4mf_state.is_valid(), ERR_INVALID_PARAMETER);
	if (!_file_uri.is_empty()) {
		Error err = create_missing_directories_if_needed(p_g4mf_state);
		ERR_FAIL_COND_V(err != OK, err);
		const String base_path = p_g4mf_state->get_g4mf_base_path();
		Ref<FileAccess> file = FileAccess::open(base_path.path_join(_file_uri), FileAccess::WRITE);
		ERR_FAIL_COND_V(file.is_null(), ERR_FILE_CANT_WRITE);
		file->store_buffer(p_data.ptr(), p_data.size());
		return OK;
	}
	// If there is no URI, write to a new buffer view.
	const int buffer_view_index = G4MFBufferView4D::write_new_buffer_view_into_state(p_g4mf_state, p_data, p_alignment, p_deduplicate, p_buffer_index);
	ERR_FAIL_COND_V_MSG(buffer_view_index < 0, ERR_CANT_CREATE, "G4MF export: Failed to write file data into new buffer view.");
	_buffer_view_index = buffer_view_index;
	return OK;
}

void G4MFFileReference4D::read_file_reference_entries_from_dictionary(const Dictionary &p_dict) {
	read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("bufferView")) {
		_buffer_view_index = p_dict["bufferView"];
	}
	if (p_dict.has("mimeType")) {
		_mime_type = p_dict["mimeType"];
	}
	if (p_dict.has("uri")) {
		_file_uri = p_dict["uri"];
	}
	// Parse the MIME type from the data URI if it wasn't explicitly provided.
	if (_mime_type.is_empty() && _file_uri.begins_with("data:")) {
		const int mime_type_end = _file_uri.find(";");
		if (mime_type_end != -1) {
			const int mime_type_start = _file_uri.find(":") + 1;
			if (mime_type_start != -1 && mime_type_start < mime_type_end) {
				_mime_type = _file_uri.substr(mime_type_start, mime_type_end - mime_type_start);
			}
		}
	}
}

Dictionary G4MFFileReference4D::write_file_reference_entries_to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_buffer_view_index >= 0) {
		dict["bufferView"] = _buffer_view_index;
	}
	if (!_mime_type.is_empty()) {
		dict["mimeType"] = _mime_type;
	}
	if (!_file_uri.is_empty()) {
		dict["uri"] = _file_uri;
	}
#if GODOT_MODULE && (GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 4))
	dict.sort();
#endif
	return dict;
}

void G4MFFileReference4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_buffer_view_index"), &G4MFFileReference4D::get_buffer_view_index);
	ClassDB::bind_method(D_METHOD("set_buffer_view_index", "name"), &G4MFFileReference4D::set_buffer_view_index);

	ClassDB::bind_method(D_METHOD("get_mime_type"), &G4MFFileReference4D::get_mime_type);
	ClassDB::bind_method(D_METHOD("set_mime_type", "mime_type"), &G4MFFileReference4D::set_mime_type);

	ClassDB::bind_method(D_METHOD("get_file_uri"), &G4MFFileReference4D::get_file_uri);
	ClassDB::bind_method(D_METHOD("set_file_uri", "file_uri"), &G4MFFileReference4D::set_file_uri);

	ClassDB::bind_method(D_METHOD("read_file_data", "g4mf_state"), &G4MFFileReference4D::read_file_data);
	ClassDB::bind_method(D_METHOD("create_missing_directories_if_needed", "g4mf_state"), &G4MFFileReference4D::create_missing_directories_if_needed);
	ClassDB::bind_method(D_METHOD("write_file_data", "g4mf_state", "data", "alignment", "deduplicate", "buffer_index"), &G4MFFileReference4D::write_file_data, DEFVAL(4), DEFVAL(true), DEFVAL(0));

	ClassDB::bind_method(D_METHOD("read_file_reference_entries_from_dictionary", "dict"), &G4MFFileReference4D::read_file_reference_entries_from_dictionary);
	ClassDB::bind_method(D_METHOD("write_file_reference_entries_to_dictionary"), &G4MFFileReference4D::write_file_reference_entries_to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "buffer_view_index"), "set_buffer_view_index", "get_buffer_view_index");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "mime_type"), "set_mime_type", "get_mime_type");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "file_uri"), "set_file_uri", "get_file_uri");
}
