#pragma once

#include "g4mf_item_4d.h"

class G4MFState4D;

class G4MFLight4D : public G4MFItem4D {
	GDCLASS(G4MFLight4D, G4MFItem4D);

	String _light_type = "point";
	Color _color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	double _cone_inner_angle = 0.0;
	double _cone_outer_angle = 0.7853981633974483;
	double _intensity = 1.0;
	double _range = INFINITY;

protected:
	static void _bind_methods();

public:
	String get_light_type() const { return _light_type; }
	void set_light_type(const String &p_light_type) { _light_type = p_light_type; }

	Color get_color() const { return _color; }
	void set_color(const Color &p_color) { _color = p_color; }

	double get_cone_inner_angle() const { return _cone_inner_angle; }
	void set_cone_inner_angle(const double p_cone_inner_angle) { _cone_inner_angle = p_cone_inner_angle; }

	double get_cone_outer_angle() const { return _cone_outer_angle; }
	void set_cone_outer_angle(const double p_cone_outer_angle) { _cone_outer_angle = p_cone_outer_angle; }

	double get_intensity() const { return _intensity; }
	void set_intensity(const double p_intensity) { _intensity = p_intensity; }

	double get_range() const { return _range; }
	void set_range(const double p_range) { _range = p_range; }

	static Ref<G4MFLight4D> from_dictionary(const Dictionary &p_dict);
	Dictionary to_dictionary() const;
};
