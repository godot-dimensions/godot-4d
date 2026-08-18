shader_type sky;

// Compatibility fallback for Godot versions whose sky subpasses do not compile reliably.

#include "../sky/sky_render_parameters_4d.inc.glsl"
#include "volumetric_cloud_common.inc.glsl"

void sky() {
	vec4 eye_direction_4d = vec4(EYEDIR, 0.0);
	vec3 cloud_factors = render_volumetric_cloud_factors(eye_direction_4d, FRAGCOORD.xy, AT_CUBEMAP_PASS);
	COLOR = colorize_volumetric_cloud_factors(cloud_factors).rgb;
}
