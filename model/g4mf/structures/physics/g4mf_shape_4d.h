#pragma once

#include "../g4mf_item_4d.h"

#include "../../../../physics/shapes/general_shape_4d.h"

#if GDEXTENSION
#include <godot_cpp/variant/typed_array.hpp>
#elif GODOT_MODULE
#include "core/variant/typed_array.h"
#endif

class G4MFState4D;

class G4MFShape4D : public G4MFItem4D {
	GDCLASS(G4MFShape4D, G4MFItem4D);

	Vector4 _base_size = Vector4();
	TypedArray<GeneralShapeCurve4D> _curves;
	String _shape_type = "general";
	double _ray_length = 1.0;
	int _heights_accessor_index = -1;
	int _mesh_index = -1;

	Ref<Shape4D> _generate_shape_from_general() const;

protected:
	static void _bind_methods();

public:
	Vector4 get_base_size() const { return _base_size; }
	void set_base_size(const Vector4 &p_base_size) { _base_size = p_base_size; }

	TypedArray<GeneralShapeCurve4D> get_curves() const { return _curves; }
	void set_curves(const TypedArray<GeneralShapeCurve4D> &p_curves) { _curves = p_curves; }

	String get_shape_type() const { return _shape_type; }
	void set_shape_type(const String &p_shape_type) { _shape_type = p_shape_type; }

	real_t get_ray_length() const { return _ray_length; }
	void set_ray_length(real_t p_ray_length) { _ray_length = p_ray_length; }

	int get_heights_accessor_index() const { return _heights_accessor_index; }
	void set_heights_accessor_index(int p_heights_accessor_index) { _heights_accessor_index = p_heights_accessor_index; }

	int get_mesh_index() const { return _mesh_index; }
	void set_mesh_index(int p_mesh_index) { _mesh_index = p_mesh_index; }

	bool is_equal_exact(const Ref<G4MFShape4D> &p_other) const;

	PackedFloat64Array load_heights(const Ref<G4MFState4D> &p_g4mf_state) const;

	Ref<Shape4D> generate_shape(const Ref<G4MFState4D> &p_g4mf_state) const;
	static Ref<G4MFShape4D> convert_shape(Ref<G4MFState4D> p_g4mf_state, const Ref<Shape4D> &p_shape, const bool p_deduplicate = true);
	static int convert_shape_into_state(Ref<G4MFState4D> p_g4mf_state, const Ref<Shape4D> &p_shape, const bool p_deduplicate = true);

	static Ref<G4MFShape4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
