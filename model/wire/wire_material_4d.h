#pragma once

#include "../material_4d.h"

class WireMaterial4D : public Material4D {
	GDCLASS(WireMaterial4D, Material4D);

public:
	// TODO: Switch to BitField in a future Godot version https://github.com/godotengine/godot/pull/89457
	enum WireColorSource {
		WIRE_COLOR_SOURCE_SINGLE_COLOR,
		WIRE_COLOR_SOURCE_PER_EDGE_ONLY,
		WIRE_COLOR_SOURCE_PER_EDGE_AND_SINGLE,
	};

private:
	WireColorSource _albedo_source = WIRE_COLOR_SOURCE_SINGLE_COLOR;
	real_t _line_thickness = 0.0f;

	static Material4D::ColorSourceFlags _wire_source_to_flags(const WireColorSource p_wire_source);

protected:
	static void _bind_methods();
	void _get_property_list(List<PropertyInfo> *p_list) const;

public:
	virtual void merge_with(const Ref<Material4D> &p_material, const int p_first_edge_count, const int p_second_edge_count) override;

	WireColorSource get_albedo_source() const;
	void set_albedo_source(const WireColorSource p_albedo_source);

	real_t get_line_thickness() const;
	void set_line_thickness(const real_t p_line_thickness);

	WireMaterial4D();
};

VARIANT_ENUM_CAST(WireMaterial4D::WireColorSource);
