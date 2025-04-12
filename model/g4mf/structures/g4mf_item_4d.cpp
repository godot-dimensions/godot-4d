#include "g4mf_item_4d.h"

Variant G4MFItem4D::get_additional_data(const StringName &p_extension_name) {
	return _additional_data[p_extension_name];
}

bool G4MFItem4D::has_additional_data(const StringName &p_extension_name) {
	return _additional_data.has(p_extension_name);
}

void G4MFItem4D::set_additional_data(const StringName &p_extension_name, Variant p_additional_data) {
	_additional_data[p_extension_name] = p_additional_data;
}

void G4MFItem4D::read_item_entries_from_dictionary(const Dictionary &p_dict) {
	if (p_dict.has("name")) {
		set_name(p_dict["name"]);
	}
	if (p_dict.has("extras")) {
		set_meta("extras", p_dict["extras"]);
	}
}

Dictionary G4MFItem4D::write_item_entries_to_dictionary() const {
	Dictionary dict;
	if (!dict.has("name") && !get_name().is_empty()) {
		dict["name"] = get_name();
	}
	if (!dict.has("extras") && has_meta("extras")) {
		dict["extras"] = get_meta("extras");
	}
	return dict;
}

// Static helper functions.

Array G4MFItem4D::int32_array_to_json_array(const PackedInt32Array &p_int32_array) {
	Array json_array;
	json_array.resize(p_int32_array.size());
	for (int i = 0; i < p_int32_array.size(); i++) {
		json_array[i] = p_int32_array[i];
	}
	return json_array;
}

PackedInt32Array G4MFItem4D::json_array_to_int32_array(const Array &p_json_array) {
	PackedInt32Array int32_array;
	int32_array.resize(p_json_array.size());
	for (int i = 0; i < p_json_array.size(); i++) {
		int32_array.set(i, p_json_array[i]);
	}
	return int32_array;
}

void G4MFItem4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_additional_data", "extension_name"), &G4MFItem4D::get_additional_data);
	ClassDB::bind_method(D_METHOD("has_additional_data", "extension_name"), &G4MFItem4D::has_additional_data);
	ClassDB::bind_method(D_METHOD("set_additional_data", "extension_name", "additional_data"), &G4MFItem4D::set_additional_data);

	ClassDB::bind_method(D_METHOD("read_item_entries_from_dictionary", "dict"), &G4MFItem4D::read_item_entries_from_dictionary);
	ClassDB::bind_method(D_METHOD("write_item_entries_to_dictionary"), &G4MFItem4D::write_item_entries_to_dictionary);

	ClassDB::bind_static_method("G4MFItem4D", D_METHOD("int32_array_to_json_array", "int32_array"), &G4MFItem4D::int32_array_to_json_array);
	ClassDB::bind_static_method("G4MFItem4D", D_METHOD("json_array_to_int32_array", "json_array"), &G4MFItem4D::json_array_to_int32_array);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "name"), "set_name", "get_name");
}
