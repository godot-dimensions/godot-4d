// AI-generated. Adapted from Godot's PhysicalSkyMaterial shader to evaluate directions in 4D.
shader_type sky;
render_mode use_debanding;

#include "physical_sky_common.inc.glsl"

void sky() {
	vec4 eye_direction_4d = vec4(EYEDIR, 0.0);
	COLOR = render_physical_sky(eye_direction_4d, AT_CUBEMAP_PASS);
}
