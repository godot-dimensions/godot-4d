shader_type sky;
render_mode use_debanding, use_quarter_res_pass;

// The quarter-resolution subpass keeps volumetric raymarching practical on RenderingDevice backends.

#include "../sky/physical_sky_common.inc.glsl"
#include "volumetric_cloud_common.inc.glsl"

void sky() {
	vec4 eye_direction_4d = vec4(EYEDIR, 0.0);
	if (AT_QUARTER_RES_PASS) {
		COLOR = encode_volumetric_cloud_factors(render_volumetric_cloud_factors(eye_direction_4d, FRAGCOORD.xy, AT_CUBEMAP_PASS));
	} else {
		vec4 clouds = colorize_volumetric_cloud_factors(decode_volumetric_cloud_factors(QUARTER_RES_COLOR.rgb));
		vec3 base_sky = render_physical_sky(eye_direction_4d, AT_CUBEMAP_PASS);
		COLOR = base_sky * (1.0 - clouds.a) + clouds.rgb;
	}
}
