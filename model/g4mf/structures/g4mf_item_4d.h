#pragma once

#include "../g4mf_defines_4d.h"

class G4MFItem4D : public Resource {
	GDCLASS(G4MFItem4D, Resource);

	Dictionary _additional_data;
	bool _export_preparing = false;

protected:
	static void _bind_methods();
	virtual Error _export_prepare_for_serialization_internal(const Ref<G4MFState4D> &p_g4mf_state, const bool p_deduplicate);

public:
	static String sanitize_item_name(const String &p_desired_name, const bool p_warn = true);
	inline String get_item_name() const { return get_name(); }
	void set_item_name(const String &p_desired_name);
	void set_unique_item_name(const Ref<G4MFState4D> &p_g4mf_state, const String &p_desired_name, const bool p_print_warning = true);

	virtual TypedArray<G4MFItem4D> get_child_items() const;
	GDVIRTUAL0RC(TypedArray<G4MFItem4D>, _get_child_items);

	Variant get_additional_data(const StringName &p_extension_name);
	bool has_additional_data(const StringName &p_extension_name);
	void set_additional_data(const StringName &p_extension_name, Variant p_additional_data);

	Error export_prepare_for_serialization(const Ref<G4MFState4D> &p_g4mf_state, const bool p_deduplicate = true);
	GDVIRTUAL2R(Error, _export_prepare_for_serialization, Ref<Resource>, bool);

	void read_item_entries_from_dictionary(const Dictionary &p_dict);
	Dictionary write_item_entries_to_dictionary() const;

	// Static helper functions.
	static Array color_to_json_array(const Color &p_color, const bool p_force_include_alpha = true);
	static Color json_array_to_color(const Array &p_json_array);
	static Array int32_array_to_json_array(const PackedInt32Array &p_int32_array);
	static PackedInt32Array json_array_to_int32_array(const Array &p_json_array);
	static Array vector3i_to_json_array(const Vector3i &p_vector);
	static Array vector3_to_json_array(const Vector3 &p_vector);
	static Vector3 json_array_to_vector3(const Array &p_json_array);
	static Array bivector_4d_to_json_array(const Bivector4D &p_bivector);
	static Bivector4D json_array_to_bivector_4d(const Array &p_json_array);
	static Array rotor_4d_to_json_array(const Rotor4D &p_rotor);
	static Rotor4D json_array_to_rotor_4d(const Array &p_json_array);
};
