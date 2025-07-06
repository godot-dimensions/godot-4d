#include "g4mf_item_4d.h"

String G4MFItem4D::get_item_name() const {
	return get_name();
}

void G4MFItem4D::set_item_name(const String &p_name) {
	set_name(p_name);
}

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
	if (!dict.has("extras") && has_meta("extras")) {
		dict["extras"] = get_meta("extras");
	}
	if (!dict.has("name") && !get_name().is_empty()) {
		dict["name"] = get_name();
	}
	return dict;
}

// Static helper functions.

Array G4MFItem4D::color_to_json_array(const Color &p_color, const bool p_force_include_alpha) {
	const bool include_alpha = p_force_include_alpha || p_color.a != 1.0;
	Array json_array;
	json_array.resize(include_alpha ? 4 : 3);
	json_array[0] = p_color.r;
	json_array[1] = p_color.g;
	json_array[2] = p_color.b;
	if (include_alpha) {
		json_array[3] = p_color.a;
	}
	return json_array;
}

Color G4MFItem4D::json_array_to_color(const Array &p_json_array) {
	Color color;
	if (likely(p_json_array.size() >= 3)) {
		color.r = p_json_array[0];
		color.g = p_json_array[1];
		color.b = p_json_array[2];
	} else if (p_json_array.size() >= 1) {
		color.r = color.g = color.b = p_json_array[0];
	}
	if (p_json_array.size() >= 4) {
		color.a = p_json_array[3];
	} else {
		color.a = 1.0;
	}
	return color;
}

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

Array G4MFItem4D::bivector_4d_to_json_array(const Bivector4D &p_bivector) {
	// Note: Bivector4D uses lexicographic geometric algebra order, but G4MF uses dimensionally-increasing order.
	Array json_array;
	json_array.resize(6);
	json_array[0] = p_bivector.xy;
	json_array[1] = p_bivector.xz;
	json_array[2] = p_bivector.yz;
	json_array[3] = p_bivector.xw;
	json_array[4] = p_bivector.yw;
	json_array[5] = p_bivector.zw;
	return json_array;
}

Bivector4D G4MFItem4D::json_array_to_bivector_4d(const Array &p_json_array) {
	// Note: Bivector4D uses lexicographic geometric algebra order, but G4MF uses dimensionally-increasing order.
	if (likely(p_json_array.size() >= 6)) {
		return Bivector4D(
				p_json_array[0], p_json_array[1], p_json_array[3],
				p_json_array[2], p_json_array[4], p_json_array[5]);
	}
	if (p_json_array.size() >= 3) {
		return Bivector4D(
				p_json_array[0], p_json_array[1], 0.0,
				p_json_array[2], 0.0, 0.0);
	}
	if (p_json_array.size() >= 1) {
		return Bivector4D(p_json_array[0], 0.0, 0.0, 0.0, 0.0, 0.0);
	}
	return Bivector4D();
}

Array G4MFItem4D::rotor_4d_to_json_array(const Rotor4D &p_rotor) {
	// Note: Rotor4D uses lexicographic geometric algebra order, but G4MF uses dimensionally-increasing order.
	Array json_array;
	json_array.resize(8);
	json_array[0] = p_rotor.s;
	json_array[1] = p_rotor.xy;
	json_array[2] = p_rotor.xz;
	json_array[3] = p_rotor.yz;
	json_array[4] = p_rotor.xw;
	json_array[5] = p_rotor.yw;
	json_array[6] = p_rotor.zw;
	json_array[7] = p_rotor.xyzw;
	return json_array;
}

Rotor4D G4MFItem4D::json_array_to_rotor_4d(const Array &p_json_array) {
	// Note: Rotor4D uses lexicographic geometric algebra order, but G4MF uses dimensionally-increasing order.
	if (likely(p_json_array.size() == 8)) {
		return Rotor4D(
				p_json_array[0], p_json_array[1], p_json_array[2], p_json_array[4],
				p_json_array[3], p_json_array[5], p_json_array[6], p_json_array[7])
				.normalized();
	}
	// The rest of these only support simple rotations.
	if (p_json_array.size() >= 7) {
		return Rotor4D(
				p_json_array[0], p_json_array[1], p_json_array[2], p_json_array[4],
				p_json_array[3], p_json_array[5], p_json_array[6], 0.0)
				.normalized();
	}
	if (p_json_array.size() >= 4) {
		return Rotor4D(
				p_json_array[0], p_json_array[1], p_json_array[2], 0.0,
				p_json_array[3], 0.0, 0.0, 0.0)
				.normalized();
	}
	if (p_json_array.size() >= 2) {
		return Rotor4D(p_json_array[0], p_json_array[1], 0.0, 0.0, 0.0, 0.0, 0.0, 0.0).normalized();
	}
	return Rotor4D::identity();
}

void G4MFItem4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_item_name"), &G4MFItem4D::get_item_name);
	ClassDB::bind_method(D_METHOD("set_item_name", "name"), &G4MFItem4D::set_item_name);

	ClassDB::bind_method(D_METHOD("get_additional_data", "extension_name"), &G4MFItem4D::get_additional_data);
	ClassDB::bind_method(D_METHOD("has_additional_data", "extension_name"), &G4MFItem4D::has_additional_data);
	ClassDB::bind_method(D_METHOD("set_additional_data", "extension_name", "additional_data"), &G4MFItem4D::set_additional_data);

	ClassDB::bind_method(D_METHOD("read_item_entries_from_dictionary", "dict"), &G4MFItem4D::read_item_entries_from_dictionary);
	ClassDB::bind_method(D_METHOD("write_item_entries_to_dictionary"), &G4MFItem4D::write_item_entries_to_dictionary);

	ClassDB::bind_static_method("G4MFItem4D", D_METHOD("int32_array_to_json_array", "int32_array"), &G4MFItem4D::int32_array_to_json_array);
	ClassDB::bind_static_method("G4MFItem4D", D_METHOD("json_array_to_int32_array", "json_array"), &G4MFItem4D::json_array_to_int32_array);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "item_name"), "set_item_name", "get_item_name");
}
