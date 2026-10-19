#include "g4mf_file_reference_4d.h"

#include "../g4mf_state_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/marshalls.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#elif GODOT_MODULE
#include "core/config/project_settings.h"
#include "core/core_bind.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#endif

String G4MFFileReference4D::sanitize_file_extension(const String &p_extension, const bool p_print_warning) {
	String sanitized_ext = p_extension.to_lower();
	// Sanitize the extension by stripping all non-dot and non-lower-alphanumeric characters.
	// Valid examples which will not be modified: "txt", ".txt", "mesh.tres", "g4tf".
	for (int32_t char_index = 0; char_index < sanitized_ext.length(); char_index++) {
		uint32_t char_code = sanitized_ext[char_index];
		const bool valid = (char_code >= 'a' && char_code <= 'z') || (char_code >= '0' && char_code <= '9') || char_code == '.';
		if (!valid) {
			if (p_print_warning) {
				const String char_str = String::chr(char_code);
				WARN_PRINT(vformat("G4MFItem4D: The file extension '%s' contains a disallowed character '%s' (char code %d). Stripping it.", p_extension, char_str, char_code));
			}
			// `erase` is the same as `remove_at`, but the GDExtension API does not have `remove_at`.
			sanitized_ext = sanitized_ext.erase(char_index);
			char_index--; // Adjust index to account for removed character.
		}
	}
	// The last character cannot be a dot.
	const int length = sanitized_ext.length();
	if (length > 0) {
		if (sanitized_ext[length - 1] == '.') {
			if (p_print_warning) {
				WARN_PRINT(vformat("G4MFItem4D: The file extension '%s' ends with a disallowed character '.' (char code %d). Replacing with '_'.", p_extension, '.'));
			}
			sanitized_ext[length - 1] = '_';
		}
	}
	return sanitized_ext;
}

// NOTE: This is only intended for filesystem paths, including res:// and user://.
// Other protocols and data URIs must be preserved through set_file_uri().
String G4MFFileReference4D::sanitize_file_path_uri(const String &p_desired_file_path, const bool p_print_warning) {
	if (p_desired_file_path.is_empty()) {
		return String();
	}
	// Consider switching to `replace_char` in the future, but the char version is not available in older Godot versions.
	String sanitized_path = p_desired_file_path.replace("\\", "/");
	String path_prefix;
	// Keep roots outside simplify_path(), independently of its protocol support.
	if (sanitized_path.begins_with("res://") || sanitized_path.begins_with("user://")) {
		const int prefix_length = sanitized_path.find("://") + 3;
		path_prefix = sanitized_path.substr(0, prefix_length);
		sanitized_path = sanitized_path.substr(prefix_length);
	} else if (sanitized_path.length() >= 3 && sanitized_path[1] == ':' && sanitized_path[2] == '/' &&
			((sanitized_path[0] >= 'A' && sanitized_path[0] <= 'Z') || (sanitized_path[0] >= 'a' && sanitized_path[0] <= 'z'))) {
		path_prefix = sanitized_path.substr(0, 3);
		sanitized_path = sanitized_path.substr(3);
	} else {
		ERR_FAIL_COND_V_MSG(sanitized_path.contains("://") || sanitized_path.begins_with("data:"), String(), "G4MFFileReference4D: Expected a filesystem path, not a URI using another protocol.");
		if (sanitized_path.begins_with("//")) {
			// The server and share together form the root of a network path.
			const int server_end = sanitized_path.find("/", 2);
			ERR_FAIL_COND_V_MSG(server_end <= 2, String(), "G4MFFileReference4D: A network path must name a server, share, and file.");
			int share_start = server_end + 1;
			while (share_start < sanitized_path.length() && sanitized_path[share_start] == '/') {
				share_start++;
			}
			const int share_end = sanitized_path.find("/", share_start);
			ERR_FAIL_COND_V_MSG(share_end == -1, String(), "G4MFFileReference4D: A network path must name a file within its share.");
			const String share = sanitized_path.substr(share_start, share_end - share_start);
			ERR_FAIL_COND_V_MSG(share == "." || share == "..", String(), "G4MFFileReference4D: A network path must name a valid share.");
			path_prefix = sanitized_path.substr(0, server_end) + "/" + share + "/";
			sanitized_path = sanitized_path.substr(share_end + 1);
		} else if (sanitized_path.begins_with("/")) {
			path_prefix = "/";
			sanitized_path = sanitized_path.substr(1);
		}
	}
	if (!path_prefix.is_empty()) {
		while (sanitized_path.begins_with("/")) {
			sanitized_path = sanitized_path.substr(1);
		}
	}
	const String file_name = sanitized_path.get_file();
	ERR_FAIL_COND_V_MSG(file_name.is_empty() || file_name == "." || file_name == "..", String(), "G4MFFileReference4D: A file path must name a file, not a directory.");
	// Sanitize the name in-place by replacing characters which are problematic in file system path URIs.
	// Banned characters: ", #, *, :, |, ?, @, `, <, >, {, }, [, ], and literal %.
	// Remove ASCII and Unicode control characters (0-31 and 127-159 inclusive).
	for (int32_t char_index = 0; char_index < sanitized_path.length(); char_index++) {
		uint32_t char_code = sanitized_path[char_index];
		if ((/*char_code >= 0 &&*/ char_code <= 31) ||
				char_code == '"' || char_code == '#' || char_code == '*' || char_code == ':' ||
				char_code == '|' || char_code == '?' || char_code == '@' || char_code == '`' ||
				char_code == '<' || char_code == '>' || char_code == '{' || char_code == '}' ||
				char_code == '[' || char_code == ']' || char_code == '%' ||
				(char_code >= 127 && char_code <= 159)) {
			if (p_print_warning) {
				const String char_str = String::chr(char_code);
				WARN_PRINT(vformat("G4MFItem4D: The path '%s' contains a disallowed character '%s' (char code %d). Replacing with '_'.", p_desired_file_path, char_str, char_code));
			}
			sanitized_path[char_index] = '_';
		}
	}
	// The body cannot contain another protocol prefix now. Resolve dot segments before filename restrictions.
	sanitized_path = sanitized_path.simplify_path();
	ERR_FAIL_COND_V_MSG(!path_prefix.is_empty() && sanitized_path.begins_with("../"), String(), "G4MFFileReference4D: A file path cannot traverse above its root.");
	// Convert between digits so snake_case never inserts separators beside them.
	String snake_case_path;
	int start_index = 0;
	for (int char_index = 0; char_index < sanitized_path.length(); char_index++) {
		const uint32_t char_code = sanitized_path[char_index];
		if (char_code >= '0' && char_code <= '9') {
			snake_case_path += sanitized_path.substr(start_index, char_index - start_index).to_snake_case();
			snake_case_path += String::chr(char_code);
			start_index = char_index + 1;
		}
	}
	sanitized_path = snake_case_path + sanitized_path.substr(start_index).to_snake_case();
	// Special cases: Windows cannot contain certain reserved names. Ban names starting with these.
	// This check is a bit more aggressive than technically necessary, but that's fine.
	const PackedStringArray windows_reserved = PackedStringArray({ "CON", "PRN", "AUX", "NUL", "COM", "LPT" });
	// Read from `sanitized_upper` for insensitivity, but write to `sanitized_name`.
	const String sanitized_upper = sanitized_path.to_upper();
	for (int64_t i = 0; i < windows_reserved.size(); i++) {
		if (sanitized_upper.begins_with(windows_reserved[i])) {
			if (sanitized_upper.length() <= 3 || !(sanitized_upper[3] >= 'A' && sanitized_upper[3] <= 'Z')) {
				if (p_print_warning) {
					WARN_PRINT(vformat("G4MFItem4D: The path '%s' contains a reserved name on Windows. Replacing the first character with '_'.", p_desired_file_path));
				}
				sanitized_path[0] = '_';
			}
		}
		const String slash_prefixed = String("/") + windows_reserved[i];
		int64_t found_index = sanitized_upper.find(slash_prefixed);
		while (found_index != -1) {
			const int trailing_index = found_index + 4;
			if (sanitized_upper.length() <= trailing_index || !(sanitized_upper[trailing_index] >= 'A' && sanitized_upper[trailing_index] <= 'Z')) {
				if (p_print_warning) {
					WARN_PRINT(vformat("G4MFItem4D: The path '%s' contains a reserved name on Windows. Replacing a character with '_'.", p_desired_file_path));
				}
				sanitized_path[found_index + 1] = '_';
			}
			found_index = sanitized_upper.find(slash_prefixed, found_index + 1);
		}
	}
	// The last character cannot be a dot.
	const int length = sanitized_path.length();
	if (length > 0) {
		if (sanitized_path[length - 1] == '.') {
			if (p_print_warning) {
				WARN_PRINT(vformat("G4MFItem4D: The path '%s' ends with a disallowed character '.' (char code %d). Replacing with '_'.", p_desired_file_path, '.'));
			}
			sanitized_path[length - 1] = '_';
		}
	}
	return path_prefix + sanitized_path;
}

String G4MFFileReference4D::sanitize_path_uri(const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning, String &r_sanitized_ext) {
	r_sanitized_ext = sanitize_file_extension(p_extension, p_print_warning);
	if (!r_sanitized_ext.is_empty() && r_sanitized_ext[0] != '.') {
		r_sanitized_ext = String(".") + r_sanitized_ext;
	}
	// Normalize the complete filename so an empty stem can still name a file such as .png.
	return sanitize_file_path_uri(p_requested_path_stem + r_sanitized_ext, p_print_warning);
}

void G4MFFileReference4D::set_file_uri(const String &p_file_uri) {
	// General URIs use protocol-specific rules rather than filesystem sanitation.
	const bool sanitize = p_file_uri.begins_with("res://") || p_file_uri.begins_with("user://") ||
			(!p_file_uri.begins_with("data:") && !p_file_uri.contains("://"));
	if (sanitize) {
		const String sanitized_path = sanitize_file_path_uri(p_file_uri, true);
		if (!p_file_uri.is_empty() && sanitized_path.is_empty()) {
			return;
		}
		_file_uri = sanitized_path;
	} else {
		_file_uri = p_file_uri;
	}
}

void G4MFFileReference4D::set_file_uri_no_sanitize(const String &p_file_uri) {
	_file_uri = p_file_uri;
}

void G4MFFileReference4D::set_unique_file_path_uri(const Ref<G4MFState4D> &p_g4mf_state, const String &p_desired_path_stem, const String &p_file_ext, const bool p_print_warning) {
	ERR_FAIL_COND(p_g4mf_state.is_null());
	p_g4mf_state->assign_unique_path_uri(Ref<G4MFFileReference4D>(this), p_desired_path_stem, p_file_ext, p_print_warning);
}

void G4MFFileReference4D::set_unique_file_path_combined_uri(const Ref<G4MFState4D> &p_g4mf_state, const String &p_desired_file_uri, const bool p_print_warning) {
	ERR_FAIL_COND(p_g4mf_state.is_null());
	const String sanitized_path = sanitize_file_path_uri(p_desired_file_uri, p_print_warning);
	if (!p_desired_file_uri.is_empty() && sanitized_path.is_empty()) {
		return;
	}
	const String base_dir = sanitized_path.get_base_dir();
	const String file_name = sanitized_path.get_file();
	// Split the file name on the first dot to avoid breaking multi-part extensions with uniqueness.
	// For example, multiple "archive.tar.gz" should become "archive2.tar.gz" not "archive.tar2.gz".
	// This does mean that "Mr. Bob.txt" becomes "mr.bob.txt", but that's an acceptable compromise.
	const int first_dot = file_name.find(".");
	const String name_without_ext = first_dot == -1 ? file_name : file_name.substr(0, first_dot);
	const String file_ext = first_dot == -1 ? String() : file_name.substr(first_dot);
	const String path_without_ext = base_dir.path_join(name_without_ext);
	p_g4mf_state->assign_unique_path_uri(Ref<G4MFFileReference4D>(this), path_without_ext, file_ext, p_print_warning);
}

String G4MFFileReference4D::resolve_file_uri(const String &p_base_path) const {
	if (_file_uri.is_empty() || _file_uri.is_absolute_path()) {
		return _file_uri;
	}
	return p_base_path.path_join(_file_uri);
}

PackedByteArray G4MFFileReference4D::import_read_file_data(const Ref<G4MFState4D> &p_g4mf_state, const bool p_read_from_cache) {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), PackedByteArray());
	if (p_read_from_cache && !_file_data_cache.is_empty()) {
		return _file_data_cache;
	}
	if (!_file_uri.is_empty()) {
		if (_file_uri.begins_with("data:")) {
			const int base64_marker = _file_uri.find(";base64,");
			ERR_FAIL_COND_V_MSG(base64_marker == -1, PackedByteArray(), "G4MF import: Only base64-encoded data URIs are currently supported.");
			const String payload = _file_uri.substr(base64_marker + 8);
			_file_data_cache = CoreBind::Marshalls::get_singleton()->base64_to_raw(payload);
			return _file_data_cache;
		}
		const String path = resolve_file_uri(p_g4mf_state->get_g4mf_base_path());
		Ref<DirAccess> dir = DirAccess::open(path.get_base_dir());
		if (dir.is_valid() && dir->file_exists(path.get_file())) {
			_file_data_cache = FileAccess::get_file_as_bytes(path);
			return _file_data_cache;
		} else {
			ERR_FAIL_V_MSG(PackedByteArray(), "G4MF import: Failed to load file data from URI '" + _file_uri + "' resolved to '" + path + "'. File does not exist.");
		}
	}
	if (_buffer_view_index >= 0) {
		TypedArray<G4MFBufferView4D> buffer_views = p_g4mf_state->get_g4mf_buffer_views();
		ERR_FAIL_INDEX_V(_buffer_view_index, buffer_views.size(), PackedByteArray());
		Ref<G4MFBufferView4D> buffer_view = buffer_views[_buffer_view_index];
		ERR_FAIL_COND_V(buffer_view.is_null(), PackedByteArray());
		_file_data_cache = buffer_view->read_buffer_view_data(p_g4mf_state);
		return _file_data_cache;
	}
	ERR_FAIL_V_MSG(PackedByteArray(), "G4MF import: Failed to load file data, file reference does not have a valid URI or buffer view.");
}

Error G4MFFileReference4D::import_parse_file_data(const Ref<G4MFState4D> &p_g4mf_state) {
	// This should be overridden by derived classes if they wish to add more complex logic during import.
	Error result = OK;
	GDVIRTUAL_CALL(_import_parse_file_data, p_g4mf_state, result);
	return result;
}

Error G4MFFileReference4D::export_create_missing_directories_if_needed(const Ref<G4MFState4D> &p_g4mf_state) const {
	ERR_FAIL_COND_V(!p_g4mf_state.is_valid(), ERR_INVALID_PARAMETER);
	if (_file_uri.is_empty()) {
		return OK; // Nothing to do.
	}
	if (_file_uri.is_absolute_path()) {
		const String folder = resolve_file_uri(p_g4mf_state->get_g4mf_base_path()).get_base_dir();
		const Error err = DirAccess::make_dir_recursive_absolute(folder);
		ERR_FAIL_COND_V(err != OK && err != ERR_ALREADY_EXISTS, err);
		return OK;
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

void G4MFFileReference4D::export_assign_name_if_empty(const Ref<G4MFState4D> &p_g4mf_state) {
	ERR_FAIL_COND(p_g4mf_state.is_null());
	if (!get_item_name().is_empty()) {
		return;
	}
	String file_name;
	if (!_file_uri.begins_with("data:")) {
		file_name = _file_uri.get_basename().get_file();
	}
	if (file_name.is_empty()) {
		const int64_t index_of_self = p_g4mf_state->get_g4mf_file_references().find(this);
		file_name = "file" + String::num_int64(index_of_self);
	}
	// Generated filenames can contain characters which are disallowed in item names.
	file_name = sanitize_item_name(file_name, false);
	p_g4mf_state->assign_unique_item_name(Ref<G4MFFileReference4D>(this), file_name, false);
}

Error G4MFFileReference4D::_export_prepare_for_serialization_internal(const Ref<G4MFState4D> &p_g4mf_state, const bool p_deduplicate) {
	export_assign_name_if_empty(p_g4mf_state);
	return OK;
}

void G4MFFileReference4D::export_store_file_bytes(const PackedByteArray &p_data) {
	_file_data_cache = p_data;
}

Error G4MFFileReference4D::export_write_to_buffer(const Ref<G4MFState4D> &p_g4mf_state, const int64_t p_alignment, const bool p_deduplicate, const int p_buffer_index) {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), ERR_INVALID_PARAMETER);
	const int buffer_view_index = G4MFBufferView4D::write_new_buffer_view_into_state(p_g4mf_state, _file_data_cache, p_alignment, p_deduplicate, p_buffer_index);
	ERR_FAIL_COND_V_MSG(buffer_view_index < 0, ERR_CANT_CREATE, "G4MF export: Failed to write file data into new buffer view.");
	_buffer_view_index = buffer_view_index;
	set_unique_file_path_uri(p_g4mf_state, "", "");
	return OK;
}

Error G4MFFileReference4D::export_write_to_file(const Ref<G4MFState4D> &p_g4mf_state) {
	ERR_FAIL_COND_V(p_g4mf_state.is_null(), ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(_file_uri.is_empty(), ERR_INVALID_DATA);
	Error err = export_create_missing_directories_if_needed(p_g4mf_state);
	ERR_FAIL_COND_V(err != OK, err);
	const String base_path = p_g4mf_state->get_g4mf_base_path();
	Ref<FileAccess> file = FileAccess::open(resolve_file_uri(base_path), FileAccess::WRITE);
	ERR_FAIL_COND_V(file.is_null(), ERR_FILE_CANT_WRITE);
	file->store_buffer(_file_data_cache.ptr(), _file_data_cache.size());
	_buffer_view_index = -1;
	return OK;
}

Ref<G4MFFileReference4D> G4MFFileReference4D::file_reference_from_dictionary(const Dictionary &p_dict) {
	Ref<G4MFFileReference4D> file_ref;
	file_ref.instantiate();
	file_ref->read_file_reference_entries_from_dictionary(p_dict);
	return file_ref;
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
	ClassDB::bind_method(D_METHOD("set_buffer_view_index", "buffer_view_index"), &G4MFFileReference4D::set_buffer_view_index);

	ClassDB::bind_method(D_METHOD("get_mime_type"), &G4MFFileReference4D::get_mime_type);
	ClassDB::bind_method(D_METHOD("set_mime_type", "mime_type"), &G4MFFileReference4D::set_mime_type);

	ClassDB::bind_method(D_METHOD("get_file_uri"), &G4MFFileReference4D::get_file_uri);
	ClassDB::bind_method(D_METHOD("set_file_uri", "file_uri"), &G4MFFileReference4D::set_file_uri);
	ClassDB::bind_method(D_METHOD("resolve_file_uri", "base_path"), &G4MFFileReference4D::resolve_file_uri);

	ClassDB::bind_method(D_METHOD("import_read_file_data", "g4mf_state", "read_from_cache"), &G4MFFileReference4D::import_read_file_data, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("import_parse_file_data", "g4mf_state"), &G4MFFileReference4D::import_parse_file_data);
	GDVIRTUAL_BIND(_import_parse_file_data, "g4mf_state");

	ClassDB::bind_method(D_METHOD("export_create_missing_directories_if_needed", "g4mf_state"), &G4MFFileReference4D::export_create_missing_directories_if_needed);
	ClassDB::bind_method(D_METHOD("export_store_file_bytes", "data"), &G4MFFileReference4D::export_store_file_bytes);
	ClassDB::bind_method(D_METHOD("export_write_to_buffer", "g4mf_state", "alignment", "deduplicate", "buffer_index"), &G4MFFileReference4D::export_write_to_buffer, DEFVAL(4), DEFVAL(true), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("export_write_to_file", "g4mf_state"), &G4MFFileReference4D::export_write_to_file);

	ClassDB::bind_method(D_METHOD("read_file_reference_entries_from_dictionary", "dict"), &G4MFFileReference4D::read_file_reference_entries_from_dictionary);
	ClassDB::bind_method(D_METHOD("write_file_reference_entries_to_dictionary"), &G4MFFileReference4D::write_file_reference_entries_to_dictionary);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "buffer_view_index"), "set_buffer_view_index", "get_buffer_view_index");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "mime_type"), "set_mime_type", "get_mime_type");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "file_uri"), "set_file_uri", "get_file_uri");
}
