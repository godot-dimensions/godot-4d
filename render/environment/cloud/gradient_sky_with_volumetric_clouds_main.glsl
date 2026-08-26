void sky() {
	vec4 eye_direction_4d = vec4(EYEDIR, 0.0);
	if (AT_QUARTER_RES_PASS) {
		COLOR = encode_volumetric_cloud_factors(render_volumetric_cloud_factors(eye_direction_4d, FRAGCOORD.xy, AT_CUBEMAP_PASS));
	} else {
		vec4 clouds = colorize_volumetric_cloud_factors(decode_volumetric_cloud_factors(QUARTER_RES_COLOR.rgb));
		vec3 base_sky = render_gradient_sky(eye_direction_4d);
		COLOR = base_sky * (1.0 - clouds.a) + clouds.rgb;
	}
}
