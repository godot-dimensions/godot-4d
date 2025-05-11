#pragma once

#include "../../../math/geometric_algebra/rotor_4d.h"
#include "../../../math/transform_4d.h"

#if GDEXTENSION
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#elif GODOT_MODULE
#include "core/io/resource.h"
#endif

class G4MFItem4D : public Resource {
	GDCLASS(G4MFItem4D, Resource);

	Dictionary _additional_data;

protected:
	static void _bind_methods();

public:
	Variant get_additional_data(const StringName &p_extension_name);
	bool has_additional_data(const StringName &p_extension_name);
	void set_additional_data(const StringName &p_extension_name, Variant p_additional_data);

	void read_item_entries_from_dictionary(const Dictionary &p_dict);
	Dictionary write_item_entries_to_dictionary() const;

	// Static helper functions.
	static Array int32_array_to_json_array(const PackedInt32Array &p_int32_array);
	static PackedInt32Array json_array_to_int32_array(const Array &p_json_array);
	static Array bivector_4d_to_json_array(const Bivector4D &p_bivector);
	static Bivector4D json_array_to_bivector_4d(const Array &p_json_array);
	static Array rotor_4d_to_json_array(const Rotor4D &p_rotor);
	static Rotor4D json_array_to_rotor_4d(const Array &p_json_array);
};
