#pragma once

#include "sky_material_4d.h"

class PlainSkyMaterial4D : public SkyMaterial4D {
	GDCLASS(PlainSkyMaterial4D, SkyMaterial4D);

	static Ref<Shader> _shader;

	Color _color;

protected:
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	Color get_color() const { return _color; }
	void set_color(const Color &p_color);

	static void init_shader();
	static void cleanup_shader();

	PlainSkyMaterial4D();
};
