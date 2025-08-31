#include "g4mf_file_reference_4d.h"

#include "../g4mf_state_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#elif GODOT_MODULE
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#endif

PackedByteArray G4MFFileReference4D::read_file_data(const Ref<G4MFState4D> &p_g4mf_state) const {
	ERR_FAIL_COND_V(!p_g4mf_state.is_valid(), PackedByteArray());
	if (!_uri.is_empty()) {
		const String path = p_g4mf_state->get_g4mf_base_path().path_join(_uri);
		Ref<DirAccess> dir = DirAccess::open(p_g4mf_state->get_g4mf_base_path());
		if (dir.is_valid() && dir->file_exists(_uri)) {
			return FileAccess::get_file_as_bytes(path);
		} else {
			ERR_FAIL_V_MSG(PackedByteArray(), vformat("G4MF import: Failed to load file data from URI '" + _uri + "' resolved to '" + path + "'. File does not exist.", path));
		}
	}
	if (_buffer_view >= 0) {
		TypedArray<G4MFBufferView4D> buffer_views = p_g4mf_state->get_g4mf_buffer_views();
		ERR_FAIL_INDEX_V(_buffer_view, buffer_views.size(), PackedByteArray());
		Ref<G4MFBufferView4D> buffer_view = buffer_views[_buffer_view];
		ERR_FAIL_COND_V(!buffer_view.is_valid(), PackedByteArray());
		return buffer_view->read_buffer_view_data(p_g4mf_state);
	}
	ERR_FAIL_V_MSG(PackedByteArray(), "G4MF import: Failed to load file data, file reference does not have a valid URI or buffer view.");
}

Error G4MFFileReference4D::write_file_data(const Ref<G4MFState4D> &p_g4mf_state, const PackedByteArray &p_data, int64_t p_alignment, bool p_deduplicate, int p_buffer_index) {
	ERR_FAIL_COND_V(!p_g4mf_state.is_valid(), ERR_INVALID_PARAMETER);
	if (!_uri.is_empty()) {
		const String base_path = p_g4mf_state->get_g4mf_base_path();
		Ref<DirAccess> dir = DirAccess::open(base_path);
		ERR_FAIL_COND_V(dir.is_null(), ERR_FILE_CANT_OPEN);
		const String relative_folder = _uri.get_base_dir();
		if (!relative_folder.is_empty()) {
			Error err = dir->make_dir_recursive(relative_folder);
			ERR_FAIL_COND_V(err != OK && err != ERR_ALREADY_EXISTS, err);
		}
		Ref<FileAccess> file = FileAccess::open(base_path.path_join(_uri), FileAccess::WRITE);
		ERR_FAIL_COND_V(file.is_null(), ERR_FILE_CANT_WRITE);
		file->store_buffer(p_data.ptr(), p_data.size());
		return OK;
	}
	// If there is no URI, write to a new buffer view.
	const int buffer_view_index = G4MFBufferView4D::write_new_buffer_view_into_state(p_g4mf_state, p_data, 1, true, 0);
	ERR_FAIL_COND_V_MSG(buffer_view_index < 0, ERR_CANT_CREATE, "G4MF export: Failed to write file data into new buffer view.");
	_buffer_view = buffer_view_index;
	return OK;
}

void G4MFFileReference4D::read_file_reference_entries_from_dictionary(const Dictionary &p_dict) {
	read_item_entries_from_dictionary(p_dict);
	if (p_dict.has("bufferView")) {
		_buffer_view = p_dict["bufferView"];
	}
	if (p_dict.has("mimeType")) {
		_mime_type = p_dict["mimeType"];
	}
	if (p_dict.has("uri")) {
		_uri = p_dict["uri"];
	}
}

Dictionary G4MFFileReference4D::write_file_reference_entries_to_dictionary() const {
	Dictionary dict = write_item_entries_to_dictionary();
	if (_buffer_view >= 0) {
		dict["bufferView"] = _buffer_view;
	}
	if (!_mime_type.is_empty()) {
		dict["mimeType"] = _mime_type;
	}
	if (!_uri.is_empty()) {
		dict["uri"] = _uri;
	}
#if GODOT_MODULE && (GODOT_VERSION_MAJOR > 4 || (GODOT_VERSION_MAJOR == 4 && GODOT_VERSION_MINOR >= 4))
	dict.sort();
#endif
	return dict;
}

void G4MFFileReference4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_buffer_view"), &G4MFFileReference4D::get_buffer_view);
	ClassDB::bind_method(D_METHOD("set_buffer_view", "name"), &G4MFFileReference4D::set_buffer_view);

	ClassDB::bind_method(D_METHOD("get_mime_type"), &G4MFFileReference4D::get_mime_type);
	ClassDB::bind_method(D_METHOD("set_mime_type", "mime_type"), &G4MFFileReference4D::set_mime_type);

	ClassDB::bind_method(D_METHOD("get_uri"), &G4MFFileReference4D::get_uri);
	ClassDB::bind_method(D_METHOD("set_uri", "uri"), &G4MFFileReference4D::set_uri);

	ClassDB::bind_method(D_METHOD("read_file_data", "g4mf_state"), &G4MFFileReference4D::read_file_data);
	ClassDB::bind_method(D_METHOD("write_file_data", "g4mf_state", "data", "alignment", "deduplicate", "buffer_index"), &G4MFFileReference4D::write_file_data, DEFVAL(1), DEFVAL(true), DEFVAL(0));

	ClassDB::bind_method(D_METHOD("read_file_reference_entries_from_dictionary", "dict"), &G4MFFileReference4D::read_file_reference_entries_from_dictionary);
	ClassDB::bind_method(D_METHOD("write_file_reference_entries_to_dictionary"), &G4MFFileReference4D::write_file_reference_entries_to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "buffer_view"), "set_buffer_view", "get_buffer_view");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "mime_type"), "set_mime_type", "get_mime_type");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "uri"), "set_uri", "get_uri");
}
