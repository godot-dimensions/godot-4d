#pragma once

#include "../../g4mf_defines_4d.h"

class G4MFFileReference4D;

// Internal helper class for file system path context shared between G4MF states.
// Coordinates referenced file destinations and prevents path conflicts across exports.
// This class does not need to be exposed to the scripting API, except to satisfy GDExtension.
class G4MFPathContext4D : public RefCounted {
	GDCLASS(G4MFPathContext4D, RefCounted);

	// Each namespace keys allocations by a path and usage: either a reusable source or a referencing destination file.
	using PathUsageKey = Pair<String, String>;
	// godot-cpp does not provide the engine's default Pair hasher.
	struct PathUsageKeyHasher {
		static uint32_t hash(const PathUsageKey &p_key) {
			return hash_one_uint64((uint64_t(p_key.first.hash()) << 32) | p_key.second.hash());
		}
	};
	HashMap<PathUsageKey, String, PathUsageKeyHasher> _source_and_usage_to_dest;
	HashMap<PathUsageKey, String, PathUsageKeyHasher> _destination_and_usage_to_dest;
	// Anonymous, shared, and explicitly marked paths remain reserved independently of owners.
	HashSet<String> _lowercase_reserved_paths;
	HashSet<String> _lowercase_owned_paths;
	// Store absolute keys so releasing ownership does not depend on the current base directory or URI.
	HashMap<uint64_t, String> _owner_ids_to_paths;

	static String _normalize_abs_path(const String &p_abs_path);
	static String _get_path_root(const String &p_abs_path);
	static String _get_relative_path(const String &p_referencing_abs_path, const String &p_target_abs_path, const bool p_use_updir_dots);
	static Pair<String, String> _get_path_stem_and_extension(const String &p_path);
	// Returns a rooted path, resolving a relative request against the explicit base directory.
	String _reserve_unique_abs_path(const String &p_requested_path_uri, const bool p_print_warning, const String &p_base_abs_dir = String());
	// The caller sanitizes the generated stem and extension. Preserve the requested URI's relative or absolute form.
	String _reserve_sanitized_unique_path_uri(const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning, const String &p_base_abs_dir = String());

protected:
	static void _bind_methods() {}

public:
	static String get_source_placement_path(const String &p_source_abs_path, const String &p_referencing_source_abs_path);
	void assign_unique_path_uri_from_stem(const Ref<G4MFFileReference4D> &p_file_reference, const String &p_requested_path_stem, const String &p_extension, const String &p_base_abs_dir, const bool p_print_warning);
	void assign_unique_path_uri(const Ref<G4MFFileReference4D> &p_file_reference, const String &p_requested_path_uri, const String &p_base_abs_dir, const bool p_print_warning);
	String reserve_unique_path_uri_from_stem(const String &p_requested_path_stem, const String &p_extension, const String &p_base_abs_dir, const bool p_print_warning);
	String reserve_destination_abs_path_for_source(const String &p_source_abs_path, const String &p_usage_context, const String &p_desired_dest_abs_path, const bool p_print_warning);
	String reserve_destination_path_for_source(const String &p_source_abs_path, const String &p_referencing_source_abs_path, const String &p_usage_context, const String &p_referencing_destination_abs_path, const String &p_desired_dest_path, const bool p_print_warning);
	String reserve_destination_usage_path(const String &p_destination_file_abs_path, const String &p_usage_context, const String &p_requested_path_stem, const String &p_extension, const bool p_print_warning);
	void mark_path_as_reserved(const String &p_abs_path);
};
