#include "g4mf_path_context_4d.h"

#include "../g4mf_file_reference_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/project_settings.hpp>
#elif GODOT_MODULE
#include "core/config/project_settings.h"
#endif

String G4MFPathContext4D::_normalize_abs_path(const String &p_abs_path) {
	// An absent source has no reusable identity. Relative paths must be resolved by the caller against an explicit base.
	if (p_abs_path.is_empty()) {
		return String();
	}
	String path = p_abs_path.replace("\\", "/");
	ERR_FAIL_COND_V_MSG(!path.is_absolute_path(), String(), "G4MF: Expected an absolute path, including res:// or user://: " + p_abs_path);
	const String globalized_path = ProjectSettings::get_singleton()->globalize_path(path).replace("\\", "/");
	// Keep a rooted URI when Godot has no filesystem mapping for it.
	if (globalized_path.is_absolute_path()) {
		path = globalized_path;
	}
	// Keep protocols outside simplify_path(), independently of its protocol support.
	const int protocol_separator = path.find("://");
	if (protocol_separator != -1) {
		const int path_start = protocol_separator + 3;
		path = path.substr(0, path_start) + path.substr(path_start).simplify_path();
	} else {
		path = path.simplify_path();
	}
	return path;
}

String G4MFPathContext4D::_get_path_root(const String &p_abs_path) {
	const int protocol_separator = p_abs_path.find("://");
	if (protocol_separator != -1) {
		return p_abs_path.substr(0, protocol_separator + 3);
	}
	if (p_abs_path.begins_with("//")) {
		// A network share is a root: `..` cannot cross into another share on the same server.
		const int server_end = p_abs_path.find("/", 2);
		const int share_end = server_end == -1 ? -1 : p_abs_path.find("/", server_end + 1);
		return share_end == -1 ? p_abs_path : p_abs_path.substr(0, share_end + 1);
	}
	if (p_abs_path.length() >= 3 && p_abs_path[1] == ':' && p_abs_path[2] == '/') {
		return p_abs_path.substr(0, 3);
	}
	return p_abs_path.begins_with("/") ? String("/") : String();
}

String G4MFPathContext4D::_get_relative_path(const String &p_referencing_abs_path, const String &p_target_abs_path, const bool p_use_updir_dots) {
	const String referencing_root = _get_path_root(p_referencing_abs_path);
	const String target_root = _get_path_root(p_target_abs_path);
	if (referencing_root.nocasecmp_to(target_root) != 0) {
		// References to another root must stay absolute. Source-derived placement instead uses only the filename.
		return p_use_updir_dots ? p_target_abs_path : p_target_abs_path.get_file();
	}
	// Root spelling is case-insensitive, but directory spelling must remain correct on case-sensitive filesystems.
	PackedStringArray referencing_parts = p_referencing_abs_path.substr(referencing_root.length()).split("/");
	PackedStringArray target_parts = p_target_abs_path.substr(target_root.length()).split("/");
	// Only the referencing file's directory can be a common prefix, even when the filenames match.
	int max_possible_length = MIN(referencing_parts.size() - 1, target_parts.size());
	int common_length = 0;
	while (common_length < max_possible_length && referencing_parts[common_length] == target_parts[common_length]) {
		common_length++;
	}
	PackedStringArray relative_parts;
	if (p_use_updir_dots) {
		const int referencing_base_dir_length = referencing_parts.size() - 1;
		for (int i = common_length; i < referencing_base_dir_length; i++) {
			relative_parts.append("..");
		}
	}
	for (int i = common_length; i < target_parts.size(); i++) {
		relative_parts.append(target_parts[i]);
	}
	return String("/").join(relative_parts);
}

String G4MFPathContext4D::get_source_placement_path(const String &p_source_abs_path, const String &p_referencing_source_abs_path) {
	// Choose export placement from the source reference's directory relationship, omitting parent-directory segments.
	// For example, cars/car.tscn referenced by houses/main.tscn is placed at cars/car.tscn, not ../cars/car.tscn.
	const String source_path = _normalize_abs_path(p_source_abs_path);
	if (source_path.is_empty()) {
		return String();
	}
	const String referencing_source_abs_path = _normalize_abs_path(p_referencing_source_abs_path);
	ERR_FAIL_COND_V(!p_referencing_source_abs_path.is_empty() && referencing_source_abs_path.is_empty(), String());
	return _get_relative_path(referencing_source_abs_path, source_path, false);
}

Pair<String, String> G4MFPathContext4D::_get_path_stem_and_extension(const String &p_path) {
	const String base_dir = p_path.get_base_dir();
	const String file_name = p_path.get_file();
	// Split the file name on the first dot to avoid breaking multi-part extensions with uniqueness.
	// For example, multiple "archive.tar.gz" should become "archive2.tar.gz" not "archive.tar2.gz".
	// This does mean that "Mr. Bob.txt" becomes "mr.bob.txt", but that's an acceptable compromise.
	const int first_dot = file_name.find(".");
	const String name_without_ext = first_dot == -1 ? file_name : file_name.substr(0, first_dot);
	const String file_ext = first_dot == -1 ? String() : file_name.substr(first_dot);
	return Pair<String, String>(base_dir.path_join(name_without_ext), file_ext);
}

String G4MFPathContext4D::_reserve_unique_abs_path(const String &p_requested_path_uri, const bool p_print_warning, const String &p_base_abs_dir) {
	const Pair<String, String> path_parts = _get_path_stem_and_extension(p_requested_path_uri);
	String sanitized_ext;
	String sanitized_path = G4MFFileReference4D::sanitize_file_path_uri_from_stem(path_parts.first, path_parts.second, p_print_warning, sanitized_ext);
	if (sanitized_path.is_empty()) {
		return String();
	}
	// Sanitize the generated portion before adding the base directory, whose spelling belongs to the caller.
	if (!p_base_abs_dir.is_empty()) {
		sanitized_path = _normalize_abs_path(p_base_abs_dir.path_join(sanitized_path));
		ERR_FAIL_COND_V(sanitized_path.is_empty(), String());
	}
	return _reserve_sanitized_unique_path_uri(sanitized_path.trim_suffix(sanitized_ext), sanitized_ext, p_print_warning);
}

String G4MFPathContext4D::_reserve_sanitized_unique_path_uri(const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning, const String &p_base_abs_dir) {
	String unique_path_uri = p_requested_path_stem + p_extension;
	if (unique_path_uri.is_empty()) {
		return String();
	}
	String lower_unique_path_uri = _normalize_abs_path(p_base_abs_dir.path_join(unique_path_uri)).to_lower();
	ERR_FAIL_COND_V(lower_unique_path_uri.is_empty(), String());
	uint64_t discriminator = 2;
	bool found_any_conflict = false;
	// Filesystem path URIs are always treated case-insensitively to avoid conflicts.
	while (_lowercase_reserved_paths.has(lower_unique_path_uri) || _lowercase_owned_paths.has(lower_unique_path_uri)) {
		// Attempt a new unique name by appending a discriminator.
		unique_path_uri = p_requested_path_stem + String::num_uint64(discriminator) + p_extension;
		lower_unique_path_uri = _normalize_abs_path(p_base_abs_dir.path_join(unique_path_uri)).to_lower();
		discriminator++;
		found_any_conflict = true;
	}
	_lowercase_reserved_paths.insert(lower_unique_path_uri);
	if (p_print_warning && found_any_conflict) {
		WARN_PRINT("G4MFState4D: The requested path URI " + p_requested_path_stem + p_extension + " is already in use. The path URI " + unique_path_uri + " will be used instead.");
	}
	return unique_path_uri;
}

void G4MFPathContext4D::assign_unique_path_uri_from_stem(const Ref<G4MFFileReference4D> &p_file_reference, const String &p_requested_path_stem, const String &p_extension, const String &p_base_abs_dir, const bool p_print_warning) {
	ERR_FAIL_COND(p_file_reference.is_null());
	String sanitized_ext;
	const String sanitized_path = G4MFFileReference4D::sanitize_file_path_uri_from_stem(p_requested_path_stem, p_extension, p_print_warning, sanitized_ext);
	if ((!p_requested_path_stem.is_empty() || !p_extension.is_empty()) && sanitized_path.is_empty()) {
		return;
	}
	const String base_dir = sanitized_path.is_relative_path() ? p_base_abs_dir : String();
	ERR_FAIL_COND_MSG(!sanitized_path.is_empty() && sanitized_path.is_relative_path() && !base_dir.is_absolute_path(), "G4MF: A relative path requires an absolute base directory.");
	// Validate before changing either the reservation or URI. Store only the ID, not a reference to the item.
	const uint64_t owner_id = p_file_reference->get_instance_id();
	const String *previous_path = _owner_ids_to_paths.getptr(owner_id);
	if (previous_path != nullptr) {
		_lowercase_owned_paths.erase(*previous_path);
		_owner_ids_to_paths.erase(owner_id);
	}
	const String unique_path = _reserve_sanitized_unique_path_uri(sanitized_path.trim_suffix(sanitized_ext), sanitized_ext, p_print_warning, base_dir);
	if (!unique_path.is_empty()) {
		const String key = _normalize_abs_path(base_dir.path_join(unique_path)).to_lower();
		// Transfer this newly allocated reservation to the owner. Other reservations stay independent.
		_lowercase_reserved_paths.erase(key);
		_lowercase_owned_paths.insert(key);
		_owner_ids_to_paths[owner_id] = key;
	}
	p_file_reference->set_file_uri_no_sanitize(unique_path);
}

void G4MFPathContext4D::assign_unique_path_uri(const Ref<G4MFFileReference4D> &p_file_reference, const String &p_requested_path_uri, const String &p_base_abs_dir, const bool p_print_warning) {
	ERR_FAIL_COND(p_file_reference.is_null());
	const String sanitized_path = G4MFFileReference4D::sanitize_file_path_uri(p_requested_path_uri, p_print_warning);
	if (!p_requested_path_uri.is_empty() && sanitized_path.is_empty()) {
		return;
	}
	const Pair<String, String> path_parts = _get_path_stem_and_extension(sanitized_path);
	assign_unique_path_uri_from_stem(p_file_reference, path_parts.first, path_parts.second, p_base_abs_dir, p_print_warning);
}

String G4MFPathContext4D::reserve_unique_path_uri_from_stem(const String &p_requested_path_stem, const String &p_extension, const String &p_base_abs_dir, const bool p_print_warning) {
	String sanitized_ext;
	const String sanitized_path = G4MFFileReference4D::sanitize_file_path_uri_from_stem(p_requested_path_stem, p_extension, p_print_warning, sanitized_ext);
	if (sanitized_path.is_empty()) {
		return String();
	}
	// Resolve relative destinations only for collision checks, preserving the returned URI and the caller's base directory.
	const String base_dir = sanitized_path.is_relative_path() ? p_base_abs_dir : String();
	return _reserve_sanitized_unique_path_uri(sanitized_path.trim_suffix(sanitized_ext), sanitized_ext, p_print_warning, base_dir);
}

void G4MFPathContext4D::mark_path_as_reserved(const String &p_abs_path) {
	// Only record the normalized location as reserved; no alternative path is allocated.
	const String absolute_path = _normalize_abs_path(p_abs_path);
	if (!absolute_path.is_empty()) {
		_lowercase_reserved_paths.insert(absolute_path.to_lower());
	}
}

// TL;DR: This function essentially answers this inquiry:
// "For a given source file and usage context, I am proposing storing the exported version in this location. What path should I actually use?"
//
// Reserve a destination for the source and usage identity, reusing an existing allocation when present.
// "For source" describes the identity; the reserved and returned path is an output destination.
// When both source and usage are empty, each call allocates a new unique destination.
// An empty desired destination can only retrieve an existing allocation.
String G4MFPathContext4D::reserve_destination_abs_path_for_source(const String &p_source_abs_path, const String &p_usage_context, const String &p_desired_dest_abs_path, const bool p_print_warning) {
	ERR_FAIL_COND_V(!p_desired_dest_abs_path.is_empty() && !p_desired_dest_abs_path.is_absolute_path(), String());
	const String source_abs_path = _normalize_abs_path(p_source_abs_path);
	ERR_FAIL_COND_V(!p_source_abs_path.is_empty() && source_abs_path.is_empty(), String());
	const PathUsageKey key = PathUsageKey(source_abs_path, p_usage_context);
	const bool empty_key = source_abs_path.is_empty() && p_usage_context.is_empty();
	// If we already have a determined destination path for this source and usage context,
	// reuse that same path so that the export does not end up with duplicate files.
	if (!empty_key && _source_and_usage_to_dest.has(key)) {
		return _source_and_usage_to_dest[key];
	}
	// If this is a new source and/or usage context, determine a new destination path.
	ERR_FAIL_COND_V(p_desired_dest_abs_path.is_empty(), String());
	const String unique_dest_path = _reserve_unique_abs_path(p_desired_dest_abs_path, p_print_warning);
	if (unique_dest_path.is_empty()) {
		return String();
	}
	if (!empty_key) {
		_source_and_usage_to_dest[key] = unique_dest_path;
	}
	return unique_dest_path;
}

// TL;DR: This function essentially answers this inquiry:
// "For a given source file, used at this location, with this context, I want to use an exported
// version of it from `p_referencing_destination_abs_path`. What relative path should I actually use?"
//
// Reserve a destination for the source and usage identity, then express it relative to the referencing output file.
// The referencing source file's directory relationship supplies preferred placement when no destination is requested.
// Referencing describes a file that points to the dependency; the dependency need not be embedded in that file.
// The returned destination remains absolute when it is on another drive or network share.
// When both source and usage are empty, each call allocates a new unique destination.
String G4MFPathContext4D::reserve_destination_path_for_source(const String &p_source_abs_path, const String &p_referencing_source_abs_path, const String &p_usage_context, const String &p_referencing_destination_abs_path, const String &p_desired_dest_path, const bool p_print_warning) {
	const String source_abs_path = _normalize_abs_path(p_source_abs_path);
	ERR_FAIL_COND_V(!p_source_abs_path.is_empty() && source_abs_path.is_empty(), String());
	const String referencing_destination_abs_path = _normalize_abs_path(p_referencing_destination_abs_path);
	ERR_FAIL_COND_V(referencing_destination_abs_path.is_empty(), String());
	const PathUsageKey key = PathUsageKey(source_abs_path, p_usage_context);
	const bool empty_key = source_abs_path.is_empty() && p_usage_context.is_empty();
	// If we already have a determined destination path for this source and usage context,
	// reuse that same path so that the export does not end up with duplicate files.
	if (!empty_key && _source_and_usage_to_dest.has(key)) {
		const String dest_abs = _normalize_abs_path(_source_and_usage_to_dest[key]);
		return _get_relative_path(referencing_destination_abs_path, dest_abs, true);
	}
	// If this is a new source and/or usage context, determine a new destination path.
	String relative_path;
	if (p_desired_dest_path.is_empty()) {
		const String referencing_source_abs_path = _normalize_abs_path(p_referencing_source_abs_path);
		ERR_FAIL_COND_V_MSG(source_abs_path.is_empty() || referencing_source_abs_path.is_empty(), String(), "G4MFPathContext4D: At least one of source paths or desired destination path must be non-empty.");
		// No destination path was desired, so it will be determined based on the relative path.
		// If `/a/b/c.tscn` references `/a/b/models/tree.tscn`, place the export at `models/tree.tscn`.
		relative_path = _get_relative_path(referencing_source_abs_path, source_abs_path, false);
	} else {
		// An explicit desired destination path was requested.
		if (p_desired_dest_path.is_absolute_path()) {
			// Example: `/a/b/textures/wood.png`.
			relative_path = _get_relative_path(referencing_destination_abs_path, _normalize_abs_path(p_desired_dest_path), true);
		} else {
			// Example: `textures/wood.png`.
			relative_path = p_desired_dest_path;
		}
	}
	// Sanitize the generated path, which stays absolute if its root differs from the referencing destination's.
	// If the user requests `Textures/WOOD.PNG` we will normalize to `textures/wood.png`.
	relative_path = G4MFFileReference4D::sanitize_file_path_uri(relative_path, p_print_warning);
	if (relative_path.is_empty()) {
		return String();
	}
	const String referencing_destination_base_dir = relative_path.is_relative_path() ? referencing_destination_abs_path.get_base_dir() : String();
	// Store the absolute path to allow for future relative lookups for different locations.
	// If the referencing destination is `/a/b/c.g4tf`, this means `/a/b/textures/wood.png`.
	// However, note that this is checked for uniqueness, so it may end up as `wood2.png` etc.
	const String unique_dest_path = _reserve_unique_abs_path(relative_path, p_print_warning, referencing_destination_base_dir);
	if (unique_dest_path.is_empty()) {
		return String();
	}
	if (!empty_key) {
		_source_and_usage_to_dest[key] = unique_dest_path;
	}
	return _get_relative_path(referencing_destination_abs_path, unique_dest_path, true);
}

String G4MFPathContext4D::reserve_destination_usage_path(const String &p_destination_file_abs_path, const String &p_usage_context, const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning) {
	const String destination_file_path = _normalize_abs_path(p_destination_file_abs_path);
	ERR_FAIL_COND_V(destination_file_path.is_empty(), String());
	// Sidecars belong to the full destination filename, including its extension, rather than a reusable source.
	const PathUsageKey key(destination_file_path.to_lower(), p_usage_context);
	const String *existing_path = _destination_and_usage_to_dest.getptr(key);
	if (existing_path != nullptr) {
		return _get_relative_path(destination_file_path, *existing_path, true);
	}
	const String base_dir = destination_file_path.get_base_dir();
	// Keep the stem and extension separate so dots within the stem do not change extension sanitation.
	const String unique_path_uri = reserve_unique_path_uri_from_stem(p_requested_path_stem, p_extension, base_dir, p_print_warning);
	if (unique_path_uri.is_empty()) {
		return String();
	}
	const String absolute_path = _normalize_abs_path(unique_path_uri.is_absolute_path() ? unique_path_uri : base_dir.path_join(unique_path_uri));
	_destination_and_usage_to_dest[key] = absolute_path;
	return _get_relative_path(destination_file_path, absolute_path, true);
}
