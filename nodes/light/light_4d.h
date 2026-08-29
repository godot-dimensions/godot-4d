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
	// Helpers for rendering engines that represent Light4D nodes with Godot's 3D light RIDs.
	virtual RID create_light_3d_render_base() const = 0;
	// The cross-section and projected approximations of a 4D light are potentially different shapes
	// (at least in the fallback case where we can't just send the real 4D geometry), so each engine
	// gets its own render base, which the combined rendering engine needs to use at the same time.
	virtual bool update_light_3d_cross_section_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const = 0;
	virtual bool update_light_3d_projected_render_base(const Projection &p_relative_to_camera_basis, const Vector4 &p_relative_to_camera_position, const RID p_light_3d_render_base) const = 0;

	Color get_light_color() const { return _light_color; }
	void set_light_color(const Color &p_light_color) { _light_color = p_light_color; }

	real_t get_light_energy() const { return _light_energy; }
	void set_light_energy(const real_t p_light_energy) { _light_energy = p_light_energy; }
};
