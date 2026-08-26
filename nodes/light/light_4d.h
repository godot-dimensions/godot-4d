#pragma once

#include "../node_4d.h"

class Light4D : public Node4D {
	GDCLASS(Light4D, Node4D);

private:
	Color _light_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	real_t _light_energy = 1.0f;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// Shared code that can be used by any 4D rendering engine that builds on top of 3D lighting.
	virtual RID create_light_3d_render_base() const = 0;
	virtual bool update_light_3d_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const = 0;

	Color get_light_color() const { return _light_color; }
	void set_light_color(const Color &p_light_color) { _light_color = p_light_color; }

	real_t get_light_energy() const { return _light_energy; }
	void set_light_energy(const real_t p_light_energy) { _light_energy = p_light_energy; }
};
